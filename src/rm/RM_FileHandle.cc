#include "rm.h"
#include <cstring>

RC RM_FileHandle::Open(PF_FileHandle& handle) {
    int rc_ret = RM_FILEHANDLE_OPEN;
    if (isOpen) {
        RMRC(RM_FILE_IS_OPEN, rc_ret);
    }
    pfFileHandle = handle;
    PF_PageHandle pfPageHandle;
    int rc = pfFileHandle.GetFirstPage(pfPageHandle);
    PFRC(rc, rc_ret)
    char* data;
    rc = pfPageHandle.GetData(data);
    PFRC(rc, rc_ret)
    fileHeader = *reinterpret_cast<FileHeader*> (data);
    isOpen = true;

    PageNum pageNum;
    rc = pfPageHandle.GetPageNum(pageNum);
    PFRC(rc, rc_ret)
    rc = pfFileHandle.UnpinPage(pageNum);
    PFRC(rc, rc_ret)
    return OK_RC;
}

RC RM_FileHandle::Close() {
    int rc_ret = RM_FILEHANDLE_CLOSE;
    if (!isOpen) {
        RMRC(RM_FILE_NOT_OPEN, RM_FILEHANDLE_CLOSE);
    }
    RC rc = pfFileHandle.ForcePages();
    PFRC(rc, rc_ret)

    isOpen = false;
    return OK_RC;
}

RC RM_FileHandle::checkRid(const RID &rid) const {
    SlotNum slot;
    RC rc = rid.GetSlotNum(slot);
    RIDRC(rc, RM_FILEHANDLE_CHECKRID)
    if (slot < 0 || slot >= fileHeader.recordPerPage) {
        return RM_SLOT_OUTOFRANGE;
    }
    return OK_RC;
}

int RM_FileHandle::getIndex(SlotNum slot) const{
    return sizeof(PageHeader) + slot * fileHeader.recordSize;
}

RC RM_FileHandle::GetRec(const RID &rid, RM_Record &rec) const {
    int rc_ret = RM_FILEHANDLE_GETREC;
    if (!isOpen) {
        RMRC(RM_FILE_NOT_OPEN, rc_ret);
    }

    RC rc = rid.check();
    RIDRC(rc, rc_ret)
    
    PF_PageHandle pgHandle;
    PageNum pageNum;
    SlotNum slotNum;
    
    rc = checkRid(rid);
    RMRC(rc, rc_ret)
    rc = rid.GetPageNum(pageNum);
    RIDRC(rc, rc_ret)
    rc = rid.GetSlotNum(slotNum);
    RIDRC(rc, rc_ret);
    
    rc = pfFileHandle.GetThisPage(pageNum, pgHandle);
    PFRC(rc, rc_ret)

    char* data;
    rc = pgHandle.GetData(data);
    PFRC(rc, rc_ret)

    if (!recordExist(data, slotNum)) {
        pfFileHandle.UnpinPage(pageNum);
        RMRC(RM_NO_SUCH_REC, rc_ret)
    }

    rec.SetValue(data + getIndex(slotNum), fileHeader.recordSize, rid);
    rc = pfFileHandle.UnpinPage(pageNum);
    PFRC(rc, rc_ret)
    return OK_RC;
}

RC RM_FileHandle::InsertRec(const char *pData, RID &rid) {
    int rc_ret = RM_FILEHANDLE_INSERTREC;
    if (!isOpen) {
        RMRC(RM_FILE_NOT_OPEN, rc_ret);
    }

    RC rc;
    PF_PageHandle pgHandle;
    PageNum pageNum;
    SlotNum slotNum;
    char* data;
    bool findEmpty = false;
    
    rc = pfFileHandle.GetFirstPage(pgHandle);
    while (!findEmpty) {
        rc = pgHandle.GetPageNum(pageNum);
        PFRC(rc, rc_ret)

        rc = pfFileHandle.UnpinPage(pageNum);
        PFRC(rc, rc_ret)

        rc = pfFileHandle.GetNextPage(pageNum, pgHandle);
        if (rc == PF_EOF)
            break;
        PFRC(rc, rc_ret)

        rc = pgHandle.GetData(data);
        PFRC(rc, rc_ret)
        
        for (SlotNum i=0; i<fileHeader.recordPerPage; i++)
            if (!recordExist(data, i)) {
                slotNum = i;
                rc = pgHandle.GetPageNum(pageNum);
                PFRC(rc, rc_ret)
                findEmpty = true;
                break;
            }
    }

    if (!findEmpty) {
        RC rc = pfFileHandle.AllocatePage(pgHandle);
        PFRC(rc, rc_ret)
        rc = pgHandle.GetPageNum(pageNum);
        PFRC(rc, rc_ret)
        rc = pgHandle.GetData(data);
        PFRC(rc, rc_ret)
        slotNum = 0;
        memset(data, 0, sizeof(PF_PAGE_SIZE));
    }
    
    //run
    memcpy(data + getIndex(slotNum), pData, fileHeader.recordSize);
    revRecord(data, slotNum);
    pfFileHandle.MarkDirty(pageNum);
    rid = RID(pageNum, slotNum);

    rc = pfFileHandle.UnpinPage(pageNum);
    PFRC(rc, rc_ret)
    return OK_RC;
};

RC RM_FileHandle::DeleteRec(const RID &rid) {
    int rc_ret = RM_FILEHANDLE_DELETEREC;
    if (!isOpen) {
        RMRC(RM_FILE_NOT_OPEN, rc_ret);
    }

    PF_PageHandle pgHandle;
    PageNum pageNum;
    SlotNum slotNum;
    RC rc = rid.check();
    RIDRC(rc, rc_ret)
    rc = checkRid(rid);
    RMRC(rc, rc_ret)
    rc = rid.GetPageNum(pageNum);
    RIDRC(rc, rc_ret)
    rc = rid.GetSlotNum(slotNum);
    RIDRC(rc, rc_ret);

    rc = pfFileHandle.GetThisPage(pageNum, pgHandle);
    PFRC(rc, rc_ret)

    char* data;
    rc = pgHandle.GetData(data);
    PFRC(rc, rc_ret)

    if (!recordExist(data, slotNum)) {
        pfFileHandle.UnpinPage(pageNum);
        RMRC(RM_NO_SUCH_REC, rc_ret)
    }

    revRecord(data, slotNum);
    pfFileHandle.MarkDirty(pageNum);

    rc = pfFileHandle.UnpinPage(pageNum);
    PFRC(rc, rc_ret)
    return OK_RC;
}

RC RM_FileHandle::UpdateRec(const RM_Record &rec) {
    int rc_ret = RM_FILEHANDLE_UPDATEREC;
    if (!isOpen) {
        RMRC(RM_FILE_NOT_OPEN, rc_ret);
    }
    RID rid;
    PF_PageHandle pgHandle;
    PageNum pageNum;
    SlotNum slotNum;
    RC rc = rec.GetRid(rid);
    RMRC(rc, rc_ret)
    rc = rid.check();
    RIDRC(rc, rc_ret)
    rc = checkRid(rid);
    RMRC(rc, rc_ret)
    rc = rid.GetPageNum(pageNum);
    RIDRC(rc, rc_ret)
    rc = rid.GetSlotNum(slotNum);
    RIDRC(rc, rc_ret);

    rc = pfFileHandle.GetThisPage(pageNum, pgHandle);
    PFRC(rc, rc_ret)
    char* data;
    rc = pgHandle.GetData(data);
    PFRC(rc, rc_ret)
    if (!recordExist(data, slotNum)) {
        pfFileHandle.UnpinPage(pageNum);
        RMRC(RM_NO_SUCH_REC, rc_ret)
    }

    char* pData;
    rc = rec.GetData(pData);
    RMRC(rc, rc_ret)
    memcpy(data + getIndex(slotNum), pData, fileHeader.recordSize);
    pfFileHandle.MarkDirty(pageNum);

    rc = pfFileHandle.UnpinPage(pageNum);
    PFRC(rc, rc_ret);
    return OK_RC;
}

RC RM_FileHandle::ForcePages (PageNum pageNum) {
    int rc_ret = RM_FILEHANDLE_FORCEPAGES;
    if (!isOpen) {
        RMRC(RM_FILE_NOT_OPEN, rc_ret);
    }
    RC rc = pfFileHandle.ForcePages(pageNum);
    PFRC(rc, rc_ret)
    return OK_RC;
};

RC RM_FileHandle::GetFirstRec(PageNum& pageNum, SlotNum& slotNum, RM_Record &rec) const {
    PF_PageHandle pgHandle;
    RC rc = pfFileHandle.GetFirstPage(pgHandle);
    int rc_ret = RM_FILEHANDLE_GETFIRSTREC;
    rc = pgHandle.GetPageNum(pageNum);
    PFRC(rc, rc_ret)
    rc = pfFileHandle.UnpinPage(pageNum);
    PFRC(rc, rc_ret)
    slotNum = fileHeader.recordPerPage - 1;
    return GetNextRec(pageNum, slotNum, rec);
}

// RC RM_FileHandle::GetNextRec(RM_Record &rec) const {
//     int rc_ret = RM_FILEHANDLE_GETNEXTREC;
//     RID rid;
//     PageNum pageNum;
//     SlotNum slotNum;
//     RC rc = rec.GetRid(rid);
//     RMRC(rc, rc_ret)
//     rc = rid.GetPageNum(pageNum);
//     RMRC(rc, rc_ret)
//     rc = rid.GetSlotNum(slotNum);
//     RMRC(rc, rc_ret)

//     return GetNextRec(pageNum, slotNum, rec);
// }

RC RM_FileHandle::GetNextRec(PageNum& pageNum, SlotNum& slotNum, RM_Record &rec) const {
    int rc_ret = RM_FILEHANDLE_GETNEXTREC;
    RC rc;
    char* data;
    PF_PageHandle pgHandle;
    if (slotNum + 1 >= fileHeader.recordPerPage) {
        rc = pfFileHandle.GetNextPage(pageNum, pgHandle);
        slotNum = 0;
    } else {
        rc = pfFileHandle.GetThisPage(pageNum, pgHandle);
        slotNum++;
    }
    while (true) {
        rc = pgHandle.GetData(data);
        PFRC(rc, rc_ret)
        
        for (SlotNum i=slotNum; i<fileHeader.recordPerPage; i++)
            if (recordExist(data, i)) {
                slotNum = i;
                rc = pgHandle.GetPageNum(pageNum);
                PFRC(rc, rc_ret)
                rec.SetValue(data + getIndex(slotNum), fileHeader.recordSize, RID(pageNum, slotNum));
                rc = pfFileHandle.UnpinPage(pageNum);
                PFRC(rc, rc_ret)
                return OK_RC;
            }
        rc = pfFileHandle.UnpinPage(pageNum);
        PFRC(rc, rc_ret)

        rc = pfFileHandle.GetNextPage(pageNum, pgHandle);
        if (rc == PF_EOF)
            return RM_NO_SUCH_REC;
        PFRC(rc, rc_ret)
        slotNum = 0;
    }
    return rc_ret;
}

RC RM_FileHandle::GetMeta(char* pData, int &size) {
    const int rc_ret = RM_FILEHANDLE_GETMETA;
    if (!isOpen) {
        RMRC(RM_FILE_NOT_OPEN, rc_ret);
    }
    PF_PageHandle pfPageHandle;
    int rc = pfFileHandle.GetFirstPage(pfPageHandle);
    PFRC(rc, rc_ret)
    char* data;
    rc = pfPageHandle.GetData(data);
    PFRC(rc, rc_ret)
    fileHeader = *reinterpret_cast<FileHeader*> (data);
    size = fileHeader.metaSize;
    
    memcpy(pData, data + sizeof(fileHeader), fileHeader.metaSize);

    PageNum pageNum;
    rc = pfPageHandle.GetPageNum(pageNum);
    PFRC(rc, rc_ret)
    rc = pfFileHandle.UnpinPage(pageNum);
    PFRC(rc, rc_ret)
    return OK_RC;
}

RC RM_FileHandle::SetMeta(const char* pData, int size) {
    int rc_ret = RM_FILEHANDLE_SETMETA;
    if (!isOpen) {
        RMRC(RM_FILE_NOT_OPEN, rc_ret);
    }
    PF_PageHandle pfPageHandle;
    int rc = pfFileHandle.GetFirstPage(pfPageHandle);
    PFRC(rc, rc_ret)
    char* data;
    rc = pfPageHandle.GetData(data);
    PFRC(rc, rc_ret)
    (reinterpret_cast<FileHeader*>(data))->metaSize = size;
    fileHeader = *reinterpret_cast<FileHeader*> (data);
    memcpy(data + sizeof(FileHeader), pData, size);
    
    PageNum pageNum;
    rc = pfPageHandle.GetPageNum(pageNum);
    pfFileHandle.MarkDirty(pageNum);

    rc = pfFileHandle.UnpinPage(pageNum);
    PFRC(rc, rc_ret);
    return OK_RC;
}