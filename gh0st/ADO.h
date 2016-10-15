// ADO.h: interface for the ADO class.
//
//////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
//�W�١GADO��
//�\��G�NADO�������ާ@�ʸ˦��@����
//�@�̡G�ªQ(QQ:306496294)
//����G2009.03.28
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
	UINT GetRecordsetCount(_RecordsetPtr pRecordset);			//��^�O�����Ӽ�
	void CloseConn();											//�����s��
	void CloseRecordset();										//�����O����
	_RecordsetPtr& OpenRecordset(CString sql);					//���}�O����
	void OnInitADOConn();										//��l��COM����
	_RecordsetPtr m_pRecordset;									//������w
	_ConnectionPtr m_pConnection;								//������w
	ADO();
	virtual ~ADO();

};

#endif // !defined(AFX_ADO_H__E57AA002_2E56_4088_96D8_AA98EE73DAA3__INCLUDED_)
