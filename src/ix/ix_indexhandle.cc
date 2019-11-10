#include "ix.h"

IX_IndexHandle::IX_IndexHandle() {

}

RC IX_IndexHandle::InsertEntry(void *pData, const RID &rid) {
    return OK_RC;
}

RC IX_IndexHandle::DeleteEntry(void *pData, const RID &rid) {
    return OK_RC;
}

RC IX_IndexHandle::ForcePages() {
    return OK_RC;
}