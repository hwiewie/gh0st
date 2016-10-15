#if !defined(AFX_IP3322DLG_H__D3E995EA_9C01_4A18_84EB_CFA5680E13FD__INCLUDED_)
#define AFX_IP3322DLG_H__D3E995EA_9C01_4A18_84EB_CFA5680E13FD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Ip3322Dlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CIp3322Dlg dialog

class CIp3322Dlg : public CDialog
{
// Construction
public:
	void ONBtnUpdate();
	BOOL InitComboBox();
	BOOL OnInitDialog();
	BOOL InitLocalIP();
	CIp3322Dlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CIp3322Dlg)
	enum { IDD = IDD_FTP_3322 };
	CComboBox	m_cc;
	CComboBox	m_cmbIP;
	CButton	m_btnUpdate;
	CEdit	m_editym;
	CEdit	m_editpass;
	CEdit	m_editname;
	CString	m_strname;
	CString	m_strym;
	CString	m_strpass;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CIp3322Dlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CIp3322Dlg)
	afx_msg void OnButton2();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IP3322DLG_H__D3E995EA_9C01_4A18_84EB_CFA5680E13FD__INCLUDED_)
