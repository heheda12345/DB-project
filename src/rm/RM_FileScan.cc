#include "rm.h"

RC RM_FileScan::OpenScan  (
    const RM_FileHandle &_fileHandle,
    AttrType   _attrType,
    int        _attrLength,
    int        _attrOffset,
    CompOp     _compOp,
    void       *_value,
    ClientHint _pinHint) {
    
    if (state != CLOSE)
        return RM_SCAN_NOT_CLOSE;
    fileHandle = &_fileHandle;
    attr.set(_attrType, _attrLength, _attrOffset, _value);
    compOp = _compOp;
    state = UNSTART;
    return OK_RC;
}

RC RM_FileScan::GetNextRec(RM_Record &rec) {
    RC rc;
    int rc_ret = RM_FILESCAN_GETNEXTREC;
    switch (state) {
        case CLOSE: {
            return RM_SCAN_NOT_OPEN;
        }
        case UNSTART: {
            rc = fileHandle->GetFirstRec(pageNum, slotNum, rec);
            if (rc == RM_NO_SUCH_REC) {
                state = DONE;
                return RM_EOF;
            }
            RMRC(rc, rc_ret)
            state = RUNNING;
            break;
        }
        case RUNNING: {
            rc = fileHandle->GetNextRec(pageNum, slotNum, rec);
            if (rc == RM_NO_SUCH_REC) {
                state = DONE;
                return RM_EOF;
            }
            RMRC(rc, rc_ret)
            break;
        }
        case DONE:
            return RM_EOF;
    }
    while (true) {
        char *data;
        rc = rec.GetData(data);
        RMRC(rc, rc_ret)
        if (attr.Satisfy(compOp, data)) {
            return OK_RC;
        }
        rc = fileHandle->GetNextRec(pageNum, slotNum, rec);
        if (rc == RM_NO_SUCH_REC) {
            state = DONE;
            return RM_EOF;
        }
        RMRC(rc, rc_ret)
    }
    printf("should not reach here! RM_FileScan");
    return rc_ret;
}

RC RM_FileScan::CloseScan() {
    if (state == CLOSE)
        return RM_SCAN_NOT_OPEN;
    state = CLOSE;
    return OK_RC;
}
