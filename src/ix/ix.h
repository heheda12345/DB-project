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
#include "../rm/rm.h"
#include "ix_error.h"
#include "ix_btree.h"
#include "queue"

class IX_BTKEY;
class IX_BTNode;
class IX_BTree;

//
// IX_FileHdr: Header structure for files
//
class IX_IndexHandle {
    friend class IX_Manager;
    friend class IX_IndexScan;
public:
	IX_IndexHandle  () : hasInit(false) {}           // Constructor
	~IX_IndexHandle () {                             // Destructor
        if (hasInit)
            delete bTree;
    }
    IX_IndexHandle(const IX_IndexHandle&) = delete;
    IX_IndexHandle& operator = (const IX_IndexHandle&) = delete;

	RC InsertEntry     (void *pData, const RID &rid);  // Insert new index entry
	RC DeleteEntry     (void *pData, const RID &rid);  // Delete index entry
	RC ForcePages      ();                             // Copy index to disk
    int getAttrLen() {
        return header.attrLength;
    }

    IX_BTNode get(RID pos);

    IX_BTNode loadRoot();
    void setRoot(const RID& pos);
    
    void update(IX_BTNode& node);

    RID newNode(IX_BTNode& tr);

    int getAttrLen() const {
        return header.attrLength;
    }

    struct Header {
        AttrType attrType;
        int attrLength;
        int btm;
        long long rootPage;
        int rootSlot;
        int nodeSize;
    };
    
    Header getHeader() const {
        return header;
    }
    
    void init();

private:
    RM_FileHandle fh;
    Header header;
    IX_BTree* bTree;
    bool hasInit;
};

//
// IX_IndexScan: condition-based scan of index entries
//
class IX_IndexScan {
public:
	IX_IndexScan  ();                                 // Constructor
	~IX_IndexScan () = default;                       // Destructor
	RC OpenScan      (IX_IndexHandle &indexHandle, // Initialize index scan
					  CompOp      compOp,
					  void        *value,
					  ClientHint  pinHint = NO_HINT);       
	RC GetNextEntry  (RID &rid);                          // Get next matching entry
	RC CloseScan     ();                                  // Terminate index scan
private:
    enum State {
        UNSTART, RUNNING
    } state;
    IX_IndexHandle* handle;
    std::queue<RID> entrys;
    RC search(RID pos, CompOp compOp, IX_BTKEY &key, bool needCheck);
};

//
// IX_Manager: provides IX index file management
//
class IX_Manager {
public:
    ~IX_Manager() = default;

    // Create a new Index
    RC CreateIndex(const char *fileName, int indexNo,
				    AttrType attrType, int attrLength);

    // Destroy and Index
    RC DestroyIndex(const char *fileName, int indexNo);

    // Open an Index
    RC OpenIndex(const char *fileName, int indexNo,
			    IX_IndexHandle &indexHandle);

    // Close an Index
    RC CloseIndex(IX_IndexHandle &indexHandle);

    static IX_Manager& instance() {
        static IX_Manager ins;
        return ins;
    }
    private:
    IX_Manager(): rmm(RM_Manager::instance()) {} // rmm is shorter than RM_Maneger::instance()
    RM_Manager &rmm;
};

#endif // IX_H