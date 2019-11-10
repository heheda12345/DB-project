#include "ix.h"
#include <cstring>
#include <string>
using namespace std;

// Create a new Index
RC IX_Manager::CreateIndex(const char *fileName, int indexNo,
                AttrType attrType, int attrLength,
                int pageSize) {
    string name = string(fileName) + "." + to_string(indexNo);
    RC rc = pfm.CreateFile(name.c_str());
    IXRC(rc, IX_PF);
    return OK_RC;
}

// Destroy and Index
RC IX_Manager::DestroyIndex(const char *fileName, int indexNo) {
    string name = string(fileName) + "." + to_string(indexNo);
    RC rc = pfm.DestroyFile(name.c_str());
    IXRC(rc, IX_PF);
    return OK_RC;
}

// Open an Index
RC IX_Manager::OpenIndex(const char *fileName, int indexNo,
            IX_IndexHandle &indexHandle) {
    string name = string(fileName) + "." + to_string(indexNo);
    RC rc = pfm.OpenFile(name.c_str(), indexHandle.fh);
    PFRC(rc, IX_PF);
    return OK_RC;
}

// Close an Index
RC IX_Manager::CloseIndex(IX_IndexHandle &indexHandle) {
    RC rc = pfm.CloseFile(indexHandle.fh);
    PFRC(rc, IX_PF);
    return OK_RC;
}
