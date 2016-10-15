// ADO.h: interface for the ADO class.
//
//////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
//名稱：ADO類
//功能：將ADO的相關操作封裝成一個類
//作者：謝松(QQ:306496294)
//日期：2009.03.28
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_ADO_H__E57AA002_2E56_4088_96D8_AA98EE73DAA3__INCLUDED_)
#define AFX_ADO_H__E57AA002_2E56_4088_96D8_AA98EE73DAA3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class ADO  
{
public:
	CString DecodeStr(LPCTSTR lpszSrc);
	UINT GetRecordsetCount(_RecordsetPtr pRecordset);			//返回記錄集個數
	void CloseConn();											//關閉連接
	void CloseRecordset();										//關閉記錄集
	_RecordsetPtr& OpenRecordset(CString sql);					//打開記錄集
	void OnInitADOConn();										//初始化COM環境
	_RecordsetPtr m_pRecordset;									//智能指針
	_ConnectionPtr m_pConnection;								//智能指針
	ADO();
	virtual ~ADO();

};

#endif // !defined(AFX_ADO_H__E57AA002_2E56_4088_96D8_AA98EE73DAA3__INCLUDED_)
