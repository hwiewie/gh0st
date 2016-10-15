#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>

#pragma comment(lib, "Psapi.lib")

void HideLibrary(HMODULE hModule, LPVOID pCallBackAddr, LPVOID lParam);

typedef struct
{
      HMODULE lpDllBase;
      LPVOID lpNewDllBase;
      PTHREAD_START_ROUTINE pAddress;
      LPVOID lParam;
}UNLOADLIB_CALLBACK, *PUNLOADLIB_CALLBACK;

typedef
LPVOID WINAPI VIRTUALALLOC(
    LPVOID lpAddress,
    SIZE_T dwSize,
    DWORD flAllocationType,
    DWORD flProtect
);

typedef
BOOL WINAPI VIRTUALFREE(
    LPVOID lpAddress,
    SIZE_T dwSize,
    DWORD dwFreeType
);


typedef
BOOL WINAPI HEAPDESTROY(
    HANDLE hHeap
);

typedef
HMODULE WINAPI LOADLIBRARY(
    LPCTSTR lpFileName
);

typedef
HANDLE WINAPI CREATETHREAD(
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    SIZE_T dwStackSize,
    LPTHREAD_START_ROUTINE lpStartAddress,
    LPVOID lpParameter,
    DWORD dwCreationFlags,
    LPDWORD lpThreadId
);

typedef void *    __cdecl MEMCPY(void *, const void *, size_t);


BOOL incLibraryCount(HMODULE hMe)
{
      //FreeLibrary後很多系統dll也會free掉，所以將所有已加載的再load一次以增加計數
    
      HANDLE hModsSnap =    CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, 0);

      if(INVALID_HANDLE_VALUE == hModsSnap)
      {
          return FALSE;
      }

      MODULEENTRY32 meModuleEntry;
      meModuleEntry.dwSize = sizeof(MODULEENTRY32);

      if(!Module32First(hModsSnap, &meModuleEntry))
      {
          CloseHandle(hModsSnap);
          return FALSE;
      }
      do
      {
          if(LoadLibrary(meModuleEntry.szModule) == hMe)
              FreeLibrary(hMe);

      } while(Module32Next(hModsSnap, &meModuleEntry));

      CloseHandle(hModsSnap);

      return TRUE;
}

//枚舉指定進程的所有線程
DWORD WINAPI EnumAndSetThreadState(LPVOID lParam)
{
      HANDLE hThreadSnap = NULL;
      THREADENTRY32 te32;
      memset(&te32,0,sizeof(te32));
      te32.dwSize = sizeof(THREADENTRY32);
      hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD,0);

      DWORD myThreadId = GetCurrentThreadId();
      DWORD pid = GetCurrentProcessId();

      if (Thread32First(hThreadSnap, &te32))
      {
          do
          {
               if (pid == te32.th32OwnerProcessID)
               {
                   if(myThreadId != te32.th32ThreadID)
                   {
                       HANDLE hThread = OpenThread(
                           THREAD_SUSPEND_RESUME,
                           FALSE,
                           te32.th32ThreadID);

                       if(hThread != NULL)
                       {
                           if((int)lParam)
                               ResumeThread(hThread);
                           else
                               SuspendThread(hThread);

                           CloseHandle(hThread);
                       }
                   }
               }
          }
          while (Thread32Next(hThreadSnap,&te32));
      }
      CloseHandle( hThreadSnap );

      return 0;
}

DWORD WINAPI GotoCallBackAddr(LPVOID lParam)
{
      PUNLOADLIB_CALLBACK cbFunc = (PUNLOADLIB_CALLBACK)lParam;

      DWORD dwThreadId;
      HANDLE hThread;

      if(cbFunc->pAddress)
      {
          hThread = CreateThread(
              NULL,
              0,
              cbFunc->pAddress,
              cbFunc->lParam,
              0,
              &dwThreadId);

          if(hThread)
              CloseHandle(hThread);
      }

      //那份dll的拷貝不需要了，釋放~
      VirtualFree(cbFunc->lpNewDllBase, 0, MEM_DECOMMIT);
      delete cbFunc;

      return 0;
}

DWORD WINAPI UnLoadLibrary(LPVOID lParam)
{
      BYTE HeapDestroy_HookCode_bak[4];
      BYTE HeapDestroy_HookCode[4] = "\xC2\x04\x00";//RETN 0004
      MODULEINFO modinfo;
      DWORD oldProtect;

      PUNLOADLIB_CALLBACK cbFunc = (PUNLOADLIB_CALLBACK)lParam;

      HMODULE hDllInstance = cbFunc->lpDllBase;
      char dllpath_bak[MAX_PATH];

      GetModuleFileName(hDllInstance, dllpath_bak, sizeof(dllpath_bak));
      GetModuleInformation(GetCurrentProcess(), hDllInstance, &modinfo, sizeof(MODULEINFO));

      //給所有dll(除了自己)增加計數,防止FreeLibrary的時候那些dll給系統卸載掉
      incLibraryCount(hDllInstance);

      //保險起見，掛起其他線程，搞定後再恢復
      EnumAndSetThreadState((LPVOID)FALSE);

      //FreeLibrary之後原來存放api地址的內存也會被釋放，
      //但是FreeLibrary之後還有些動作，趁現在還沒free，關鍵API記下來
      VIRTUALALLOC *_VirtualAlloc = (VIRTUALALLOC*)
          GetProcAddress(GetModuleHandle("kernel32.dll"), "VirtualAlloc");
      LOADLIBRARY    *_LoadLibrary    = (LOADLIBRARY*)
          GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA");
      CREATETHREAD *_CreateThread = (CREATETHREAD*)
          GetProcAddress(GetModuleHandle("kernel32.dll"), "CreateThread");
      MEMCPY         *_memcpy         = (MEMCPY*)
          GetProcAddress(GetModuleHandle("ntdll.dll"), "memcpy");

      //這個很關鍵，並不是我要調用，是 FreeLibrary 時系統會調用，我要hook它,
      //不能給系統破壞這個heap，否則之後的dll貌似能工作，
      //但卻不能用new或malloc申請內存, VirtualAlloc可以代替之，
      //但如果改寫好多代碼是划不來的，況且一些代碼不好改,如list<T>的push內部的new

      HEAPDESTROY *_HeapDestroy    = (HEAPDESTROY*)
          GetProcAddress(GetModuleHandle("kernel32.dll"), "HeapDestroy");

      VirtualProtect(_HeapDestroy, 3, PAGE_EXECUTE_READWRITE, &oldProtect);

      //修改第一條指令為直接返回
      _memcpy(HeapDestroy_HookCode_bak, _HeapDestroy, 3);
      _memcpy(_HeapDestroy, HeapDestroy_HookCode, 3);


      //Sleep(100);
      //終於到這裡了~~~^_^!
      FreeLibrary(hDllInstance);//釋放

      //修復剛hook的函數
      _memcpy(_HeapDestroy, HeapDestroy_HookCode_bak, 3);
      //_memcpy(_RtlFreeHeap, RtlFreeHeap_HookCode_bak, 3);

      //在原來的dll基址申請同樣大小的內存，並把之前的那份dll拷貝還原回去
      if(_VirtualAlloc(hDllInstance,
          modinfo.SizeOfImage,
          MEM_COMMIT|MEM_RESERVE,
          PAGE_EXECUTE_READWRITE) == NULL
          )
      {
          //失敗，加載原來dll, 以正常方式工作
          //注意，不宜在dllmain中調用HideLibrary，LoadLibrary將導致dllmain再次被調用,導致死循環啦
          HMODULE hDll = _LoadLibrary(dllpath_bak);

          //重新計算回調函數在hDll地址空間的地址
          cbFunc->pAddress = (LPTHREAD_START_ROUTINE)
              ((DWORD)cbFunc->pAddress - (DWORD)hDllInstance + (DWORD)hDll);

          LPTHREAD_START_ROUTINE pFunc1 = (LPTHREAD_START_ROUTINE)
              ((DWORD)EnumAndSetThreadState - (DWORD)hDllInstance + (DWORD)hDll);

          //恢復被掛起的線程
          _CreateThread(0, 0, pFunc1, (LPVOID)TRUE, 0, 0);

          //調用回調函數
          if(cbFunc->pAddress)
              _CreateThread(0, 0, cbFunc->pAddress, cbFunc->lParam, 0, 0);

          return 0;
      }

      _memcpy(hDllInstance, cbFunc->lpNewDllBase, modinfo.SizeOfImage);

      //恢復被掛起的線程
      EnumAndSetThreadState((LPVOID)TRUE);

      //跳回原dll地址空間的GotoCallBackAddr，由它來釋放這邊VirtualAlloc申請的指針
      _CreateThread(0, 0, GotoCallBackAddr, cbFunc, 0, 0);

      return 0;
}

DWORD WINAPI HideLibrary02(LPVOID lParam)
{
      //__asm INT 3

      PUNLOADLIB_CALLBACK cbFunc = (PUNLOADLIB_CALLBACK)lParam;

      MODULEINFO modinfo;

      GetModuleInformation(GetCurrentProcess(), cbFunc->lpDllBase, &modinfo, sizeof(MODULEINFO));

      //申請一塊和當前dll同樣大小的內存
      cbFunc->lpNewDllBase = VirtualAlloc(NULL, modinfo.SizeOfImage, MEM_COMMIT|MEM_RESERVE, PAGE_EXECUTE_READWRITE);

      if(cbFunc->lpNewDllBase == NULL)
          return FALSE;

      //給當前dll做份拷貝，複製所有數據到剛申請的內存，
      memcpy(cbFunc->lpNewDllBase, modinfo.lpBaseOfDll, modinfo.SizeOfImage);

      //計算在copy中UnLoadLibrary的地址,並另起線程到該地址執行
      void *pNewUnLoadLibrary = LPVOID((DWORD)cbFunc->lpNewDllBase + (DWORD)UnLoadLibrary - (DWORD)modinfo.lpBaseOfDll);

      DWORD ThreadId;
      HANDLE hThread = CreateThread(0,0,
          (LPTHREAD_START_ROUTINE)pNewUnLoadLibrary, (LPVOID)cbFunc, CREATE_SUSPENDED, &ThreadId);

      if(hThread == NULL)
      {
          VirtualFree(cbFunc->lpNewDllBase, 0, MEM_DECOMMIT);
          delete cbFunc;

          return FALSE;
      }

      ResumeThread(hThread);
      CloseHandle(hThread);

      return TRUE;
}


void HideLibrary(HMODULE hModule, LPVOID pCallBackAddr, LPVOID lParam)
{

      PUNLOADLIB_CALLBACK lparam = new UNLOADLIB_CALLBACK;

      lparam->lpDllBase      = hModule;
      lparam->lpNewDllBase = NULL;
      lparam->pAddress       = (PTHREAD_START_ROUTINE)pCallBackAddr;
      lparam->lParam         = lParam;

      HANDLE hThread = CreateThread(0,0,
          HideLibrary02, (LPVOID)lparam, 0, NULL);

      if(hThread == NULL)
      {
          delete lparam;
          return;
      }

      CloseHandle(hThread);

      return;
}

