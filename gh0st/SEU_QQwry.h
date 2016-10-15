/*********************************************************************
* SEU_QQwry.h
*
* ����������z
*
* ����:�¯uIP�ƾڮw QQWry.dat���ާ@��
* 
* �����N�X�ӷ������,�A�i�H�H�N�ϥ�,�Ǽ��έק�.���O���i�Ω�ӷ~�γ~
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
public://���q�Τᱵ�f���
	void SetPath(CString path);//�]�mQQWry.dat�����|
    CString IPtoAdd(CString szIP);//�ѼƬOIP,��^IP�������a�}   
	void SaveToFile(CString Name);//�NQQWry.dat��󪺤��e�ɥX�����w�����

public://���ťΤ���	
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
	bool OpenQQwry(CString szFileName);//���}QQwry�ƾڮw
	void CloseQQwry(void);//����QQwry�ƾڮw
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
