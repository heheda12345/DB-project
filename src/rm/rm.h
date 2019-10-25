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

#define RM_WARN_EMPTY_RECORD (START_RM_WARN + 1)
#define RM_EOF               (START_RM_WARN + 2)


//
// RM_Record: RM Record interface
//
class RM_Record {
public:
    RM_Record () : data(nullptr) {}
    ~RM_Record() {
        delete[] data;
    }

    // Return the data corresponding to the record.  Sets *pData to the
    // record contents.
    RC GetData(char *&pData) const {
        if (data == nullptr)
            return RM_WARN_EMPTY_RECORD;
        pData = data;
        return OK_RC;
    }

    // Return the RID associated with the record
    RC GetRid (RID &rid) const {
        if (data == nullptr)
            return RM_WARN_EMPTY_RECORD;
        rid = this->rid;
        return OK_RC;
    }

private:
    char* data;
    RID rid;
};

//
// RM_FileHandle: RM File interface
//
class RM_FileHandle {
    friend class RM_Manager;
public:
    RM_FileHandle () {};
    ~RM_FileHandle() = default;

    // Given a RID, return the record
    RC GetRec     (const RID &rid, RM_Record &rec) const { return OK_RC; };

    RC InsertRec  (const char *pData, RID &rid) { return OK_RC; };       // Insert a new record

    RC DeleteRec  (const RID &rid) { return OK_RC; };                    // Delete a record
    RC UpdateRec  (const RM_Record &rec) { return OK_RC; };              // Update a record

    // Forces a page (along with any contents stored in this class)
    // from the buffer pool to disk.  Default value forces all pages.
    RC ForcePages (PageNum pageNum = ALL_PAGES) { return OK_RC; };

private:
    PF_FileHandle pfFileHandle;
};

//
// RM_FileScan: condition-based scan of records in the file
//
class RM_FileScan {
public:
    RM_FileScan  () {};
    ~RM_FileScan () = default;

    RC OpenScan  (const RM_FileHandle &fileHandle,
                  AttrType   attrType,
                  int        attrLength,
                  int        attrOffset,
                  CompOp     compOp,
                  void       *value,
                  ClientHint pinHint = NO_HINT) {}; // Initialize a file scan
    RC GetNextRec(RM_Record &rec) {};               // Get next matching record
    RC CloseScan () {};                             // Close the scan
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

const int MAX_RECORD_PER_PAGE = 256;
//
// Print-error function
//
void RM_PrintError(RC rc);
#define RMRC(rc)  { if (rc) { RM_PrintError(rc); return rc;} }
#endif
