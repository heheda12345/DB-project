#include "rm.h"

struct FileHeader {
    int recordSize, recordPerPage, pageCnt, poolHead;
};

struct PageHeader {
    int poolNext;
    bool used[MAX_RECORD_PER_PAGE];
};