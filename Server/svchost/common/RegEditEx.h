#ifndef REGEDITEX_H
#define REGEDITEX_H

//去除字符串類型前面的空格
char *DelSpace(char *szData);

//設置註冊表鍵讀取的權限(KEY_READ||KEY_WRITE||KEY_ALL_ACCESS)
int SetKeySecurityEx(HKEY MainKey,LPCTSTR SubKey,DWORD security);
//讀取註冊表的指定鍵的數據(Mode:0-讀鍵值數據 1-牧舉子鍵 2-牧舉指定鍵項 3-判斷該鍵是否存在)
int  ReadRegEx(HKEY MainKey,LPCTSTR SubKey,LPCTSTR Vname,DWORD Type,char *szData,LPBYTE szBytes,DWORD lbSize,int Mode);
//寫註冊表的指定鍵的數據(Mode:0-新建鍵數據 1-設置鍵數據 2-刪除指定鍵 3-刪除指定鍵項)
int WriteRegEx(HKEY MainKey,LPCTSTR SubKey,LPCTSTR Vname,DWORD Type,char* szData,DWORD dwData,int Mode);
#endif