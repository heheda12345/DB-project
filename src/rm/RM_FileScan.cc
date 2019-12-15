#include "rm.h"

RC RM_FileScan::OpenScan  (const RM_FileHandle &_fileHandle) {
    
    if (state != CLOSE)
        return RM_SCAN_NOT_CLOSE;
    fileHandle = &_fileHandle;
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
    return OK_RC;
}

RC RM_FileScan::CloseScan() {
    if (state == CLOSE)
        return RM_SCAN_NOT_OPEN;
    state = CLOSE;
    return OK_RC;
}
