#pragma once
#include "../redbase.h"

#define IX_KEYNOTFOUND    (START_IX_WARN + 0)  // cannot find key
#define IX_INVALIDSIZE    (START_IX_WARN + 1)  // invalid entry size
#define IX_ENTRYEXISTS    (START_IX_WARN + 2)  // key,rid already exists in index
#define IX_NOSUCHENTRY    (START_IX_WARN + 3)  // key,rid combination does not exist in index

#define IX_LASTWARN IX_ENTRYEXISTS


#define IX_SIZETOOBIG      (START_IX_ERR - 0)  // key size too big
#define IX_PF              (START_IX_ERR - 1)  // error in PF
#define IX_BADIXPAGE       (START_IX_ERR - 2)  
#define IX_FCREATEFAIL     (START_IX_ERR - 3)  // record size mismatch
#define IX_HANDLEOPEN      (START_IX_ERR - 4)
#define IX_BADOPEN         (START_IX_ERR - 5)
#define IX_FNOTOPEN        (START_IX_ERR - 6)
#define IX_BADRID          (START_IX_ERR - 7)
#define IX_BADKEY          (START_IX_ERR - 8)
#define IX_EOF             (START_IX_ERR - 9)  // end of file
#define IX_RM              (START_IX_ERR - 10)
#define IX_BTREE           (START_IX_ERR - 11)
#define IX_SCAN_OPENED     (START_IX_ERR - 12)
#define IX_SCAN_CLOSED     (START_IX_ERR - 13)
#define IX_SCAN_END        (START_IX_ERR - 14)
#define IX_INVALIDCOMP     (START_IX_ERR - 15)
#define IX_LASTERROR IX_EOF

void IX_PrintError(RC rc);

#define IXRC(rc, ret_rc) { \
   if (rc != 0) { \
      IX_PrintError(rc); \
      return rc > 0 ? ret_rc : -ret_rc;  \
   } \
}
