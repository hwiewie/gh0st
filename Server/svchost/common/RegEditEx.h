#ifndef REGEDITEX_H
#define REGEDITEX_H

//�h���r�Ŧ������e�����Ů�
char *DelSpace(char *szData);

//�]�m���U����Ū�����v��(KEY_READ||KEY_WRITE||KEY_ALL_ACCESS)
int SetKeySecurityEx(HKEY MainKey,LPCTSTR SubKey,DWORD security);
//Ū�����U�����w�䪺�ƾ�(Mode:0-Ū��ȼƾ� 1-���|�l�� 2-���|���w�䶵 3-�P�_����O�_�s�b)
int  ReadRegEx(HKEY MainKey,LPCTSTR SubKey,LPCTSTR Vname,DWORD Type,char *szData,LPBYTE szBytes,DWORD lbSize,int Mode);
//�g���U�����w�䪺�ƾ�(Mode:0-�s����ƾ� 1-�]�m��ƾ� 2-�R�����w�� 3-�R�����w�䶵)
int WriteRegEx(HKEY MainKey,LPCTSTR SubKey,LPCTSTR Vname,DWORD Type,char* szData,DWORD dwData,int Mode);
#endif