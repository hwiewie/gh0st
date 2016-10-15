#if !defined(AFX_UPIP_H__929DFBCA_A8FB_4A85_B8AB_7E15A9BDF302__INCLUDED_)
#define AFX_UPIP_H__929DFBCA_A8FB_4A85_B8AB_7E15A9BDF302__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// UpIp.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CUpIp form view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif
#include "control/HoverEdit.h"
#include "CONTROL\HoverEdit.h"	// Added by ClassView
class CUpIp : public CFormView
{
public:
	CUpIp(); 
	virtual ~CUpIp();// protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CUpIp)

// Form Data
public:
	//{{AFX_DATA(CUpIp)
	enum { IDD = IDD_UPIP };
	CString	m_FtpIP;
	int		m_FtpPort;
	CString	m_Pass;
	CString	m_User;
	CString	m_FtpUrl;
	//}}AFX_DATA

// Attributes
public:

// Operations
public:
	BOOL InitInstance();
	CHoverEdit m_Edit1[9];

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUpIp)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);
	//}}AFX_VIRTUAL

// Implementation
protected:
	
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CUpIp)
	afx_msg void OnUpdnsip();
	afx_msg void OnUpftp();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_UPIP_H__929DFBCA_A8FB_4A85_B8AB_7E15A9BDF302__INCLUDED_)
