#include "utils.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <assert.h>
using namespace std;

bool DirExist(const char* dir) {
    struct stat info;
    if(stat(dir, &info ) != 0)
        return 0;
    else if(info.st_mode & S_IFDIR)
        return 1;
    else
        return 0;
}

void InitDir(const char* dir) {
    if (DirExist(dir)) {
        RmDir(dir);
    }
    mkdir(dir, 0777); 
}

int TryMkDir(const char* dir) {
    if (DirExist(dir)) {
        return 1;
    }
    mkdir(dir, 0777);
    return 0;
}

void RmDir(const char* dir) {
    char cmd[1000];
    sprintf(cmd, "rm -r %s", dir);
    system(cmd);
}


void setBit(unsigned char& x, int pos, int val){
    assert(val == 0 || val == 1);
    x ^= x & (1 << pos);
    x |= val << pos;
}

int getBit(unsigned char x, int pos) {
    return (x >> pos) & 1;
}

void dumpString(char* pData, const string& st) {
    strcpy(pData, st.c_str());
}