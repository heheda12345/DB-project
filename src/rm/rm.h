//
// rm.h
//
//   Record Manager component interface
//
// This file does not include the interface for the RID class.  This is
// found in rm_rid.h
//

#ifndef RM_H
#define RM_H

#include <string>
#include <cstring>

// Please DO NOT include any files other than redbase.h and pf.h in this
// file.  When you submit your code, the test program will be compiled
// with your rm.h and your redbase.h, along with the standard pf.h that
// was given to you.  Your rm.h, your redbase.h, and the standard pf.h
// should therefore be self-contained (i.e., should not depend upon
// declarations in any other file).

// Do not change the following includes
#include "../redbase.h"
#include "rm_rid.h"
#include "../pf/pf.h"
#include "rm_internal.h"

#define RM_RECORD_GETDATA (START_RM_WARN + 1)
#define RM_RECORD_GETRID  (START_RM_WARN + 2)
#define RM_FILEHANDLE_GETREC (START_RM_WARN + 11)
#define RM_FILEHANDLE_INSERTREC (START_RM_WARN + 12)
#define RM_FILEHANDLE_DELETEREC (START_RM_WARN + 13)
#define RM_FILEHANDLE_UPDATEREC (START_RM_WARN + 14)
#define RM_FILEHANDLE_FORCEPAGES (START_RM_WARN + 15)
#define RM_FILEHANDLE_OPEN (START_RM_WARN + 16)
#define RM_FILEHANDLE_CLOSE (START_RM_WARN + 17)
#define RM_FILEHANDLE_CHECKRID (START_RM_WARN + 18)
#define RM_FILEHANDLE_GETFIRSTREC (START_RM_WARN + 19)
#define RM_FILEHANDLE_GETNEXTREC (START_RM_WARN + 20)
#define RM_FILESCAN_OPENSCAN (START_RM_WARN + 21)
#define RM_FILESCAN_GETNEXTREC (START_RM_WARN + 22)
#define RM_FILESCAN_CLOSESCAN (START_RM_WARN + 23)
#define RM_MANAGER_CREATEFILE (START_RM_WARN + 31)
#define RM_MANAGER_DESTROYFILE (START_RM_WARN + 32)
#define RM_MANAGER_OPENFILE (START_RM_WARN + 33)
#define RM_MANAGER_CLOSEFILE (START_RM_WARN + 34)
#define RM_NEW_WARN_START  (START_RM_WARN + 50)
#define RM_NEW_ERROR_START (START_RM_ERR - 50)

#define RM_WARN_EMPTY_RECORD (RM_NEW_WARN_START + 1)
#define RM_EOF               (RM_NEW_WARN_START + 2)
#define RM_FILE_IS_OPEN      (RM_NEW_WARN_START + 3)
#define RM_FILE_NOT_OPEN     (RM_NEW_WARN_START + 4)
#define RM_SLOT_OUTOFRANGE   (RM_NEW_WARN_START + 5)
#define RM_NO_SUCH_REC       (RM_NEW_WARN_START + 6)
#define RM_NOT_EMPTY_REC     (RM_NEW_WARN_START + 7)
#define RM_SCAN_NOT_OPEN     (RM_NEW_WARN_START + 8)
#define RM_SCAN_NOT_CLOSE    (RM_NEW_WARN_START + 9)
//
// RM_Record: RM Record interface
//
class RM_Record {
public:
    RM_Record () : valid(false) {}
    
    RM_Record (const char* _data, int _len, RID _rid):
        len(_len), rid(_rid), valid(true) {
            data = new char[len];
            memcpy(data, _data, len);
        }

    ~RM_Record () {
        if (valid)
            delete[] data;
    }

    // Return the data corresponding to the record.  Sets *pData to the
    // record contents.
    RC GetData(char * &pData) const {
        if (!valid)
            return RM_WARN_EMPTY_RECORD;
        pData = data;
        return OK_RC;
    }

    // Return the RID associated with the record
    RC GetRid (RID &rid) const {
        if (!valid)
            return RM_WARN_EMPTY_RECORD;
        rid = this->rid;
        return OK_RC;
    }
    
    // assume data is not null
    void SetValue(const char* _data, int _len, RID _rid) {
        if (valid)
            delete[] data;
        valid = true;
        len = _len;
        data = new char[len];
        memcpy(data, _data, len);
    }

    void CopyTo(RM_Record& rec) {
        rec.SetValue(data, len, rid);
    }


private:
    char* data;
    int len;
    RID rid;
    bool valid;
    const RM_Record& operator=(const RM_Record&);
};

//
// RM_FileHandle: RM File interface
//
class RM_FileHandle {
    friend class RM_Manager;
public:
    RM_FileHandle(): isOpen(false) {}
    ~RM_FileHandle() = default;

    // Given a RID, return the record
    RC GetRec     (const RID &rid, RM_Record &rec) const;

    RC InsertRec  (const char *pData, RID &rid);       // Insert a new record

    RC DeleteRec  (const RID &rid);                    // Delete a record
    RC UpdateRec  (const RM_Record &rec);              // Update a record

    // Forces a page (along with any contents stored in this class)
    // from the buffer pool to disk.  Default value forces all pages.
    RC ForcePages (PageNum pageNum = ALL_PAGES);
    RC GetFirstRec(PageNum& pageNum, SlotNum& slotNum,  RM_Record &rec) const;
    RC GetNextRec(PageNum& pageNum, SlotNum& slotNum,  RM_Record &rec) const; // pageNum & slotNum should be valid

private:
    RC Open(PF_FileHandle& handle);
    RC Close();
    RC checkRid(const RID &rid) const;

    int getIndex(SlotNum slot) const;
    bool recordExist(char* data, SlotNum slot) const {
        // printf("record %x %d\n", data[slot>>3], slot);
        return (data[slot >> 3] >> (slot & 7)) & 1;
    }
    void revRecord(char* data, SlotNum slot) const {
        data[slot>>3] = data[slot>>3] ^ (1<<(slot&7));
    }
    
    bool isOpen;
    PF_FileHandle pfFileHandle;
    FileHeader fileHeader;
};

//
// RM_FileScan: condition-based scan of records in the file
//
class RM_FileScan {
public:
    RM_FileScan  () : state(CLOSE) {};
    ~RM_FileScan () = default;

    RC OpenScan  (const RM_FileHandle &fileHandle,
                  AttrType   attrType,
                  int        attrLength,
                  int        attrOffset,
                  CompOp     compOp,
                  void       *value,
                  ClientHint pinHint = NO_HINT); // Initialize a file scan
    RC GetNextRec(RM_Record &rec);               // Get next matching record
    RC CloseScan ();                             // Close the scan
private:
    const RM_FileHandle *fileHandle;
    CompOp     compOp;
    RM_Attr attr;
    enum State {
        CLOSE, UNSTART, RUNNING, DONE
    } state;
    PageNum pageNum;
    SlotNum slotNum;
};

//
// RM_Manager: provides RM file management
//
class RM_Manager {
public:
    ~RM_Manager   () = default;

    RC CreateFile (const std::string& fileName, int recordSize);
    RC DestroyFile(const std::string& fileName);
    RC OpenFile   (const std::string& fileName, RM_FileHandle &fileHandle);

    RC CloseFile  (RM_FileHandle &fileHandle);

    static RM_Manager& instance() {
        static RM_Manager ins;
        return ins;
    }
private:
    RM_Manager    ();
    PF_Manager &pfm;
};

//
// Print-error function
//
void RM_PrintError(RC rc);

#define RMRC(rc, ret_rc) { \
   if (rc != 0) { \
      RM_PrintError(rc); \
      return rc > 0 ? ret_rc : -ret_rc;  \
   } \
}
#endif
