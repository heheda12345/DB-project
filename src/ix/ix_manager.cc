#include "ix.h"
#include <cstring>
#include <string>
using namespace std;

// Create a new Index
RC IX_Manager::CreateIndex(const char *fileName, int indexNo,
                           const std::vector<AttrType> &attrType, const std::vector<int> &attrLength) {
    string name = string(fileName) + "." + to_string(indexNo);
    IX_IndexHandle::Header header;
    header.attrLength = attrLength;
    header.attrType = attrType;
    for (auto type: attrType) {
        assert(type != DATE);
    }
    header.btm = 3;
    header.nodeSize = IX_BTNode::getSize(header.attrLength, header.btm);
    char headerPool[header.getSize()];
    int size = header.dump(headerPool); assert(size == header.getSize());
    RC rc = rmm.CreateFile(name.c_str(), header.nodeSize, headerPool, size);
    RMRC(rc, IX_RM);

    RM_FileHandle handle;
    rc = rmm.OpenFile(name.c_str(), handle);
    RMRC(rc, IX_RM);
    IX_BTNode root;
    char data[header.nodeSize];
    root.dump(data, header.attrLength, header.btm);
    RID rid;
    rc = handle.InsertRec(data, rid);
    RMRC(rc, IX_RM);
    header.rootPage = rid.GetPageNum();
    header.rootSlot = rid.GetSlotNum();
    char dataHeader[header.getSize()];
    size = header.dump(dataHeader); assert(size == header.getSize());
    rc = handle.SetMeta(dataHeader, size);
    // printf("save header(%d) %lld %d %d\n", header.getSize(), header.rootPage, header.rootSlot, header.nodeSize);
    RMRC(rc, IX_RM);
    rmm.CloseFile(handle);
    return OK_RC;
}

// Destroy and Index
RC IX_Manager::DestroyIndex(const char *fileName, int indexNo) {
    string name = string(fileName) + "." + to_string(indexNo);
    RC rc = rmm.DestroyFile(name.c_str());
    RMRC(rc, IX_PF);
    return OK_RC;
}

// Open an Index
RC IX_Manager::OpenIndex(const char *fileName, int indexNo,
            IX_IndexHandle &indexHandle) {
    string name = string(fileName) + "." + to_string(indexNo);
    // printf("[OpenIndex] %s\n", name.c_str());
    RC rc = rmm.OpenFile(name.c_str(), indexHandle.fh);
    RMRC(rc, IX_PF);
    indexHandle.init();
    return OK_RC;
}

// Close an Index
RC IX_Manager::CloseIndex(IX_IndexHandle &indexHandle) {
    // printf("[CloseIndex]\n");
    RC rc = rmm.CloseFile(indexHandle.fh);
    RMRC(rc, IX_PF);
    return OK_RC;
}
