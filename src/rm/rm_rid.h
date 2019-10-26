//
// rm_rid.h
//
//   The Record Id interface
//

#ifndef RM_RID_H
#define RM_RID_H

// We separate the interface of RID from the rest of RM because some
// components will require the use of RID but not the rest of RM.

#include "../redbase.h"

//
// PageNum: uniquely identifies a page in a file
//
typedef long long PageNum;

//
// SlotNum: uniquely identifies a record in a page
//
typedef int SlotNum;

#define RID_INVALID (START_RID_WARN + 1)

//
// RID: Record id interface
//
class RID {
public:
    RID(): valid(false) {};                                     // Default constructor
    RID(PageNum _pageNum, SlotNum _slotNum):
        pageNum(_pageNum), slotNum(_slotNum), valid(true) {};
    ~RID() {};                                        // Destructor

    RC GetPageNum(PageNum &_pageNum) const {
        if (!valid)
            return RID_INVALID;
        _pageNum = pageNum;
        return OK_RC;
    };         // Return page number
    RC GetSlotNum(SlotNum &_slotNum) const {
        if (!valid)
            return RID_INVALID;
        _slotNum = slotNum;
        return OK_RC;
    };         // Return slot number
    RC check() const {
        if (!valid)
            return RID_INVALID;
        return OK_RC;
    }

private:
    PageNum pageNum;
    SlotNum slotNum;
    bool valid;
};

#define RIDRC(rc, ret_rc) { \
   if (rc) { \
      if (rc == RID_INVALID) \
        printf("RID WARNING: Invalid rid\n"); \
      else \
        printf("errno in ridrc %d\n", rc); \
      return rc > 0 ? ret_rc : -ret_rc;  \
   } \
}

#endif
