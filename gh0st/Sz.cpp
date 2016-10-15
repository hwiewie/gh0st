// Sz.cpp : implementation file
//

#include "stdafx.h"
#include "gh0st.h"
#include "Sz.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSz dialog


CSz::CSz(CWnd* pParent /*=NULL*/)
	: CDialog(CSz::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSz)
	//}}AFX_DATA_INIT
}


void CSz::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSz)
	DDX_Control(pDX, IDC_TAB1, m_tabctrl);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSz, CDialog)
	//{{AFX_MSG_MAP(CSz)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, OnSelchangeTab1)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSz message handlers


BOOL CSz::OnInitDialog()
{
	CDialog::OnInitDialog();

	// �]�m����ܮت��ϼСC�����ε{�ǥD���f���O��ܮخɡA�ج[�N�۰�
	//  ���榹�ާ@
	SetIcon(m_hIcon, TRUE);			// �]�m�j�ϼ�
	SetIcon(m_hIcon, FALSE);


	TCITEM item;
	item.mask = TCIF_TEXT;

	item.pszText = "�n���]�m";
	m_tabctrl.InsertItem (0,&item);

	item.pszText ="�W�u�]�m";
	m_tabctrl.InsertItem (1,&item);


	m_mm1.Create(IDD_SOUND,&m_tabctrl);
	m_mm2.Create(IDD_set,&m_tabctrl);

	m_mm1.Mytest(this);
	m_mm2.Mytest(this);
	
	m_mm1.SetWindowPos(NULL,10,30,400,400,SWP_SHOWWINDOW);
	m_mm2.SetWindowPos(NULL,10,30,400,400,SWP_HIDEWINDOW ); 

	return TRUE;  // ���D�]�m�F���󪺵J�I�A�_�h��^ TRUE
}

void CSz::OnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO:
	CRect r;
	m_tabctrl.GetClientRect (&r);

	switch(m_tabctrl.GetCurSel())
	{
	case 0:
		m_mm1.SetWindowPos(NULL,10,30,r.right -20,r.bottom -40,SWP_SHOWWINDOW);
		m_mm2.SetWindowPos(NULL,10,30,r.right -20,r.bottom -40,SWP_HIDEWINDOW ); 

		break;
	case 1:
		m_mm1.SetWindowPos(NULL,10,30,r.right -20,r.bottom -40,SWP_HIDEWINDOW);
		m_mm2.SetWindowPos(NULL,10,30,r.right -20,r.bottom -40,SWP_SHOWWINDOW );
		break;
	}

	*pResult = 0;
}

void CSz::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // �Ω�ø�s���]�ƤW�U��

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// �ϹϼЦb�u�@�x�Τ��~��
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ø�s�ϼ�
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

BOOL CSz::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
       if(pMsg->message   ==   WM_KEYDOWN)   
          {   
                  switch(pMsg->wParam)   
                  {   
                  case   VK_RETURN://�̽��j��   
                          return   TRUE;   
                  case   VK_ESCAPE://�̽�Esc   
                          return   TRUE;   
                  }  
				  


          }   
          return   CDialog::PreTranslateMessage(pMsg);   

}
