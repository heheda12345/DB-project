  
//
// ix_indexscan.h
//
//   Index Manager Component Interface
//

#ifndef IX_INDEX_SCAN_H
#define IX_INDEX_SCAN_H

// Please do not include any other files than the ones below in this file.

#include "../redbase.h"  // Please don't change these lines
#include "../rm/rm_rid.h"  // Please don't change these lines
#include "../pf/pf.h"
#include "ix_indexhandle.h"

//
// IX_IndexScan: condition-based scan of index entries
//
class IX_IndexScan {
  public:
       IX_IndexScan  ();                                 // Constructor
       ~IX_IndexScan ();                                 // Destructor
    RC OpenScan      (const IX_IndexHandle &indexHandle, // Initialize index scan
                      CompOp      compOp,
                      void        *value,
                      ClientHint  pinHint = NO_HINT);           
    RC GetNextEntry  (RID &rid);                         // Get next matching entry
    RC CloseScan     ();                                 // Terminate index scan
};

#endif