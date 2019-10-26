#pragma once
#include <cstring>

#include "../redbase.h"
#include "rm_rid.h"

const int MAX_RECORD_PER_PAGE = 256;

struct FileHeader {
    int recordSize, recordPerPage;
    PageNum poolHead;
    // recordSize的单位是字节
};

struct PageHeader {
    char used[MAX_RECORD_PER_PAGE >> 3]; // used must be the first obj
    PageNum poolNext;
};

class RM_Attr {
public:
    RM_Attr() : ty(NO_TYPE) {}
    ~RM_Attr() {
        if (ty == STRING)
            delete[] val.strValue;
    }
    void set(AttrType _attrType, int _attrLength, int _attrOffset, void* value);

    bool Satisfy(CompOp op, const char* data) const;

private:
    AttrType ty;
    int len, offset;
    union VAL
    {
        int intValue;
        float floatValue;
        char* strValue;
    } val;
    const RM_Attr& operator = (const RM_Attr&);
};