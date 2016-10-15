

class CRegistry
{
public:
	CRegistry(HKEY hKey=HKEY_LOCAL_MACHINE)
	{ 
		m_MainKey = hKey;
		m_hKey = hKey;
	}
	virtual ~CRegistry(){}

	BOOL ReSetMainKey(HKEY hKey)
	{
		if (hKey == NULL)
			return FALSE;
		m_MainKey = hKey;
		return TRUE;
	}

	BOOL CreateKey(LPCTSTR lpSubKey)
	{
		HKEY hKey;
		DWORD dw;
		long lReturn=RegCreateKeyEx(m_hKey,lpSubKey,0L,NULL,REG_OPTION_VOLATILE,KEY_ALL_ACCESS,NULL,&hKey,&dw);
    
		if(lReturn==ERROR_SUCCESS)
		{
			m_hKey=hKey;
			return TRUE;
		}
		
		return FALSE;
		
	}

	BOOL Open(LPCTSTR lpSubKey)
	{
		HKEY hKey;
		long lReturn=RegOpenKeyEx(m_MainKey,lpSubKey,0L,KEY_ALL_ACCESS,&hKey);
    
		if(lReturn==ERROR_SUCCESS)
		{
			m_hKey=hKey;
			return TRUE;
		}
		return FALSE;
		
	}

	void Close()
	{
		if(m_hKey)
		{
			RegCloseKey(m_hKey);
			m_hKey=NULL;
		}
		
	}

	BOOL DeleteValue(LPCTSTR lpValueName)
	{
		long lReturn=RegDeleteValue(m_hKey,lpValueName);
		
		if(lReturn==ERROR_SUCCESS)
			return TRUE;
		return FALSE;
		
	}

	BOOL DeleteKey(HKEY hKey, LPCTSTR lpSubKey)
	{
		long lReturn=RegDeleteValue(hKey,lpSubKey);
		
		if(lReturn==ERROR_SUCCESS)
			return TRUE;
		return FALSE;
		
	}

	BOOL EnumKey(DWORD dwIndex, char *szSubKey ,DWORD dwBufSize)
	{
		long lReturn = RegEnumKeyEx(m_hKey, dwIndex, szSubKey, &dwBufSize, NULL, NULL, NULL, NULL);
		if(lReturn==ERROR_SUCCESS)
		{
			return TRUE;
		}
		return FALSE;
	}

	BOOL WriteDword(LPCTSTR lpSubKey, DWORD dwVal)
	{
		long lReturn=RegSetValueEx(m_hKey,lpSubKey,0L,REG_DWORD,(const BYTE *) &dwVal,sizeof(DWORD));
		
   		if(lReturn==ERROR_SUCCESS)
			return TRUE;
		
		return FALSE;
		
	}

	BOOL WriteString(LPCTSTR lpValueName, LPCTSTR lpValue)
	{
		long lReturn=RegSetValueEx(m_hKey,lpValueName,0L,REG_SZ,(const BYTE *) lpValue,strlen(lpValue)+1);
		
   		if(lReturn==ERROR_SUCCESS)
			return TRUE;
		
		return FALSE;
		
	}
	DWORD ReadDword(LPCTSTR lpValueName)
	{
		DWORD dwType;
		DWORD dwSize=sizeof(DWORD);
		DWORD dwDest;
		long lReturn=RegQueryValueEx(m_hKey,lpValueName,NULL,&dwType,(BYTE *)&dwDest,&dwSize);
		
		if(lReturn==ERROR_SUCCESS)
		{
			return dwDest;
		}
		return FALSE;
		
	}

	BOOL RestoreKey(LPCTSTR lpFileName)
	{
		long lReturn=RegRestoreKey(m_hKey,lpFileName,REG_WHOLE_HIVE_VOLATILE);
		
		if(lReturn==ERROR_SUCCESS)
			return TRUE;
		return FALSE;
	}

	BOOL SaveKey(LPCTSTR lpFileName)
	{
		long lReturn=RegSaveKey(m_hKey,lpFileName,NULL);
		
		if(lReturn==ERROR_SUCCESS)
			return TRUE;
		
		return FALSE;
	}

	char* ReadString(LPCTSTR lpValueName)
	{
		DWORD dwType;
		DWORD dwSize=200;
		char szString[2550];
		char *lpVal;
		long lReturn=RegQueryValueEx(m_hKey,lpValueName,NULL,&dwType,(BYTE *)szString,&dwSize);
		
		if(lReturn==ERROR_SUCCESS)
		{
			lpVal=szString;
			return lpVal;
		}
		return NULL;
	}

protected:
	HKEY m_hKey;
	HKEY m_MainKey;
};