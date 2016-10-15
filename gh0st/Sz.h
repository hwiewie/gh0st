#if !defined(AFX_SZ_H__236A8281_B188_4A29_91D3_2F475032BAAF__INCLUDED_)
#define AFX_SZ_H__236A8281_B188_4A29_91D3_2F475032BAAF__INCLUDED_

#include "Set.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Sz.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSz dialog

class CSz : public CDialog
{
// Construction
public:
	void OnPaint();
	CTabCtrl m_tabctrl;
	CSound m_mm1;
	CSet m_mm2;
	BOOL OnInitDialog();
	CSz(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSz)
	enum { IDD = IDD_GLSZ };
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSz)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CSz)
	afx_msg void OnSelchangeTab1(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SZ_H__236A8281_B188_4A29_91D3_2F475032BAAF__INCLUDED_)
