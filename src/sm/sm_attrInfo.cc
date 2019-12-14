#include "sm.h"
using namespace std;

 AttrInfo::AttrInfo(const AttrType& _type, unsigned short _mxLen, const std::string& _attrName, bool notNull, bool hasDefault, const std::string& _dVal): type(_type), flag(0), mxLen(_mxLen), attrName(_attrName), dVal(_dVal) {
    setNotNullFlag(notNull);
    setDefaultFlag(hasDefault);
    mxLen = getMaxLen();
}

void AttrInfo::setNotNullFlag(bool b) { setBit(flag, 0, b); }
bool AttrInfo::isNotNull() const { return getBit(flag, 0); }
bool AttrInfo::hasDefault() const { return getBit(flag, 4); }
void AttrInfo::setDefaultFlag(bool b) {
    setBit(flag, 4, b);
}

int AttrInfo::load(const char* pData) {
    int cur = 0;
    type = (AttrType)*reinterpret_cast<const unsigned char*>(pData); cur += sizeof(char);
    flag = *reinterpret_cast<const unsigned char*>(pData + cur); cur += sizeof(char);
    mxLen = *reinterpret_cast<const unsigned short*>(pData + cur); cur += sizeof(short);
    // printf("[load] %d %d %d %d\n", (int)type, (int)flag, (int)mxLen, (int) linked);
    attrName = std::string(pData + cur); cur += MAXNAME;
    if (hasDefault()) {
        int len = *reinterpret_cast<const int*>(pData + cur); cur += sizeof(int);
        dVal = std::string(pData + cur, len); cur += mxLen;
    }
    return cur;
}

int AttrInfo::dump(char* pData) const {
    int cur = 0;
    *reinterpret_cast<unsigned char*>(pData) = (unsigned char) type; cur += sizeof(char);
    *reinterpret_cast<unsigned char*>(pData + cur) = flag; cur += sizeof(char);
    *reinterpret_cast<unsigned short*>(pData + cur) = mxLen; cur += sizeof(short);
    // printf("[dump] %d %d %d %d\n", (int)type, (int)flag, (int)mxLen, (int) linkedForeign.size());
    assert(attrName.length() < MAXNAME);
    dumpString(pData + cur, attrName); cur += MAXNAME;
    if (hasDefault()) {
        *reinterpret_cast<int*>(pData + cur) = int(dVal.length()); cur += sizeof(int);
        memset(pData + cur, 0, mxLen);
        memcpy(pData + cur, dVal.c_str(), dVal.length()); cur += mxLen;
    }
    return cur;
}

int AttrInfo::getSize() const {
    int cur = sizeof(char) + sizeof(char) + sizeof(short) + MAXNAME;
    if (hasDefault()) {
        cur += sizeof(int) + mxLen;
    }
    return cur;
}

vector<AttrInfo> AttrInfo::loadAttrs(const char* pData) {
    vector<AttrInfo> ret;
    int n = *reinterpret_cast<const int*>(pData);
    int cur = sizeof(int);
    for (int i = 0; i < n; i++) {
        AttrInfo attr;
        int size = attr.load(pData + cur);
        ret.push_back(attr);
        cur += size;
    }
    return ret;
}

int AttrInfo::dumpAttrs(char* pData, const std::vector<AttrInfo>& vec) {
    *reinterpret_cast<int*>(pData) = (int)vec.size();
    int cur = sizeof(int);
    for (const AttrInfo& x: vec) {
        int size = x.dump(pData + cur);
        cur += size;
    }
    return cur;
}

int AttrInfo::getAttrsSize(const std::vector<AttrInfo>& vec) {
    int ret = sizeof(int);
    for (auto& x: vec) {
        ret += x.getSize();
    }
    return ret;
}

int AttrInfo::getRecordSize(const std::vector<AttrInfo>& attrs) {
    int ret = 0;
    for (auto& attr: attrs) {
        ret += attr.mxLen;
    }
    return ret;
}

int AttrInfo::getPos(const std::vector<AttrInfo>& attrs, std::string attrName) {
    for (int i=0; i<(int)attrs.size(); i++) {
        if (attrs[i].attrName == attrName)
            return i;
    }
    return -1;
}

std::ostream& operator << (std::ostream& os, const std::vector<AttrInfo>& attrs) {
    for (auto& attr: attrs) {
        std::string st_ty = getName(attr.type);
        std::string info = "";
        if (attr.isNotNull()) {
            info += "Not Null";
        }
        if (attr.hasDefault()) {
            if (info != "")
                info += ", ";
            info += "Has Default";
        }
        if (info != "") {
            info.insert(0, "(");
            info.append(")");
        }
        os << attr.attrName << std::string(" ") << st_ty << info << "    ";
    }
    return os;
}

int AttrInfo::getMaxLen() const {
    return mxLen ? mxLen : getDefaultLen(type);
}

std::vector<int> AttrInfo::getAllMxLen(const std::vector<AttrInfo>& attrs) {
    std::vector<int> ret;
    for (auto& attr: attrs) {
        ret.push_back(attr.getMaxLen());
    }
    return ret;
}

std::vector<AttrType> AttrInfo::getAllType(const std::vector<AttrInfo>& attrs) {
    std::vector<AttrType> ret;
    for (auto& attr: attrs) {
        ret.push_back(attr.type);
    }
    return ret;
}

std::vector<int> AttrInfo::mapMxLen(const std::vector<AttrInfo>& attrs, const std::vector<std::string>& attrNames) {
    std::vector<int> ret;
    for (auto& name: attrNames) {
        int pos = getPos(attrs, name);
        assert(pos != -1);
        ret.push_back(attrs[pos].mxLen);
    }
    return ret;
}

std::vector<AttrType> AttrInfo::mapType(const std::vector<AttrInfo>& attrs, const std::vector<std::string>& attrNames) {
    std::vector<AttrType> ret;
    for (auto& name: attrNames) {
        int pos = getPos(attrs, name);
        assert(pos != -1);
        ret.push_back(attrs[pos].type);
    }
    return ret;
}