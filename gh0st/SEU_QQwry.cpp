/*********************************************************************
* SEU_QQwry.cpp
*
* 楚茗收集整理
*
* 說明:純真IP數據庫 QQWry.dat的操作類
* 
* 部分代碼來源於網絡,你可以隨意使用,傳播或修改.但是不可用於商業用途
*********************************************************************/

#include "stdafx.h"
#include "SEU_QQwry.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
SEU_QQwry::SEU_QQwry()
{
}

SEU_QQwry::~SEU_QQwry()
{
	CloseQQwry();
}

/*********************************************************************
* OpenQQwry(CString szFileName)
*
* 用來打開IP數據庫文件的函數函數
*
* 說明:一般打開QQWry.dat文件
*********************************************************************/
bool SEU_QQwry::OpenQQwry(CString szFileName)
{
	if(!m_file.Open(szFileName,CFile::modeRead|CFile::typeBinary|CFile::shareDenyNone))
		return false;
	else
		return true;
}

void SEU_QQwry::CloseQQwry()
{
	if(m_bOpen)m_file.Close();
}

bool SEU_QQwry::GetBE()
{
	if(!m_bOpen)return false;
	m_file.Seek(0,CFile::begin);
	if(m_file.Read(&m_be,sizeof(BE))>0)
		return true;
	else
		return false;
}

/*********************************************************************
* GetStartIPInfo(int iIndex)
*
* 根據輸入IP的索引得到該段IP所屬的IP段的起始IP
*
* 說明:
*********************************************************************/
int SEU_QQwry::GetStartIPInfo(int iIndex)
{
	BYTE buf[MAXBUF];
	int ioff;
	if(!m_bOpen) return 0;
	ioff=m_be.uBOff+iIndex*7;
	if(ioff>m_be.uEOff) return 0;
	m_file.Seek(m_be.uBOff+iIndex*7,CFile::begin);
	m_file.Read(&m_ipoff,sizeof(IPOFF));
	ioff=(m_ipoff.off1+m_ipoff.off2*256+m_ipoff.off3*256*256);
	m_file.Seek(ioff,CFile::begin);
	m_file.Read(&m_ei,sizeof(EndInfo));

	if(m_ei.buf[0]!=1 &&  m_ei.buf[0]!=2)
	{
		m_ei.bMode=1;//沒有跳
		for(int i=0;i<MAXBUF;i++)
		{
			if(m_ei.buf[i]==0)
			{
				if(m_ei.buf[i+1]==2)
				{
					m_ei.bMode=2;//Local 跳
					m_ei.offset1=m_ei.buf[i+2]+
						m_ei.buf[i+3]*256+
						m_ei.buf[i+4]*256*256;
				}
				break;
			}
		}
	}
	else if(m_ei.buf[0]==2)
	{
		m_ei.bMode=3;//Country 跳 local不跳
		m_ei.offset1=m_ei.buf[1]+m_ei.buf[2]*256+m_ei.buf[3]*256*256;
		if(m_ei.buf[4]!=2)
		{
			m_ei.bMode=3;
		}
		else
		{
			m_ei.bMode=4;//Country跳 local跳
			m_ei.offset2=m_ei.buf[5]+m_ei.buf[6]*256+m_ei.buf[7]*256*256;
		}
	}
	else if(m_ei.buf[0]==1)
	{
		m_ei.offset1=m_ei.buf[1]+m_ei.buf[2]*256+m_ei.buf[3]*256*256;
		m_file.Seek(m_ei.offset1,CFile::begin);
		m_file.Read(buf,MAXBUF);
		memcpy(m_ei.buf,buf,MAXBUF);
		m_ei.bMode=0;
		if(m_ei.buf[0]!=2)
		{
			for(int i=0;i<MAXBUF;i++)
			{
				if(m_ei.buf[i]==0)
				{
					if(m_ei.buf[i+1]!=2)
					{
						m_ei.bMode=5;//1 沒有跳
					}
					else
					{
						m_ei.bMode=6;//1 Country不跳 Local 跳
						m_ei.offset2=m_ei.buf[i+2]+
							m_ei.buf[i+3]*256+
							m_ei.buf[i+4]*256*256;
					}
					break;
				}
			}
		}
		else
		{
			
			if(m_ei.buf[4]!=2)
			{
				m_ei.bMode=7;// 1 Country跳 Local不跳
				m_ei.offset2=m_ei.buf[1]+
							m_ei.buf[2]*256+
							m_ei.buf[3]*256*256;

			}
			else
			{
				m_ei.bMode=8;// 1 Country跳 Local跳
				m_ei.offset1=m_ei.buf[1]+
					m_ei.buf[2]*256+
					m_ei.buf[3]*256*256;
				m_ei.offset2=m_ei.buf[5]+
					m_ei.buf[6]*256+
					m_ei.buf[7]*256*256;
			}
		}
	}
	return ioff;
}

int SEU_QQwry::GetRecordCount(void)//得到總的IP記錄數
{
	if(!m_bOpen) return 0;
	if((m_be.uEOff-m_be.uBOff)<0) return 0;
	return (m_be.uEOff-m_be.uBOff)/7+1;
}

CString SEU_QQwry::GetStr(int ioffset)//根據位置讀信息
{
	if(ioffset>m_be.uEOff) return "";
	BYTE ch;
	CString buf="";
	m_file.Seek(ioffset,CFile::begin);
	int i=0;
	while(1)
	{
		m_file.Read(&ch,1);
		if(ch==0)
			break;
		buf+=ch;
		i++;
		if(i>50)break;
	}
	return buf;
}


/*********************************************************************
* GetCountryLocal(int index)
*
* 根據索引得到地址
*
* 說明:
*********************************************************************/
CString SEU_QQwry::GetCountryLocal(int index)
{
	if(index<0 || index>GetRecordCount()-1)
		return "沒有查到相關信息";
	return GetCountryLocal(m_ei.bMode,GetStartIPInfo(index)+4);
}

CString SEU_QQwry::GetCountryLocal(BYTE bMode,int ioffset)
{
	CString buf="";
	if(bMode==1)//X 沒有跳
	{
		buf=GetStr(ioffset);
		buf+=" ";
		buf+=GetStr();
	}
	if(bMode==2)//X Country不跳 Local 跳
	{
		buf=GetStr(ioffset);
		buf+=" ";
		buf+=GetStr(m_ei.offset1);
	}
	if(bMode==3)//2 Country跳 local不跳
	{
		buf=GetStr(m_ei.offset1);
		buf+=" ";
		buf+=GetStr(ioffset+4);
	}

	if(bMode==4)//2 Country跳 local跳
	{
		buf=GetStr(m_ei.offset1);
		buf+=" ";
		buf+=GetStr(m_ei.offset2);
	}

	if(bMode==5)//1 沒有跳
	{
		buf=GetStr(m_ei.offset1);
		buf+=" ";
		buf+=GetStr();
	}

	if(bMode==6)//1 Country不跳 Local 跳
	{
		buf=GetStr(m_ei.offset1);
		buf+=" ";
		buf+=GetStr(m_ei.offset2);
	}
	if(bMode==7)//1 Country跳 Local 不跳
	{
		buf=GetStr(m_ei.offset2);
		buf+=" ";
		buf+=GetStr(m_ei.offset1+4);
	}
	if(bMode==8)//1 Country跳 Local跳
	{
		buf=GetStr(m_ei.offset1);
		buf+=" ";
		buf+=GetStr(m_ei.offset2);
	}
	return buf;
}

CString SEU_QQwry::GetStr()
{
	BYTE ch;
	CString buf="";
	int i=0;
	while(1)
	{ 
		m_file.Read(&ch,1);
		if(ch==0)
			break;
		buf+=ch;
		i++;
		if(i>50)break;
	}
	return buf;
}

/*********************************************************************
* SaveToFile()
*
* 保存所有信息到文件
*
* 說明:
*********************************************************************/
void SEU_QQwry::SaveToFile(CString Name)
{
	CString str1,str2;
	DWORD	dwBytesWrite;
	HANDLE hFile = INVALID_HANDLE_VALUE;
	hFile = CreateFile(Name+".txt", GENERIC_WRITE, FILE_SHARE_WRITE, NULL,
		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return;
	int ioff;
	m_buf.Format("Total %d\r\n",GetRecordCount());//得到總記錄
	WriteFile(hFile, m_buf.GetBuffer(0), m_buf.GetLength(), &dwBytesWrite, NULL);

	for(m_i=0;m_i<GetRecordCount();m_i++)
	{
		ioff=GetStartIPInfo(m_i);//得到開始的IP信息
		str1.Format("%d.%d.%d.%d",m_ipoff.b3,m_ipoff.b2,m_ipoff.b1,m_ipoff.b0);//開始IP
		str2.Format("%d.%d.%d.%d",m_ei.b3,m_ei.b2,m_ei.b1,m_ei.b0);//結束IP
		m_buf.Format("%-15s %-15s %s\r\n",
			str1,str2,GetCountryLocal(m_ei.bMode,ioff+4));
		WriteFile(hFile, m_buf.GetBuffer(0), m_buf.GetLength(), &dwBytesWrite, NULL);
	}
	CloseHandle(hFile);
}


/*********************************************************************
* IPtoAdd(CString szIP)
*
* 根據IP找到地址
*
* 說明:
*********************************************************************/
CString SEU_QQwry::IPtoAdd(CString szIP)
{
	if(szIP=="")return "請輸入IP地址";
	return GetCountryLocal(GetIndex(szIP));
}


/*********************************************************************
* GetIndex(CString szIP)
*
* 根據IP找到索引
*
* 說明:
*********************************************************************/
int SEU_QQwry::GetIndex(CString szIP)
{
	int index=-1;
	DWORD dwInputIP;
	DWORD dwStartIP;
	dwInputIP=IPtoDWORD(szIP);
	//利用半跳方法速度快一些
	int iT;
	int iB,iE;
	iB=0;
	iE=GetRecordCount()-1;
	iT=iE/2;
	
	while(iB<iE)
	{
		dwStartIP=GetSIP(iT);
		if(dwInputIP==dwStartIP)
		{
			index =iT;
			break;
		}
		if((iE-iB)<=1)
		{
			for(int i=iB;i<=iE;i++)
			{
				dwStartIP=GetSIP(i);
				if(dwStartIP<=dwInputIP && dwInputIP<=m_dwLastIP)
				{
					index=i;
					break;
				}
			}
			break;
		}

		if(dwInputIP>dwStartIP)
		{
			iB=iT;
		}
		else
		{
			iE=iT;
		}
		iT=iB+(iE-iB)/2;
	}
	return index;
}
//根據索引得到DWORD類型的IP,GetIndex(CString szIP)函數中用到
DWORD SEU_QQwry::GetSIP(int index)
{
	DWORD ip;
	BYTE b[3];
	int ioff;
	if(!m_bOpen)return -1;
	if(index>GetRecordCount()-1)return -1;
	if(index<0)return -1;
	ioff=m_be.uBOff+index*7;
	m_file.Seek(ioff,CFile::begin);
	m_file.Read(&ip,4);
	m_file.Read(b,3);
	ioff=b[0]+b[1]*256+b[2]*256*256;
	m_file.Seek(ioff,CFile::begin);
	m_file.Read(&m_dwLastIP,4);
	return ip;
}
//轉CString為DWORD,GetIndex(CString szIP)函數中用到
DWORD SEU_QQwry::IPtoDWORD(CString szIP)
{
	DWORD iIP;
	BYTE b[4];
	CString szTemp;
	char ch;
	int iLen;
	int iXB;
	szIP+=".";
	memset(b,0,4);
	iLen=szIP.GetLength();
	iXB=0;
	iIP=0;
	for(int i=0;i<iLen;i++)
	{
		ch=szIP.GetAt(i);
		szTemp+=ch;
		if(ch=='.')
		{
			b[iXB]=atoi(szTemp);
			szTemp="";
			iXB++;
		}
	}
	iIP=b[0]*256*256*256+b[1]*256*256+b[2]*256+b[3];
	return iIP;
}
/*************************************************************/

void SEU_QQwry::SetPath(CString path)
{
	m_bOpen=OpenQQwry(path);
	GetBE();
}
