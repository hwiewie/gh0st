#include <windows.h>
//使用Windows的HeapAlloc函數進行動態內存分配
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

   // 聲明SID變量
   SID_NAME_USE   snuType;

   // 聲明和LookupAccountName相關的變量（注意，全為0，要在程序中動態分配）
   TCHAR *        szDomain       = NULL;
   DWORD          cbDomain       = 0;
   LPVOID         pUserSID       = NULL;
   DWORD          cbUserSID      = 0;

   // 和文件相關的安全描述符 SD 的變量
   PSECURITY_DESCRIPTOR pFileSD  = NULL;     // 結構變量
   DWORD          cbFileSD       = 0;        // SD的size

   // 一個新的SD的變量，用於構造新的ACL（把已有的ACL和需要新加的ACL整合起來）
   SECURITY_DESCRIPTOR  newSD;

   // 和ACL 相關的變量
   PACL           pACL           = NULL;
   BOOL           fDaclPresent;
   BOOL           fDaclDefaulted;
   ACL_SIZE_INFORMATION AclInfo;

   // 一個新的 ACL 變量
   PACL           pNewACL        = NULL;  //結構指針變量
   DWORD          cbNewACL       = 0;     //ACL的size

   // 一個臨時使用的 ACE 變量
   LPVOID         pTempAce       = NULL;
   UINT           CurrentAceIndex = 0;  //ACE在ACL中的位置

   UINT           newAceIndex = 0;  //新添的ACE在ACL中的位置

   //API函數的返回值，假設所有的函數都返回失敗。
   BOOL           fResult;
   BOOL           fAPISuccess;

   SECURITY_INFORMATION secInfo = DACL_SECURITY_INFORMATION;

   // 下面的兩個函數是新的API函數，僅在Windows 2000以上版本的操作系統支持。 
   // 在此將從Advapi32.dll文件中動態載入。如果你使用VC++ 6.0編譯程序，而且你想
   // 使用這兩個函數的靜態鏈接。則請為你的編譯加上：/D_WIN32_WINNT=0x0500
   // 的編譯參數。並且確保你的SDK的頭文件和lib文件是最新的。
   SetSecurityDescriptorControlFnPtr _SetSecurityDescriptorControl = NULL;
   AddAccessAllowedAceExFnPtr _AddAccessAllowedAceEx = NULL; 

   __try {

      // 
      // STEP 1: 通過用戶名取得SID
      //     在這一步中LookupAccountName函數被調用了兩次，第一次是取出所需要
      // 的內存的大小，然後，進行內存分配。第二次調用才是取得了用戶的帳戶信息。
      // LookupAccountName同樣可以取得域用戶或是用戶組的信息。（請參看MSDN）
      //

      fAPISuccess = LookupAccountName(NULL, lpszAccountName,
            pUserSID, &cbUserSID, szDomain, &cbDomain, &snuType);

      // 以上調用API會失敗，失敗原因是內存不足。並把所需要的內存大小傳出。
      // 下面是處理非內存不足的錯誤。

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
      // STEP 2: 取得文件（目錄）相關的安全描述符SD
      //     使用GetFileSecurity函數取得一份文件SD的拷貝，同樣，這個函數也
       // 是被調用兩次，第一次同樣是取SD的內存長度。注意，SD有兩種格式：自相關的
       // （self-relative）和 完全的（absolute），GetFileSecurity只能取到“自
       // 相關的”，而SetFileSecurity則需要完全的。這就是為什麼需要一個新的SD，
       // 而不是直接在GetFileSecurity返回的SD上進行修改。因為“自相關的”信息
       // 是不完整的。

      fAPISuccess = GetFileSecurity(lpszFileName, 
            secInfo, pFileSD, 0, &cbFileSD);

      // 以上調用API會失敗，失敗原因是內存不足。並把所需要的內存大小傳出。
      // 下面是處理非內存不足的錯誤。
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
      // STEP 3: 初始化一個新的SD
      // 
      if (!InitializeSecurityDescriptor(&newSD, 
            SECURITY_DESCRIPTOR_REVISION)) {
     //    _tprintf(TEXT("InitializeSecurityDescriptor() failed.")
       //     TEXT("Error %d\n"), GetLastError());
         __leave;
      }

      // 
      // STEP 4: 從GetFileSecurity 返回的SD中取DACL
      // 
      if (!GetSecurityDescriptorDacl(pFileSD, &fDaclPresent, &pACL,
            &fDaclDefaulted)) {
       //  _tprintf(TEXT("GetSecurityDescriptorDacl() failed. Error %d\n"),
       //        GetLastError());
         __leave;
      }

      // 
      // STEP 5: 取 DACL的內存size
      //     GetAclInformation可以提供DACL的內存大小。只傳入一個類型為
      // ACL_SIZE_INFORMATION的structure的參數，需DACL的信息，是為了
      // 方便我們遍歷其中的ACE。
      AclInfo.AceCount = 0; // Assume NULL DACL.
      AclInfo.AclBytesFree = 0;
      AclInfo.AclBytesInUse = sizeof(ACL);

      if (pACL == NULL)
         fDaclPresent = FALSE;

      // 如果DACL不為空，則取其信息。（大多數情況下“自關聯”的DACL為空）
      if (fDaclPresent) {            
         if (!GetAclInformation(pACL, &AclInfo, 
               sizeof(ACL_SIZE_INFORMATION), AclSizeInformation)) {
          //  _tprintf(TEXT("GetAclInformation() failed. Error %d\n"),
          //        GetLastError());
            __leave;
         }
      }

      // 
      // STEP 6: 計算新的ACL的size
      //    計算的公式是：原有的DACL的size加上需要添加的一個ACE的size，以
      // 及加上一個和ACE相關的SID的size，最後減去兩個字節以獲得精確的大小。
      cbNewACL = AclInfo.AclBytesInUse + sizeof(ACCESS_ALLOWED_ACE) 
            + GetLengthSid(pUserSID) - sizeof(DWORD);


      // 
      // STEP 7: 為新的ACL分配內存
      // 
      pNewACL = (PACL) myheapalloc(cbNewACL);
      if (!pNewACL) {
        // _tprintf(TEXT("HeapAlloc() failed. Error %d\n"), GetLastError());
       //  __leave;
      }

      // 
      // STEP 8: 初始化新的ACL結構
      // 
      if (!InitializeAcl(pNewACL, cbNewACL, ACL_REVISION2)) {
        // _tprintf(TEXT("InitializeAcl() failed. Error %d\n"), 
        //       GetLastError());
         __leave;
      }

      // 
      // STEP 9  如果文件（目錄） DACL 有數據，拷貝其中的ACE到新的DACL中
      // 
      //     下面的代碼假設首先檢查指定文件（目錄）是否存在的DACL，如果有的話，
      // 那麼就拷貝所有的ACE到新的DACL結構中，我們可以看到其遍歷的方法是採用
      // ACL_SIZE_INFORMATION結構中的AceCount成員來完成的。在這個循環中，
      // 會按照默認的ACE的順序來進行拷貝（ACE在ACL中的順序是很關鍵的），在拷
      // 貝過程中，先拷貝非繼承的ACE（我們知道ACE會從上層目錄中繼承下來）
      // 

      newAceIndex = 0;

      if (fDaclPresent && AclInfo.AceCount) {

         for (CurrentAceIndex = 0; 
               CurrentAceIndex < AclInfo.AceCount;
               CurrentAceIndex++) {

            // 
            // STEP 10: 從DACL中取ACE
            // 
            if (!GetAce(pACL, CurrentAceIndex, &pTempAce)) {
              // _tprintf(TEXT("GetAce() failed. Error %d\n"), 
              //       GetLastError());
               __leave;
            }

            // 
            // STEP 11: 檢查是否是非繼承的ACE
            //     如果當前的ACE是一個從父目錄繼承來的ACE，那麼就退出循環。
            // 因為，繼承的ACE總是在非繼承的ACE之後，而我們所要添加的ACE
            // 應該在已有的非繼承的ACE之後，所有的繼承的ACE之前。退出循環
            // 正是為了要添加一個新的ACE到新的DACL中，這後，我們再把繼承的
            // ACE拷貝到新的DACL中。
            //
            if (((ACCESS_ALLOWED_ACE *)pTempAce)->Header.AceFlags
               & INHERITED_ACE)
               break;

            // 
            // STEP 12: 檢查要拷貝的ACE的SID是否和需要加入的ACE的SID一樣，
            // 如果一樣，那麼就應該廢掉已存在的ACE，也就是說，同一個用戶的存取
            // 權限的設置的ACE，在DACL中應該唯一。這在裡，跳過對同一用戶已設置
            // 了的ACE，僅是拷貝其它用戶的ACE。
            // 
            if (EqualSid(pUserSID,
               &(((ACCESS_ALLOWED_ACE *)pTempAce)->SidStart)))
               continue;

            // 
            // STEP 13: 把ACE加入到新的DACL中
            //    下面的代碼中，注意 AddAce 函數的第三個參數，這個參數的意思是 
            // ACL中的索引值，意為要把ACE加到某索引位置之後，參數MAXDWORD的
              // 意思是確保當前的ACE是被加入到最後的位置。
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
    // STEP 14: 把一個 access-allowed 的ACE 加入到新的DACL中
    //     前面的循環拷貝了所有的非繼承且SID為其它用戶的ACE，退出循環的第一件事
    // 就是加入我們指定的ACE。請注意首先先動態裝載了一個AddAccessAllowedAceEx
    // 的API函數，如果裝載不成功，就調用AddAccessAllowedAce函數。前一個函數僅
    // 在Windows 2000以後的版本支持，NT則沒有，我們為了使用新版本的函數，我們首
    // 先先檢查一下當前系統中可不可以裝載這個函數，如果可以則就使用。使用動態鏈接
    // 比使用靜態鏈接的好處是，程序運行時不會因為沒有這個API函數而報錯。
    // 
    // Ex版的函數多出了一個參數AceFlag（第三人蔘數），用這個參數我們可以來設置一
    // 個叫ACE_HEADER的結構，以便讓我們所設置的ACE可以被其子目錄所繼承下去，而 
    // AddAccessAllowedAce函數不能定制這個參數，在AddAccessAllowedAce函數
    // 中，其會把ACE_HEADER這個結構設置成非繼承的。
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
      // STEP 15: 按照已存在的ACE的順序拷貝從父目錄繼承而來的ACE
      // 
      if (fDaclPresent && AclInfo.AceCount) {

         for (; 
              CurrentAceIndex < AclInfo.AceCount;
              CurrentAceIndex++) {

            // 
            // STEP 16: 從文件（目錄）的DACL中繼續取ACE
            // 
            if (!GetAce(pACL, CurrentAceIndex, &pTempAce)) {
//                _tprintf(TEXT("GetAce() failed. Error %d\n"), 
//                      GetLastError());
               __leave;
            }

            // 
            // STEP 17: 把ACE加入到新的DACL中
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
      // STEP 18: 把新的ACL設置到新的SD中
      // 
      if (!SetSecurityDescriptorDacl(&newSD, TRUE, pNewACL, 
            FALSE)) {
//          _tprintf(TEXT("SetSecurityDescriptorDacl() failed. Error %d\n"),
//                GetLastError());
         __leave;
      }

      // 
      // STEP 19: 把老的SD中的控制標記再拷貝到新的SD中，我們使用的是一個叫 
      // SetSecurityDescriptorControl() 的API函數，這個函數同樣只存在於
      // Windows 2000以後的版本中，所以我們還是要動態地把其從advapi32.dll 
      // 中載入，如果系統不支持這個函數，那就不拷貝老的SD的控制標記了。
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
      // STEP 20: 把新的SD設置設置到文件的安全屬性中（千山萬水啊，終於到了）
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
      // STEP 21: 釋放已分配的內存，以免Memory Leak
      // 
      if (pUserSID)  myheapfree(pUserSID);
      if (szDomain)  myheapfree(szDomain);
      if (pFileSD) myheapfree(pFileSD);
      if (pNewACL) myheapfree(pNewACL);
   }

   return fResult;
}
