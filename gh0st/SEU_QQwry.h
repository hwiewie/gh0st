/*********************************************************************
* SEU_QQwry.h
*
* 楚茗收集整理
*
* 說明:純真IP數據庫 QQWry.dat的操作類
* 
* 部分代碼來源於網絡,你可以隨意使用,傳播或修改.但是不可用於商業用途
*********************************************************************/
#include "stdafx.h"
#if !defined(AFX_SEU_QQWRY_H)
#define AFX_SEU_QQWRY_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define MAXBUF 50

typedef struct _tagEndInfo
{
	BYTE b0;
	BYTE b1;
	BYTE b2;
	BYTE b3;
	BYTE buf[MAXBUF];
	BYTE bMode;
	int offset1;
	int offset2;
}EndInfo,PEndInfo;

typedef struct _tagIPOFF
{
	BYTE b0;
	BYTE b1;
	BYTE b2;
	BYTE b3;
	BYTE off1;
	BYTE off2;
	BYTE off3;
}IPOFF,*PIPOFF;

typedef struct _tagBE
{
	int uBOff;
	int uEOff;
}BE,*PBE;

class SEU_QQwry  
{
public://普通用戶接口函數
	void SetPath(CString path);//設置QQWry.dat的路徑
    CString IPtoAdd(CString szIP);//參數是IP,返回IP對應的地址   
	void SaveToFile(CString Name);//將QQWry.dat文件的內容導出為指定的文件

public://高級用戶函數	
	DWORD m_dwLastIP;
	CString GetCountryLocal(int index);
	DWORD GetSIP(int index);
	DWORD IPtoDWORD(CString szIP);
	int GetIndex(CString szIP);
	CString GetStr(void);
	CString GetCountryLocal(BYTE bMode,int ioffset);
	CString GetStr(int ioffset);
	int GetRecordCount(void);
	int m_i;
	int GetStartIPInfo(int iIndex);
	CString m_buf;
	bool GetBE(void);
	bool OpenQQwry(CString szFileName);//打開QQwry數據庫
	void CloseQQwry(void);//關閉QQwry數據庫
	BE m_be;
	IPOFF m_ipoff;
	EndInfo m_ei;
	SEU_QQwry();
	virtual ~SEU_QQwry();
private:
	bool m_bOpen;
	CFile m_file;
};

#endif // !defined(AFX_SEU_QQWRY_H)
