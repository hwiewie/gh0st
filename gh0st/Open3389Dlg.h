#if !defined(AFX_OPEN3389DLG_H__15636D5F_FCAF_4DFA_AED8_DAF10B54F6E2__INCLUDED_)
#define AFX_OPEN3389DLG_H__15636D5F_FCAF_4DFA_AED8_DAF10B54F6E2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Open3389Dlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COpen3389Dlg dialog

class COpen3389Dlg : public CDialog
{
// Construction
public:
	COpen3389Dlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(COpen3389Dlg)
	enum { IDD = IDD_Open3389 };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COpen3389Dlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(COpen3389Dlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPEN3389DLG_H__15636D5F_FCAF_4DFA_AED8_DAF10B54F6E2__INCLUDED_)
