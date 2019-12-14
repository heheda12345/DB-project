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
#include <queue>
#include <vector>

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

	RC InsertEntry     (const std::vector<std::string> &pData, const RID &rid);  // Insert new index entry
	RC DeleteEntry     (const std::vector<std::string> &pData, const RID &rid);  // Delete index entry
	RC ForcePages      ();                             // Copy index to disk

    IX_BTNode get(RID pos);

    IX_BTNode loadRoot();
    void setRoot(const RID& pos);
    
    void update(IX_BTNode& node);

    RID newNode(IX_BTNode& tr);
    
    void deleteNode(IX_BTNode& tr);

    int getAttrLen() const;

    struct Header {
        std::vector<AttrType> attrType;
        std::vector<int> attrLength;
        int btm;
        long long rootPage;
        int rootSlot;
        int nodeSize;

        int getSize() {
            // printf("[header] getsize %d %d\n", int(attrType.size()), int(sizeof(int) + sizeof(int) + sizeof(long long) + sizeof(int) + sizeof(int) + sizeof(AttrType) * attrType.size() + sizeof(int) * attrLength.size()));
            return sizeof(int) + sizeof(int) + sizeof(long long) + sizeof(int) + sizeof(int)
                   + sizeof(AttrType) * attrType.size() + sizeof(int) * attrLength.size();
        }

        int dump(char* pData) const {
            int cur = 0;
            assert(attrType.size() == attrLength.size());
            *reinterpret_cast<int*>(pData) = (int)attrType.size(); cur += sizeof(int);
            *reinterpret_cast<int*>(pData + cur) = btm; cur += sizeof(int);
            *reinterpret_cast<long long*>(pData + cur) = rootPage; cur += sizeof(long long);
            *reinterpret_cast<int*>(pData + cur) = rootSlot; cur += sizeof(int);
            *reinterpret_cast<int*>(pData + cur) = nodeSize; cur += sizeof(int);
            for (auto& x: attrType) {
                *reinterpret_cast<AttrType*>(pData + cur) = x; cur += sizeof(AttrType);
            }
            for (auto& x: attrLength) {
                *reinterpret_cast<int*>(pData + cur) = x; cur += sizeof(int);
            }
            // printf("[header] dump %d %d (%lld %d) %d\n", (int)attrType.size(), btm, rootPage, rootSlot, nodeSize);
            // printf("%x %x\n", *reinterpret_cast<const int*>(pData), *reinterpret_cast<const int*>(pData + 4));
            return cur;
        }

        int load(const char* pData) {
            int cur = 0;
            int attrSize = *reinterpret_cast<const int*>(pData); cur += sizeof(int);
            btm = *reinterpret_cast<const int*>(pData + cur); cur += sizeof(int);
            rootPage = *reinterpret_cast<const long long*>(pData + cur); cur += sizeof(long long);
            rootSlot = *reinterpret_cast<const int*>(pData + cur); cur += sizeof(int);
            nodeSize = *reinterpret_cast<const int*>(pData + cur); cur += sizeof(int);
            attrType.clear(); attrLength.clear();
            for (int i=0; i<attrSize; i++) {
                attrType.push_back(*reinterpret_cast<const AttrType*>(pData + cur)); cur += sizeof(AttrType);
            }
            for (int i=0; i<attrSize; i++) {
                attrLength.push_back(*reinterpret_cast<const int*>(pData + cur)); cur += sizeof(int);
            }
            printf("[header] load %d %d (%lld %d) %d\n", attrSize, btm, rootPage, rootSlot, nodeSize);
            printf("%x %x\n", *reinterpret_cast<const int*>(pData), *reinterpret_cast<const int*>(pData + 4));
            return cur;
        }
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
					  const std::vector<std::string> &value,
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
				   const std::vector<AttrType> &attrType, const std::vector<int> &attrLength);

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