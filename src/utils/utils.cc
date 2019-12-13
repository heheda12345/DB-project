#include "utils.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <assert.h>
#include <unistd.h>
#include <dirent.h>
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

std::string getName(AttrType ty) {
     switch (ty) {
        case INT: {
            return "INT";
        }
        case FLOAT: {
            return "FLOAT";
        }
        case STRING: {
            return "STRING";
        }
        case DATE: {
            return "DATE";
        }
        case NO_TYPE: {
            return "NO TYPE"; 
        }
    }
}

vector<string> getFiles(const char* path) {
    DIR *dir;
	struct dirent *ptr;
	char base[1000];
    vector<string> ret;

	if ((dir=opendir(path)) == NULL) {
		printf("[ERROR] dir %s not found!\n", path);
        return ret;
    }
 
	while ((ptr=readdir(dir)) != NULL)
	{
		if(strcmp(ptr->d_name,".")==0 || strcmp(ptr->d_name,"..")==0) // current dir OR parrent dir
		    continue;
		else if(ptr->d_type == 8) {    // file
			//printf("d_name:%s/%s\n",basePath,ptr->d_name);
			ret.push_back(string(ptr->d_name));
        } else {
			printf("[ERROR] %s/%s is not a file!\n", path, ptr->d_name);
			continue;
        }
	}
	closedir(dir);
}

vector<string> getAllTable(const char* path) {
    auto files = getFiles(path);
    vector<string> ret;
    for (auto s: files) {
        if (s.find('.') == std::string::npos)
            ret.push_back(s);
    }
    return ret;
}

int getDefaultLen(AttrType ty) {
    switch (ty) {
        case INT: {
            return sizeof(int);
        }
        case FLOAT: {
            return sizeof(int);
        }
        case DATE: {
            return sizeof(int);
        }
        case STRING: {
            return MAXSTRINGLEN;
        }
        case NO_TYPE: {
            return 0;
        }
    }
    return 0;
}