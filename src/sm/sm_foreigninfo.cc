#include "sm.h"
using namespace std;

// std::string fkName;
//     std::string refTable;
//     std::vector<std::string> srcAttrs;
int ForeignKeyInfo::load(const char* pData) {
    int cur = 0;
    int colSize = *reinterpret_cast<const int*>(pData + cur); cur += sizeof(int);
    fkName = std::string(pData + cur); cur += MAXNAME;
    refTable = std::string(pData + cur); cur += MAXNAME;
    attrs.clear();
    for (int i=0; i < colSize; i++) {
        attrs.push_back(std::string(pData + cur)); cur += MAXNAME;
    }
    return cur;
}

int ForeignKeyInfo::dump(char* pData) const {
    int cur = 0;
    *reinterpret_cast<int*>(pData + cur) = (int)attrs.size(); cur += sizeof(int);
    dumpString(pData + cur, fkName); cur += MAXNAME;
    dumpString(pData + cur, refTable); cur += MAXNAME;
    for (auto& x: attrs) {
        dumpString(pData + cur, x); cur += MAXNAME;
    }
    return cur;
}

int ForeignKeyInfo::getSize() const {
    return sizeof(int) + MAXNAME * (attrs.size() + 2);
}

std::vector<ForeignKeyInfo> ForeignKeyInfo::loadForeigns(const char* pData) {
    vector<ForeignKeyInfo> ret;
    int n = *reinterpret_cast<const int*>(pData);
    int cur = sizeof(int);
    for (int i = 0; i < n; i++) {
        ForeignKeyInfo x;
        int size = x.load(pData + cur);
        ret.push_back(x);
        cur += size;
    }
    return ret;
}

int ForeignKeyInfo::dumpForeigns(char* pData, const std::vector<ForeignKeyInfo>& vec) {
    *reinterpret_cast<int*>(pData) = (int)vec.size();
    int cur = sizeof(int);
    for (const ForeignKeyInfo& x: vec) {
        int size = x.dump(pData + cur);
        cur += size;
    }
    return cur;
}

int ForeignKeyInfo::getForeignsSize(const std::vector<ForeignKeyInfo>& vec) {
    int ret = sizeof(int);
    for (auto& x: vec) {
        ret += x.getSize();
    }
    return ret;
}

int ForeignKeyInfo::getPos(const std::vector<ForeignKeyInfo> &fKeys, const std::string& fkName) {
    assert(fkName != "@@");
    for (int i=0; i<fKeys.size(); i++)
        if (fKeys[i].fkName == fkName)
            return i;
    return -1;
}

std::ostream& operator << (std::ostream& os, const ForeignKeyInfo& fKey) {
    os << fKey.fkName << "(";
    for (int i = 0; i < fKey.attrs.size(); i++) {
        if (i) {
            os << ", " << fKey.attrs[i];
        } else {
            os << fKey.attrs[i];
        }
    }   
    os << ") [" << fKey.refTable << "]";
}
