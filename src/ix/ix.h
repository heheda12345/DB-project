//
// ix.h
//
//   Index Manager Component Interface
//

#ifndef IX_H
#define IX_H

// Please do not include any other files than the ones below in this file.

#include "../redbase.h"  // Please don't change these lines
#include "../rm/rm_rid.h"  // Please don't change these lines
#include "../pf/pf.h"

#define IX_KEYNOTFOUND    (START_IX_WARN + 0)  // cannot find key
#define IX_INVALIDSIZE    (START_IX_WARN + 1)  // invalid entry size
#define IX_ENTRYEXISTS    (START_IX_WARN + 2)  // key,rid already
											   // exists in index
#define IX_NOSUCHENTRY    (START_IX_WARN + 3)  // key,rid combination
											   // does not exist in index

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

#define IX_LASTERROR IX_EOF

//
// IX_FileHdr: Header structure for files
//
class IX_IndexHandle {
public:
	IX_IndexHandle  () = default;                             // Constructor
	~IX_IndexHandle () = default;                             // Destructor
	RC InsertEntry     (void *pData, const RID &rid) {}  // Insert new index entry
	RC DeleteEntry     (void *pData, const RID &rid) {}  // Delete index entry
	RC ForcePages      () {}                             // Copy index to disk
};

//
// IX_IndexScan: condition-based scan of index entries
//
class IX_IndexScan {
public:
	IX_IndexScan  () = default;                                // Constructor
	~IX_IndexScan () = default;                       // Destructor
	RC OpenScan      (const IX_IndexHandle &indexHandle, // Initialize index scan
					  CompOp      compOp,
					  void        *value,
					  ClientHint  pinHint = NO_HINT) {return OK_RC;}          
	RC GetNextEntry  (RID &rid) {return OK_RC;}          // Get next matching entry
	RC CloseScan     () {return OK_RC;}                  // Terminate index scan
};

//
// IX_Manager: provides IX index file management
//
class IX_Manager {
public:
    ~IX_Manager() {}

    // Create a new Index
    RC CreateIndex(const char *fileName, int indexNo,
				    AttrType attrType, int attrLength,
				    int pageSize = PF_PAGE_SIZE) {return OK_RC;}

    // Destroy and Index
    RC DestroyIndex(const char *fileName, int indexNo) {return OK_RC;}

    // Open an Index
    RC OpenIndex(const char *fileName, int indexNo,
			    IX_IndexHandle &indexHandle) {return OK_RC;}

    // Close an Index
    RC CloseIndex(IX_IndexHandle &indexHandle) {return OK_RC;}

    static IX_Manager& instance() {
        static IX_Manager ins;
        return ins;
    }
    private:
    IX_Manager(): pfm(PF_Manager::instance()) {}
    PF_Manager& pfm;
};

void IX_PrintError(RC rc) {}

#endif // IX_H