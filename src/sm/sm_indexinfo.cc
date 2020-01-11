#include "sm.h"
using namespace std;

int IndexInfo::load(const char* pData) {
    int cur = 0;
    attrs.clear();
    int size = *reinterpret_cast<const int*>(pData + cur); cur += sizeof(int);
    idxName = std::string(pData + cur); cur += MAXNAME;
    idxID = *reinterpret_cast<const int*>(pData + cur); cur += sizeof(int);
    for (int i = 0; i < size; i++) {
        attrs.push_back(std::string(pData + cur)); cur += MAXNAME;
    }
    return cur;
}

int IndexInfo::dump(char* pData) const {
    int cur = 0;
    *reinterpret_cast<int*>(pData + cur) = (int)attrs.size(); cur += sizeof(int);
    dumpString(pData + cur, idxName); cur += MAXNAME;
    *reinterpret_cast<int*>(pData + cur) = idxID; cur += sizeof(int);
    for (auto &attr: attrs) {
        dumpString(pData + cur, attr); cur += MAXNAME;
    }
    return cur;
}

int IndexInfo::getSize() const {
    return sizeof(int) * 2 + MAXNAME * (attrs.size() + 1);
}

vector<IndexInfo> IndexInfo::loadIndexes(const char* pData) {
    vector<IndexInfo> ret;
    int n = *reinterpret_cast<const int*>(pData);
    int cur = sizeof(int);
    for (int i = 0; i < n; i++) {
        IndexInfo x;
        int size = x.load(pData + cur);
        ret.push_back(x);
        cur += size;
    }
    return ret;
}

int IndexInfo::dumpIndexes(char* pData, const std::vector<IndexInfo>& vec) {
    *reinterpret_cast<int*>(pData) = (int)vec.size();
    int cur = sizeof(int);
    for (const IndexInfo& x: vec) {
        int size = x.dump(pData + cur);
        cur += size;
    }
    return cur;
}

int IndexInfo::getIndexesSize(const std::vector<IndexInfo>& vec) {
    int ret = sizeof(int);
    for (auto& x: vec) {
        ret += x.getSize();
    }
    return ret;
}

int IndexInfo::getPos(const std::vector<IndexInfo> &idxKeys, const std::string& idxName) {
    for (int i = 0; i < idxKeys.size(); i++)
        if (idxKeys[i].idxName == idxName)
            return i;
    return -1;
}

int IndexInfo::getNextId(const std::vector<IndexInfo> &idxKeys) {
    int mx = 0;
    for (auto& key: idxKeys)
        mx = max(mx, key.idxID);
    return mx + 1;
}

bool IndexInfo::inUnique(const std::vector<IndexInfo> &uniqueGroups, const std::string& attrName) {
    for (auto& g: uniqueGroups) {
        if (findName(g.attrs, attrName) != -1)
            return 1;
    }
    return 0;
}

std::ostream& operator << (std::ostream& os, const IndexInfo& fKey) {
    os << fKey.idxName;
    if (fKey.idxID != -1)
        os << "(" << fKey.idxID << ")";
    os << ":";
    for (auto &x :fKey.attrs) {
        os << " " << x;
    }
    return os;
}