#include "ix.h"

IX_IndexScan::IX_IndexScan(){

};

RC IX_IndexScan::OpenScan(const IX_IndexHandle &indexHandle,
                          CompOp      compOp,
                          void        *value,
                          ClientHint  pinHint) {
    return OK_RC;
}          

RC IX_IndexScan::GetNextEntry(RID &rid) {
    return OK_RC;
}

RC IX_IndexScan::CloseScan() {
    return OK_RC;
} 
