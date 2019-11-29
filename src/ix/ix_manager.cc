#include "ix.h"
#include <cstring>
#include <string>
#include "ix_internal.h"
using namespace std;

// Create a new Index
RC IX_Manager::CreateIndex(const char *fileName, int indexNo, AttrType attrType, int attrLength) {
    string name = string(fileName) + "." + to_string(indexNo);
    IX_IndexHandle::Header header;
    header.attrLength = attrLength;
    header.attrType = attrType;
    header.btm = 3;
    header.nodeSize = IX_BTNode::getSize(attrLength, header.btm);
    RC rc = rmm.CreateFile(name.c_str(), header.nodeSize,
        reinterpret_cast<char*>(&header), sizeof(IX_IndexHandle::Header));
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
    header.btRoot = rid;
    rc = handle.SetMeta(reinterpret_cast<char*>(&header), sizeof(IX_IndexHandle::Header));
    RMRC(rc, IX_RM);
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
    RC rc = rmm.OpenFile(name.c_str(), indexHandle.fh);
    RMRC(rc, IX_PF);
    indexHandle.loadHeader();
    return OK_RC;
}

// Close an Index
RC IX_Manager::CloseIndex(IX_IndexHandle &indexHandle) {
    RC rc = rmm.CloseFile(indexHandle.fh);
    PFRC(rc, IX_PF);
    return OK_RC;
}