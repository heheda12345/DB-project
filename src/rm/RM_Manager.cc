#include "rm.h"
#include "rm_internal.h"

RM_Manager::RM_Manager() : pfm(PF_Manager::instance()) {
}

RC RM_Manager::CreateFile (const std::string& fileName, int recordSize, char* metaData, int metaSize) {
    const int rc_ret = RM_MANAGER_CREATEFILE;
    int rc = pfm.CreateFile(fileName.c_str());
    PFRC(rc, rc_ret);
    PF_FileHandle fh;
    rc = pfm.OpenFile(fileName.c_str(), fh);
    PFRC(rc, rc_ret);
    PF_PageHandle ph;
    rc = fh.AllocatePage(ph);
    PFRC(rc, RM_MANAGER_CREATEFILE)
    char* data;
    rc = ph.GetData(data);
    PFRC(rc, RM_MANAGER_CREATEFILE)
    FileHeader* fileHeader = reinterpret_cast<FileHeader*> (data);
    fileHeader->poolHead = 0;
    fileHeader->recordPerPage = std::min((int)(PF_PAGE_SIZE - sizeof(PageHeader)), MAX_RECORD_PER_PAGE);
    fileHeader->recordSize = recordSize;
    fileHeader->metaSize = metaSize;

    if (metaSize != 0) {
        memcpy(data + sizeof(FileHeader), metaData, metaSize);
    }

    PageNum pageNum;
    rc = ph.GetPageNum(pageNum);
    PFRC(rc, RM_MANAGER_CREATEFILE)
    rc = fh.MarkDirty(pageNum);
    PFRC(rc, RM_MANAGER_CREATEFILE)
    rc = fh.UnpinPage(pageNum);
    PFRC(rc, RM_MANAGER_CREATEFILE)
    rc = pfm.CloseFile(fh);
    PFRC(rc, RM_MANAGER_CREATEFILE)
    return OK_RC;
}

RC RM_Manager::DestroyFile(const std::string& fileName) {
    return pfm.DestroyFile(fileName.c_str());
}

RC RM_Manager::OpenFile(const std::string& fileName, RM_FileHandle &fileHandle) {
    PF_FileHandle pfFileHandle;
    RC rc = pfm.OpenFile(fileName.c_str(), pfFileHandle);
    PFRC(rc, RM_MANAGER_OPENFILE)
    rc = fileHandle.Open(pfFileHandle);
    RMRC(rc, RM_MANAGER_OPENFILE)
    return OK_RC;
}

RC RM_Manager::CloseFile(RM_FileHandle &fileHandle) {
    RC rc = fileHandle.Close();
    RMRC(rc, RM_MANAGER_CLOSEFILE)

    rc = pfm.CloseFile(fileHandle.pfFileHandle);
    PFRC(rc, RM_MANAGER_CLOSEFILE)
    return OK_RC;
}