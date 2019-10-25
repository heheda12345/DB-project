#include "rm.h"
#include "rm_internal.h"

RM_Manager::RM_Manager() : pfm(PF_Manager::instance()) {
}

RC RM_Manager::CreateFile (const std::string& fileName, int recordSize) {
    int rc = pfm.CreateFile(fileName.c_str());
    PFRC(rc)
    PF_FileHandle fh;
    rc = pfm.OpenFile(fileName.c_str(), fh);
    PFRC(rc)
    PF_PageHandle ph;
    rc = fh.AllocatePage(ph);
    PFRC(rc)
    char* data;
    rc = ph.GetData(data);
    PFRC(rc)
    FileHeader* fileHeader = reinterpret_cast<FileHeader*> (data);
    fileHeader->pageCnt = 1;
    fileHeader->poolHead = 0;
    fileHeader->recordPerPage = std::min((int)(PF_PAGE_SIZE - sizeof(PageHeader)), MAX_RECORD_PER_PAGE);
    fileHeader->recordSize = recordSize;

    PageNum pageNum;
    rc = ph.GetPageNum(pageNum);
    PFRC(rc)
    rc = fh.MarkDirty(pageNum);
    PFRC(rc)
    rc = fh.UnpinPage(pageNum);
    PFRC(rc)
    rc = pfm.CloseFile(fh);
    PFRC(rc)
    return OK_RC;
}

RC RM_Manager::DestroyFile(const std::string& fileName) {
    return pfm.DestroyFile(fileName.c_str());
}

RC RM_Manager::OpenFile(const std::string& fileName, RM_FileHandle &fileHandle) {
    RC rc = pfm.OpenFile(fileName.c_str(), fileHandle.pfFileHandle);
    PFRC(rc)
    // need more action
    return OK_RC;
}

RC RM_Manager::CloseFile(RM_FileHandle &fileHandle) {
    RC rc;
    // need more action
    rc = pfm.CloseFile(fileHandle.pfFileHandle);
    PFRC(rc)
    return OK_RC;
}