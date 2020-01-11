#include "utils.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <assert.h>
#include <unistd.h>
#include <dirent.h>
#include <algorithm>
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

std::string getName(AttrType ty, bool canChange) {
     switch (ty) {
        case INT: {
            return "INT";
        }
        case FLOAT: {
            return "FLOAT";
        }
        case STRING: {
            return canChange ? "VARCHAR" : "CHAR";
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

bool typeIsMatch(std::vector<AttrType> ty1, std::vector<AttrType> ty2) {
    if (ty1.size() != ty2.size())
        return 0;
    for (int i=0; i<ty1.size(); i++)
        if (ty1[i] != ty2[i])
            return 0;
    return 1;
}

bool lengthIsMatch(std::vector<std::string> attr1, std::vector<std::string> attr2) {
    if (attr1.size() != attr2.size())
        return 0;
    for (int i=0; i<attr1.size(); i++)
        if (attr1[i].length() != attr2[i].length())
            return 0;
    return 1;
}

int getSum(const std::vector<int> &vec) {
    int ret = 0;
    for (auto &x: vec)
        ret += x;
    return ret;
}

int findName(const std::vector<std::string>& vec, const std::string& target) {
    for (int i=0; i<vec.size(); i++)
        if (vec[i] == target)
            return i;
    return -1;
}

// Note: findName returns the first match, so always return self if only one exists
bool isDumplicated(const std::vector<std::string>& vec) {
    for (int i = 0; i < vec.size(); i++) {
        if (findName(vec, vec[i]) != i)
            return 1;
    }
    return 0;
}

bool IsLeapYear(int year)
{
    if(((year%4 == 0) && (year%100 != 0)) || (year%400 == 0))
        return true;
    return false;
}

bool isLegalDate(int date) {
    int year = date / 10000, month = date % 10000 / 100, day = date % 100;
    if(year < 0 || month <= 0 || month > 12 || day <= 0 || day > 31)
        return false;

    if (month == 1 || month == 3 || month == 5 || month == 7 || month == 8 || month == 10 || month == 12)
        return (day <= 31);
    if (month == 4 || month == 6 || month == 9 || month == 11)
        return (day <= 30);
    if (month == 2)
        return IsLeapYear(year) ? day <= 29 : day <= 28;
    
    assert(false);
    return false;
}

std::string cutForPrint(std::string st) {
    if (st.length() > PRINT_WIDTH - 5)
        st = std::string(st, 0, PRINT_WIDTH - 5).append("...");
    return st;
}

int cmpStr(const std::string& sl, const std::string& sr, AttrType type) {
    if (type == DATE)
        type = INT;
    switch (type) {
        case INT: {
            int l = *reinterpret_cast<const int*>(sl.c_str()),
                r = *reinterpret_cast<const int*>(sr.c_str());
            if (l == r)
                return 0;
            return l < r ? -1 : 1;
            break;
        }
        case FLOAT: {
            float fl = *reinterpret_cast<const float*>(sl.c_str()),
                fr = *reinterpret_cast<const float*>(sr.c_str());
            if (fl == fr)
                return 0;
            if (fl != fr)
                return fl < fr ? -1 : 1;
            break;
        }
        case STRING: {
            if (sl < sr)
                return -1;
            if (sl == sr)
                return 0;
            return 1;
        }
    }
    assert(false);
    return 0;
}

bool satisfyOp(const std::string& sl, const std::string& sr, AttrType type, CompOp op) {
    if (op == NO_OP)
        return 0;
    int cp = cmpStr(sl, sr, type);
    switch (op) {
        case EQ_OP: {
            return cp == 0;
        }
        case NE_OP: {
            return cp != 0;
        }
        case LT_OP: {
            return cp < 0;
        }
        case GT_OP: {
            return cp > 0;
        }
        case LE_OP: {
            return cp <= 0;
        }
        case GE_OP: {
            return cp >= 0;
        }
    }
    assert(false);
    return 0;
}

std::string ComputeAdd(const std::string& l, const std::string& r, AttrType type) {
    if (type == INT) {
        int li = *reinterpret_cast<const int*>(l.c_str()), ri = *reinterpret_cast<const int*>(r.c_str());
        int ans = li + ri;
        return std::string(reinterpret_cast<char*>(&ans), sizeof(int));
    } else if (type == FLOAT) {
        float lf = *reinterpret_cast<const float*>(l.c_str()), rf = *reinterpret_cast<const float*>(r.c_str());
        float ans = lf + rf;
        return std::string(reinterpret_cast<char*>(&ans), sizeof(float));
    } else {
        assert(false);
        return std::string();
    }
}

std::string ComputeSub(const std::string& l, const std::string& r, AttrType type) {
    if (type == INT) {
        int li = *reinterpret_cast<const int*>(l.c_str()), ri = *reinterpret_cast<const int*>(r.c_str());
        int ans = li - ri;
        return std::string(reinterpret_cast<char*>(&ans), sizeof(int));
    } else if (type == FLOAT) {
        float lf = *reinterpret_cast<const float*>(l.c_str()), rf = *reinterpret_cast<const float*>(r.c_str());
        float ans = lf - rf;
        return std::string(reinterpret_cast<char*>(&ans), sizeof(float));
    } else {
        assert(false);
        return std::string();
    }
}

std::string ComputeMul(const std::string& l, const std::string& r, AttrType type) {
    if (type == INT) {
        int li = *reinterpret_cast<const int*>(l.c_str()), ri = *reinterpret_cast<const int*>(r.c_str());
        int ans = li * ri;
        return std::string(reinterpret_cast<char*>(&ans), sizeof(int));
    } else if (type == FLOAT) {
        float lf = *reinterpret_cast<const float*>(l.c_str()), rf = *reinterpret_cast<const float*>(r.c_str());
        float ans = lf * rf;
        return std::string(reinterpret_cast<char*>(&ans), sizeof(float));
    } else {
        assert(false);
        return std::string();
    }
}

std::string ComputeDiv(const std::string& l, const std::string& r, AttrType type) {
     if (type == FLOAT) {
        float lf = *reinterpret_cast<const float*>(l.c_str()), rf = *reinterpret_cast<const float*>(r.c_str());
        float ans = lf / rf;
        return std::string(reinterpret_cast<char*>(&ans), sizeof(float));
    } else {
        assert(false);
        return std::string();
    }
}

bool cmp(const std::vector<std::string>& a, const std::vector<std::string>& b) {
    assert(a.size() == b.size());
    for (int i = 0; i < a.size(); i++) {
        if (a[i] != b[i])
            return a[i] < b[i];
    }
    return 0;
}

bool isDumplicated(const vector<vector<std::string>>& vec) {
    auto v = vec;
    sort(v.begin(), v.end(), cmp);
    for (int i = 1; i < v.size(); i++)
        if (cmp(v[i-1], v[i]) == 0)
            return 1;
    return 0;
}

bool AInB(const std::vector<std::vector<std::string>>& a, const std::vector<std::vector<std::string>>& bb) {
    auto b = bb;
    sort(b.begin(), b.end(), cmp);
    for (auto &value: a) {
        if (!binary_search(b.begin(), b.end(), value, cmp)) {
            return 0;
        }
    }
    return 1;
}