#include "sm.h"
using namespace std;

// struct IndexInfo {
//     std::string idxName;
//     int idxID;
//     std::vector<std::string> attrs;

//     int load(const char* pData) { return 0; }
//     int dump(char* pData) const { return 0; }
//     int getSize() const { return 0; }

//     static std::vector<IndexInfo> loadIndexes(const char* pData) { return std::vector<IndexInfo>(); }
//     static int dumpIndexes(char* pData, const std::vector<IndexInfo>& vec) { return 0; }
//     static int getIndexesSize(const std::vector<IndexInfo>& vec) { return 0; }
//     static int getPos(const std::vector<IndexInfo> &idxKeys, const std::string& idxName);
//     static int getNextId(const std::vector<IndexInfo> &idxKeys);
// };

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