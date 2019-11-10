//
// ix_file_handle.h
//
//   Index Manager Component Interface
//

#ifndef IX_FILE_HANDLE_H
#define IX_FILE_HANDLE_H

#include "../redbase.h"  // Please don't change these lines
#include "../rm/rm_rid.h"  // Please don't change these lines
#include "../pf/pf.h"
#include "ix_error.h"
//
// IX_FileHdr: Header structure for files
//
class IX_IndexHandle {
  public:
       IX_IndexHandle  ();                             // Constructor
       ~IX_IndexHandle ();                             // Destructor
    RC InsertEntry     (void *pData, const RID &rid);  // Insert new index entry
    RC DeleteEntry     (void *pData, const RID &rid);  // Delete index entry
    RC ForcePages      ();                             // Copy index to disk
};

#endif // #IX_FILE_HANDLE_H