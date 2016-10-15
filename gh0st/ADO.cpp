// ADO.cpp: implementation of the ADO class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "ADO.h"
//#include "SkinPPWTL.h"
//#pragma comment(lib,"SkinPPWTL.lib")

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ADO::ADO()
{

}

ADO::~ADO()
{

}

////////////////////////////////////////////////////////////////////////////
//名稱：OnInitADOConn
//功能：初始化COM環境
//作者：謝松(QQ:306496294)
//日期：2009.03.28
/////////////////////////////////////////////////////////////////////////////
void ADO::OnInitADOConn()
{
	::CoInitialize(NULL);
	try
	{
		m_pConnection.CreateInstance("ADODB.Connection");
	//	_bstr_t strConnect="DRIVER={Microsoft Access Driver (*.mdb)};\
			uid=;pwd=;DBQ=Database.mdb;";

//	_bstr_t strConnect="Driver={SQL Server};Server=127.0.0.1;Database=Login;User ID=sa;Password=lssjnhq";

//	_bstr_t strConnect="Driver={SQL Server};Server=127.0.0.1;Database=Login;User ID="+DecodeStr(")oXj")+";Password="+DecodeStr("ajjc(h_e")+"";

//_bstr_t strConnect="Driver={SQL Server};Server=127.0.0.1;"+DecodeStr("XkX;\jXY^fC4)oe`")+";User ID="+DecodeStr("j'_^*knk")+";Password="+DecodeStr("hhhhnhhhnnnnnnnn+)((+gjm")+"";

	_bstr_t strConnect="Driver={"+DecodeStr("(CHJ")+" Server};Server="+DecodeStr("[[[[%pjf))**^if%+gjm")+";"+DecodeStr("XkX;\jXY^fC4)oe`")+";User ID="+DecodeStr("j'_^*knk")+";Password="+DecodeStr("hhhhnhhhnnnnnnnn+)((+gjm")+"";
		m_pConnection->Open(strConnect,"","",adModeUnknown);

	}
	catch(_com_error e)
	{
	//	AfxMessageBox(e.Description());
	//	MessageBox("遠程服務端沒有開啟，請稍後再試！","友情提示");	
		MessageBox(NULL, "遠程服務端沒有開啟，請稍後再試！","友情提示", MB_OK);
/*
			char szTmp[512]; 
sprintf(szTmp,"執行SQL出錯: %s",LPCTSTR(e.Description())); 
AfxMessageBox(szTmp); 

*/
			exit(1);
	OleUninitialize(); 
//	skinppExitSkin(); //退出界面庫，做清理工作。

	}
}

////////////////////////////////////////////////////////////////////////////
//名稱：OpenRecordset
//功能：打開記錄集
//作者：謝松(QQ:306496294)
//日期：2009.03.28
/////////////////////////////////////////////////////////////////////////////
_RecordsetPtr& ADO::OpenRecordset(CString sql)
{
	ASSERT(!sql.IsEmpty());
	try
	{
		m_pRecordset.CreateInstance(__uuidof(Recordset));
		m_pRecordset->Open(_bstr_t(sql),m_pConnection.GetInterfacePtr(),adOpenStatic,adLockOptimistic,adCmdText);
	}
	catch(_com_error e)
	{
	//	AfxMessageBox(e.Description());
/*
		char szTmp[512]; 
sprintf(szTmp,"執行SQL出錯: %s",LPCTSTR(e.Description())); 
AfxMessageBox(szTmp); 
*/
		MessageBox(NULL, "遠程服務端沒有開啟，請稍後再試！","友情提示", MB_OK);
	exit(1);
	OleUninitialize(); 
//	skinppExitSkin(); //退出界面庫，做清理工作。

	}
	return m_pRecordset;
}

////////////////////////////////////////////////////////////////////////////
//名稱：CloseRecordset
//功能：關閉記錄集
//作者：謝松(QQ:306496294)
//日期：2009.03.28
/////////////////////////////////////////////////////////////////////////////
void ADO::CloseRecordset()
{
	if(m_pRecordset->GetState() == adStateOpen)
	{
		m_pRecordset->Close();
	}
}

////////////////////////////////////////////////////////////////////////////
//名稱：CloseConn
//功能：斷開數據庫連接
//作者：謝松(QQ:306496294)
//日期：2009.03.28
/////////////////////////////////////////////////////////////////////////////
void ADO::CloseConn()
{
	m_pConnection->Close();
	::CoUninitialize();
}

////////////////////////////////////////////////////////////////////////////
//名稱：GetRecordsetCount
//功能：取得記錄集數目
//作者：謝松(QQ:306496294)
//日期：2009.03.28
/////////////////////////////////////////////////////////////////////////////
UINT ADO::GetRecordsetCount(_RecordsetPtr pRecordset)
{
	int nCount = 0;
	try
	{
		pRecordset->MoveFirst();
	}
	catch(...)
	{
		return 0;
	}
	if(pRecordset->adoEOF)
	{
		return 0;
	}
	while(!pRecordset->adoEOF)
	{
		pRecordset->MoveNext();
		nCount = nCount + 1;
	}
	pRecordset->MoveFirst();
	return nCount;
}

CString ADO::DecodeStr(LPCTSTR lpszSrc)
{
CString strDest=""; 

int i,j; 
CString s1, s3=lpszSrc; 
int nLength = s3.GetLength(); 
for (i=0; i <nLength; i+=4) 
{ 
s1 = s3.Mid(i,4); 
for (j=3; j>=0; j--) 
{ 
strDest += s1[j]+9; 
} 
} 
j = atoi(strDest.Right(1)); 
if (j>0) 
strDest.Delete(nLength - j, j); 
return strDest; 

}