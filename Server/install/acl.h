#include <windows.h>
//�ϥ�Windows��HeapAlloc��ƶi��ʺA���s���t
#define myheapalloc(x) (HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, x))
#define myheapfree(x)  (HeapFree(GetProcessHeap(), 0, x))

typedef BOOL (WINAPI *SetSecurityDescriptorControlFnPtr)(
   IN PSECURITY_DESCRIPTOR pSecurityDescriptor,
   IN SECURITY_DESCRIPTOR_CONTROL ControlBitsOfInterest,
   IN SECURITY_DESCRIPTOR_CONTROL ControlBitsToSet);

typedef BOOL (WINAPI *AddAccessAllowedAceExFnPtr)(
  PACL pAcl,
  DWORD dwAceRevision,
  DWORD AceFlags,
  DWORD AccessMask,
  PSID pSid
);

BOOL AddAccessRights(TCHAR *lpszFileName, TCHAR *lpszAccountName, 
      DWORD dwAccessMask) {

   // �n��SID�ܶq
   SID_NAME_USE   snuType;

   // �n���MLookupAccountName�������ܶq�]�`�N�A����0�A�n�b�{�Ǥ��ʺA���t�^
   TCHAR *        szDomain       = NULL;
   DWORD          cbDomain       = 0;
   LPVOID         pUserSID       = NULL;
   DWORD          cbUserSID      = 0;

   // �M���������w���y�z�� SD ���ܶq
   PSECURITY_DESCRIPTOR pFileSD  = NULL;     // ���c�ܶq
   DWORD          cbFileSD       = 0;        // SD��size

   // �@�ӷs��SD���ܶq�A�Ω�c�y�s��ACL�]��w����ACL�M�ݭn�s�[��ACL��X�_�ӡ^
   SECURITY_DESCRIPTOR  newSD;

   // �MACL �������ܶq
   PACL           pACL           = NULL;
   BOOL           fDaclPresent;
   BOOL           fDaclDefaulted;
   ACL_SIZE_INFORMATION AclInfo;

   // �@�ӷs�� ACL �ܶq
   PACL           pNewACL        = NULL;  //���c���w�ܶq
   DWORD          cbNewACL       = 0;     //ACL��size

   // �@���{�ɨϥΪ� ACE �ܶq
   LPVOID         pTempAce       = NULL;
   UINT           CurrentAceIndex = 0;  //ACE�bACL������m

   UINT           newAceIndex = 0;  //�s�K��ACE�bACL������m

   //API��ƪ���^�ȡA���]�Ҧ�����Ƴ���^���ѡC
   BOOL           fResult;
   BOOL           fAPISuccess;

   SECURITY_INFORMATION secInfo = DACL_SECURITY_INFORMATION;

   // �U������Ө�ƬO�s��API��ơA�ȦbWindows 2000�H�W�������ާ@�t�Τ���C 
   // �b���N�qAdvapi32.dll��󤤰ʺA���J�C�p�G�A�ϥ�VC++ 6.0�sĶ�{�ǡA�ӥB�A�Q
   // �ϥγo��Ө�ƪ��R�A�챵�C�h�Ь��A���sĶ�[�W�G/D_WIN32_WINNT=0x0500
   // ���sĶ�ѼơC�åB�T�O�A��SDK���Y���Mlib���O�̷s���C
   SetSecurityDescriptorControlFnPtr _SetSecurityDescriptorControl = NULL;
   AddAccessAllowedAceExFnPtr _AddAccessAllowedAceEx = NULL; 

   __try {

      // 
      // STEP 1: �q�L�Τ�W���oSID
      //     �b�o�@�B��LookupAccountName��ƳQ�եΤF�⦸�A�Ĥ@���O���X�һݭn
      // �����s���j�p�A�M��A�i�椺�s���t�C�ĤG���եΤ~�O���o�F�Τ᪺�b��H���C
      // LookupAccountName�P�˥i�H���o��Τ�άO�Τ�ժ��H���C�]�аѬ�MSDN�^
      //

      fAPISuccess = LookupAccountName(NULL, lpszAccountName,
            pUserSID, &cbUserSID, szDomain, &cbDomain, &snuType);

      // �H�W�ե�API�|���ѡA���ѭ�]�O���s�����C�ç�һݭn�����s�j�p�ǥX�C
      // �U���O�B�z�D���s���������~�C

      if (fAPISuccess)
         __leave;
      else if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
//          _tprintf(TEXT("LookupAccountName() failed. Error %d\n"), 
//                GetLastError());
         __leave;
      }

      pUserSID = myheapalloc(cbUserSID);
      if (!pUserSID) {
//         _tprintf(TEXT("HeapAlloc() failed. Error %d\n"), GetLastError());
         __leave;
      }

      szDomain = (TCHAR *) myheapalloc(cbDomain * sizeof(TCHAR));
      if (!szDomain) {
 //        _tprintf(TEXT("HeapAlloc() failed. Error %d\n"), GetLastError());
         __leave;
      }

      fAPISuccess = LookupAccountName(NULL, lpszAccountName,
            pUserSID, &cbUserSID, szDomain, &cbDomain, &snuType);
      if (!fAPISuccess) {
  //       _tprintf(TEXT("LookupAccountName() failed. Error %d\n"), 
        //       GetLastError());
         __leave;
      }

      // 
      // STEP 2: ���o���]�ؿ��^�������w���y�z��SD
      //     �ϥ�GetFileSecurity��ƨ��o�@�����SD�������A�P�ˡA�o�Ө�Ƥ]
       // �O�Q�եΨ⦸�A�Ĥ@���P�ˬO��SD�����s���סC�`�N�ASD����خ榡�G�۬�����
       // �]self-relative�^�M �������]absolute�^�AGetFileSecurity�u����졧��
       // ���������A��SetFileSecurity�h�ݭn�������C�o�N�O������ݭn�@�ӷs��SD�A
       // �Ӥ��O�����bGetFileSecurity��^��SD�W�i��ק�C�]�����۬��������H��
       // �O�����㪺�C

      fAPISuccess = GetFileSecurity(lpszFileName, 
            secInfo, pFileSD, 0, &cbFileSD);

      // �H�W�ե�API�|���ѡA���ѭ�]�O���s�����C�ç�һݭn�����s�j�p�ǥX�C
      // �U���O�B�z�D���s���������~�C
      if (fAPISuccess)
         __leave;
      else if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
  //       _tprintf(TEXT("GetFileSecurity() failed. Error %d\n"), 
       //        GetLastError());
         __leave;
      }

      pFileSD = myheapalloc(cbFileSD);
      if (!pFileSD) {
 //        _tprintf(TEXT("HeapAlloc() failed. Error %d\n"), GetLastError());
         __leave;
      }

      fAPISuccess = GetFileSecurity(lpszFileName, 
            secInfo, pFileSD, cbFileSD, &cbFileSD);
      if (!fAPISuccess) {
    //     _tprintf(TEXT("GetFileSecurity() failed. Error %d\n"), 
     //          GetLastError());
         __leave;
      }

      // 
      // STEP 3: ��l�Ƥ@�ӷs��SD
      // 
      if (!InitializeSecurityDescriptor(&newSD, 
            SECURITY_DESCRIPTOR_REVISION)) {
     //    _tprintf(TEXT("InitializeSecurityDescriptor() failed.")
       //     TEXT("Error %d\n"), GetLastError());
         __leave;
      }

      // 
      // STEP 4: �qGetFileSecurity ��^��SD����DACL
      // 
      if (!GetSecurityDescriptorDacl(pFileSD, &fDaclPresent, &pACL,
            &fDaclDefaulted)) {
       //  _tprintf(TEXT("GetSecurityDescriptorDacl() failed. Error %d\n"),
       //        GetLastError());
         __leave;
      }

      // 
      // STEP 5: �� DACL�����ssize
      //     GetAclInformation�i�H����DACL�����s�j�p�C�u�ǤJ�@��������
      // ACL_SIZE_INFORMATION��structure���ѼơA��DACL���H���A�O���F
      // ��K�ڭ̹M���䤤��ACE�C
      AclInfo.AceCount = 0; // Assume NULL DACL.
      AclInfo.AclBytesFree = 0;
      AclInfo.AclBytesInUse = sizeof(ACL);

      if (pACL == NULL)
         fDaclPresent = FALSE;

      // �p�GDACL�����šA�h����H���C�]�j�h�Ʊ��p�U�������p����DACL���š^
      if (fDaclPresent) {            
         if (!GetAclInformation(pACL, &AclInfo, 
               sizeof(ACL_SIZE_INFORMATION), AclSizeInformation)) {
          //  _tprintf(TEXT("GetAclInformation() failed. Error %d\n"),
          //        GetLastError());
            __leave;
         }
      }

      // 
      // STEP 6: �p��s��ACL��size
      //    �p�⪺�����O�G�즳��DACL��size�[�W�ݭn�K�[���@��ACE��size�A�H
      // �Υ[�W�@�өMACE������SID��size�A�̫��h��Ӧr�`�H��o��T���j�p�C
      cbNewACL = AclInfo.AclBytesInUse + sizeof(ACCESS_ALLOWED_ACE) 
            + GetLengthSid(pUserSID) - sizeof(DWORD);


      // 
      // STEP 7: ���s��ACL���t���s
      // 
      pNewACL = (PACL) myheapalloc(cbNewACL);
      if (!pNewACL) {
        // _tprintf(TEXT("HeapAlloc() failed. Error %d\n"), GetLastError());
       //  __leave;
      }

      // 
      // STEP 8: ��l�Ʒs��ACL���c
      // 
      if (!InitializeAcl(pNewACL, cbNewACL, ACL_REVISION2)) {
        // _tprintf(TEXT("InitializeAcl() failed. Error %d\n"), 
        //       GetLastError());
         __leave;
      }

      // 
      // STEP 9  �p�G���]�ؿ��^ DACL ���ƾڡA�����䤤��ACE��s��DACL��
      // 
      //     �U�����N�X���]�����ˬd���w���]�ؿ��^�O�_�s�b��DACL�A�p�G�����ܡA
      // ����N�����Ҧ���ACE��s��DACL���c���A�ڭ̥i�H�ݨ��M������k�O�ĥ�
      // ACL_SIZE_INFORMATION���c����AceCount�����ӧ������C�b�o�Ӵ`�����A
      // �|�����q�{��ACE�����ǨӶi������]ACE�bACL�������ǬO�����䪺�^�A�b��
      // ���L�{���A�������D�~�Ӫ�ACE�]�ڭ̪��DACE�|�q�W�h�ؿ����~�ӤU�ӡ^
      // 

      newAceIndex = 0;

      if (fDaclPresent && AclInfo.AceCount) {

         for (CurrentAceIndex = 0; 
               CurrentAceIndex < AclInfo.AceCount;
               CurrentAceIndex++) {

            // 
            // STEP 10: �qDACL����ACE
            // 
            if (!GetAce(pACL, CurrentAceIndex, &pTempAce)) {
              // _tprintf(TEXT("GetAce() failed. Error %d\n"), 
              //       GetLastError());
               __leave;
            }

            // 
            // STEP 11: �ˬd�O�_�O�D�~�Ӫ�ACE
            //     �p�G��e��ACE�O�@�ӱq���ؿ��~�ӨӪ�ACE�A����N�h�X�`���C
            // �]���A�~�Ӫ�ACE�`�O�b�D�~�Ӫ�ACE����A�ӧڭ̩ҭn�K�[��ACE
            // ���Ӧb�w�����D�~�Ӫ�ACE����A�Ҧ����~�Ӫ�ACE���e�C�h�X�`��
            // ���O���F�n�K�[�@�ӷs��ACE��s��DACL���A�o��A�ڭ̦A���~�Ӫ�
            // ACE������s��DACL���C
            //
            if (((ACCESS_ALLOWED_ACE *)pTempAce)->Header.AceFlags
               & INHERITED_ACE)
               break;

            // 
            // STEP 12: �ˬd�n������ACE��SID�O�_�M�ݭn�[�J��ACE��SID�@�ˡA
            // �p�G�@�ˡA����N���Ӽo���w�s�b��ACE�A�]�N�O���A�P�@�ӥΤ᪺�s��
            // �v�����]�m��ACE�A�bDACL�����Ӱߤ@�C�o�b�̡A���L��P�@�Τ�w�]�m
            // �F��ACE�A�ȬO�����䥦�Τ᪺ACE�C
            // 
            if (EqualSid(pUserSID,
               &(((ACCESS_ALLOWED_ACE *)pTempAce)->SidStart)))
               continue;

            // 
            // STEP 13: ��ACE�[�J��s��DACL��
            //    �U�����N�X���A�`�N AddAce ��ƪ��ĤT�ӰѼơA�o�ӰѼƪ��N��O 
            // ACL�������ޭȡA�N���n��ACE�[��Y���ަ�m����A�Ѽ�MAXDWORD��
              // �N��O�T�O��e��ACE�O�Q�[�J��̫᪺��m�C
            //
            if (!AddAce(pNewACL, ACL_REVISION, MAXDWORD, pTempAce,
                  ((PACE_HEADER) pTempAce)->AceSize)) {
              // _tprintf(TEXT("AddAce() failed. Error %d\n"), 
               //      GetLastError());
               __leave;
            }

            newAceIndex++;
         }
      }

// 
    // STEP 14: ��@�� access-allowed ��ACE �[�J��s��DACL��
    //     �e�����`�������F�Ҧ����D�~�ӥBSID���䥦�Τ᪺ACE�A�h�X�`�����Ĥ@���
    // �N�O�[�J�ڭ̫��w��ACE�C�Ъ`�N�������ʺA�˸��F�@��AddAccessAllowedAceEx
    // ��API��ơA�p�G�˸������\�A�N�ե�AddAccessAllowedAce��ơC�e�@�Ө�ƶ�
    // �bWindows 2000�H�᪺��������ANT�h�S���A�ڭ̬��F�ϥηs��������ơA�ڭ̭�
    // �����ˬd�@�U��e�t�Τ��i���i�H�˸��o�Ө�ơA�p�G�i�H�h�N�ϥΡC�ϥΰʺA�챵
    // ��ϥ��R�A�챵���n�B�O�A�{�ǹB��ɤ��|�]���S���o��API��Ʀӳ����C
    // 
    // Ex������Ʀh�X�F�@�ӰѼ�AceFlag�]�ĤT�H�x�ơ^�A�γo�ӰѼƧڭ̥i�H�ӳ]�m�@
    // �ӥsACE_HEADER�����c�A�H�K���ڭ̩ҳ]�m��ACE�i�H�Q��l�ؿ����~�ӤU�h�A�� 
    // AddAccessAllowedAce��Ƥ���w��o�ӰѼơA�bAddAccessAllowedAce���
    // ���A��|��ACE_HEADER�o�ӵ��c�]�m���D�~�Ӫ��C
    // 
      _AddAccessAllowedAceEx = (AddAccessAllowedAceExFnPtr)
            GetProcAddress(GetModuleHandle(TEXT("advapi32.dll")),
            "AddAccessAllowedAceEx");

      if (_AddAccessAllowedAceEx) {
           if (!_AddAccessAllowedAceEx(pNewACL, ACL_REVISION2,
              CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE ,
                dwAccessMask, pUserSID)) {
            // _tprintf(TEXT("AddAccessAllowedAceEx() failed. Error %d\n"),
            //       GetLastError());
             __leave;
          }
      }else{
          if (!AddAccessAllowedAce(pNewACL, ACL_REVISION2, 
                dwAccessMask, pUserSID)) {
//              _tprintf(TEXT("AddAccessAllowedAce() failed. Error %d\n"),
//                    GetLastError());
             __leave;
          }
      }

      // 
      // STEP 15: ���Ӥw�s�b��ACE�����ǫ����q���ؿ��~�ӦӨӪ�ACE
      // 
      if (fDaclPresent && AclInfo.AceCount) {

         for (; 
              CurrentAceIndex < AclInfo.AceCount;
              CurrentAceIndex++) {

            // 
            // STEP 16: �q���]�ؿ��^��DACL���~���ACE
            // 
            if (!GetAce(pACL, CurrentAceIndex, &pTempAce)) {
//                _tprintf(TEXT("GetAce() failed. Error %d\n"), 
//                      GetLastError());
               __leave;
            }

            // 
            // STEP 17: ��ACE�[�J��s��DACL��
            // 
            if (!AddAce(pNewACL, ACL_REVISION, MAXDWORD, pTempAce,
                  ((PACE_HEADER) pTempAce)->AceSize)) {
//                _tprintf(TEXT("AddAce() failed. Error %d\n"), 
//                      GetLastError());
               __leave;
            }
         }
      }

      // 
      // STEP 18: ��s��ACL�]�m��s��SD��
      // 
      if (!SetSecurityDescriptorDacl(&newSD, TRUE, pNewACL, 
            FALSE)) {
//          _tprintf(TEXT("SetSecurityDescriptorDacl() failed. Error %d\n"),
//                GetLastError());
         __leave;
      }

      // 
      // STEP 19: ��Ѫ�SD��������аO�A������s��SD���A�ڭ̨ϥΪ��O�@�ӥs 
      // SetSecurityDescriptorControl() ��API��ơA�o�Ө�ƦP�˥u�s�b��
      // Windows 2000�H�᪺�������A�ҥH�ڭ��٬O�n�ʺA�a���qadvapi32.dll 
      // �����J�A�p�G�t�Τ�����o�Ө�ơA���N�������Ѫ�SD������аO�F�C
      // 
      _SetSecurityDescriptorControl =(SetSecurityDescriptorControlFnPtr)
            GetProcAddress(GetModuleHandle(TEXT("advapi32.dll")),
            "SetSecurityDescriptorControl");
      if (_SetSecurityDescriptorControl) {

         SECURITY_DESCRIPTOR_CONTROL controlBitsOfInterest = 0;
         SECURITY_DESCRIPTOR_CONTROL controlBitsToSet = 0;
         SECURITY_DESCRIPTOR_CONTROL oldControlBits = 0;
         DWORD dwRevision = 0;

         if (!GetSecurityDescriptorControl(pFileSD, &oldControlBits,
            &dwRevision)) {
//             _tprintf(TEXT("GetSecurityDescriptorControl() failed.")
//                   TEXT("Error %d\n"), GetLastError());
            __leave;
         }

         if (oldControlBits & SE_DACL_AUTO_INHERITED) {
            controlBitsOfInterest =
               SE_DACL_AUTO_INHERIT_REQ |
               SE_DACL_AUTO_INHERITED ;
            controlBitsToSet = controlBitsOfInterest;
         }
         else if (oldControlBits & SE_DACL_PROTECTED) {
            controlBitsOfInterest = SE_DACL_PROTECTED;
            controlBitsToSet = controlBitsOfInterest;
         }        

         if (controlBitsOfInterest) {
            if (!_SetSecurityDescriptorControl(&newSD,
               controlBitsOfInterest,
               controlBitsToSet)) {
//                _tprintf(TEXT("SetSecurityDescriptorControl() failed.")
//                      TEXT("Error %d\n"), GetLastError());
               __leave;
            }
         }
      }

      // 
      // STEP 20: ��s��SD�]�m�]�m���󪺦w���ݩʤ��]�d�s�U���ڡA�ש��F�^
      // 
      if (!SetFileSecurity(lpszFileName, secInfo,
            &newSD)) {
//          _tprintf(TEXT("SetFileSecurity() failed. Error %d\n"), 
//                GetLastError());
         __leave;
      }

      fResult = TRUE;

   } __finally {

      // 
      // STEP 21: ����w���t�����s�A�H�KMemory Leak
      // 
      if (pUserSID)  myheapfree(pUserSID);
      if (szDomain)  myheapfree(szDomain);
      if (pFileSD) myheapfree(pFileSD);
      if (pNewACL) myheapfree(pNewACL);
   }

   return fResult;
}
