// Open3389Dlg.cpp : implementation file
//

#include "stdafx.h"
#include "gh0st.h"
#include "Open3389Dlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COpen3389Dlg dialog


COpen3389Dlg::COpen3389Dlg(CWnd* pParent /*=NULL*/)
	: CDialog(COpen3389Dlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(COpen3389Dlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void COpen3389Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COpen3389Dlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COpen3389Dlg, CDialog)
	//{{AFX_MSG_MAP(COpen3389Dlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COpen3389Dlg message handlers
