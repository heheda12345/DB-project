//
// rm_rid.h
//
//   The Record Id interface
//

#ifndef RM_RID_H
#define RM_RID_H

// We separate the interface of RID from the rest of RM because some
// components will require the use of RID but not the rest of RM.

#include "assert.h"
#include "../redbase.h"
#include <cstdio>

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
    RID(): pageNum(-1), slotNum(-1), valid(false) {};                                     // Default constructor
    RID(PageNum _pageNum, SlotNum _slotNum):
        pageNum(_pageNum), slotNum(_slotNum), valid(true) {};
    RID(char* pData) { this->loadFrom(pData); }
    static int getSize() { return sizeof(PageNum) + sizeof(SlotNum) + sizeof(bool); }

    void loadFrom(const char* pData) {
        pageNum = *reinterpret_cast<const PageNum*>(pData);
        slotNum = *reinterpret_cast<const SlotNum*>(pData + sizeof(PageNum));
        valid = *reinterpret_cast<const bool*>(pData + sizeof(PageNum) + sizeof(SlotNum));
    }

    void dumpTo(char* pData) {
        *reinterpret_cast<PageNum*>(pData) = pageNum;
        *reinterpret_cast<SlotNum*>(pData + sizeof(PageNum)) = slotNum;
        *reinterpret_cast<bool*>(pData + sizeof(PageNum) + sizeof(SlotNum)) = valid;
    }

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

    PageNum GetPageNum() const {
        assert(valid);
        return pageNum;
    }

    SlotNum GetSlotNum() const {
        assert(valid);
        return slotNum;
    }

    RC check() const {
        if (!valid)
            return RID_INVALID;
        return OK_RC;
    }

    bool isValid() const {
        return valid;
    }

    bool operator < (const RID& that) const{
        assert(valid);
        assert(that.valid);
        return pageNum < that.pageNum || (pageNum == that.pageNum && slotNum < that.slotNum);
    }
    bool operator == (const RID& that) const{
        assert(valid);
        assert(that.valid);
        return pageNum == that.pageNum && slotNum == that.slotNum;
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
