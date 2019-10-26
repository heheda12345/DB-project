#pragma once

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