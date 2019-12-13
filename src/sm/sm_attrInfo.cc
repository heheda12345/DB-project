#include "sm.h"
using namespace std;

 AttrInfo::AttrInfo(const AttrType& _type, unsigned short _mxLen, const std::string& _attrName, bool notNull, bool isPrimary, bool hasDefault, const std::string& _dVal): type(_type), flag(0), mxLen(_mxLen), attrName(_attrName), refTable(""), refAttr(""), dVal(_dVal) {
    assert(attrName.length() < MAXNAME);
    setNotNullFlag(notNull);
    setPrimaryFlag(isPrimary);
    setForeignFlag(0);
    setDefaultFlag(hasDefault);
    mxLen = getMaxLen();
}

AttrInfo::AttrInfo(const AttrType& _type, unsigned short _mxLen, const std::string& _attrName, bool notNull, bool hasDefault, const std::string& _dVal, const std::string &_refTable, const std::string& _refAttr): type(_type), flag(0), mxLen(_mxLen), attrName(_attrName), refTable(_refTable), refAttr(_refAttr), dVal(_dVal) {
    assert(attrName.length() < MAXNAME);
    assert(refTable.length() < MAXNAME);
    assert(refAttr.length() < MAXNAME);
    setNotNullFlag(notNull);
    setPrimaryFlag(0);
    setForeignFlag(1);
    setDefaultFlag(hasDefault);
    mxLen = getMaxLen();
}

void AttrInfo::setNotNullFlag(bool b) { setBit(flag, 0, b); }
bool AttrInfo::isNotNull() const { return getBit(flag, 0); }

void AttrInfo::setPrimaryFlag(bool b) {
    setBit(flag, 1, b);
}
bool AttrInfo::isPrimary() const { return getBit(flag, 1); }

void AttrInfo::setForeignFlag(bool b) {
    setBit(flag, 2, b);
}

bool AttrInfo::isForeign() const { return getBit(flag, 2); }

RC AttrInfo::setForeign(const std::string& _refTable, const std::string& _refAttr) {
    if (!SM_Manager::instance().ExistAttr(_refTable, _refAttr)) {
        return SM_NO_SUCH_ATTR;
    }
    this->refTable = _refTable;
    this->refAttr = _refAttr;
    setForeignFlag(1);
    return OK_RC;
}

bool AttrInfo::hasDefault() const { return getBit(flag, 4); }
void AttrInfo::setDefaultFlag(bool b) {
    setBit(flag, 4, b);
}

int AttrInfo::load(const char* pData) {
    int cur = 0;
    type = (AttrType)*reinterpret_cast<const unsigned char*>(pData); cur += sizeof(char);
    flag = *reinterpret_cast<const unsigned char*>(pData + cur); cur += sizeof(char);
    mxLen = *reinterpret_cast<const unsigned short*>(pData + cur); cur += sizeof(short);
    int linked = *reinterpret_cast<const int*>(pData + cur); cur += sizeof(int);
    attrName = std::string(pData + cur); cur += MAXNAME;
    if (isForeign()) {
        refTable = std::string(pData + cur); cur += MAXNAME;
        refAttr = std::string(pData + cur); cur += MAXNAME;
    }
    if (hasDefault()) {
        int len = *reinterpret_cast<const int*>(pData + cur); cur += sizeof(int);
        dVal = std::string(pData + cur, len); cur += mxLen;
    }
    linkedForeign.clear();
    for (int i=0; i<linked; i++) {
        std::string tb(pData + cur); cur += MAXNAME;
        std::string attr(pData + cur); cur += MAXNAME;
        linkedForeign.push_back(make_pair(tb, attr));
    }
    return cur;
}

int AttrInfo::dump(char* pData) const {
    int cur = 0;
    *reinterpret_cast<unsigned char*>(pData) = (unsigned char) type; cur += sizeof(char);
    *reinterpret_cast<unsigned char*>(pData + cur) = flag; cur += sizeof(char);
    *reinterpret_cast<unsigned short*>(pData + cur) = mxLen; cur += sizeof(char);
    *reinterpret_cast<int*>(pData + cur) = (int)linkedForeign.size(); cur += sizeof(int);
    dumpString(pData + cur, attrName); cur += MAXNAME;
    if (isForeign()) {
        dumpString(pData + cur, refTable); cur += MAXNAME;
        dumpString(pData + cur, refAttr); cur += MAXNAME;
    }
    if (hasDefault()) {
        *reinterpret_cast<int*>(pData + cur) = (int)dVal.length(); cur += sizeof(int);
        memset(pData + cur, 0, mxLen);
        memcpy(pData + cur, dVal.c_str(), dVal.length()); cur += mxLen;
    }
    for (int i=0; i<(int)linkedForeign.size(); i++) {
        dumpString(pData+cur, linkedForeign[i].first.c_str()); cur += MAXNAME;
        dumpString(pData+cur, linkedForeign[i].second.c_str()); cur += MAXNAME;
    }
    return cur;
}

int AttrInfo::getAttrSize() const {
    int cur = sizeof(char) + sizeof(char) + sizeof(short) + MAXNAME;
    if (isForeign()) {
        cur += MAXNAME * 2;
    }
    if (hasDefault()) {
        cur += sizeof(int) + mxLen;
    }
    cur += linkedForeign.size() * MAXNAME * 2;
    return cur;
}

vector<AttrInfo> AttrInfo::loadAttrs(const char* pData) {
    vector<AttrInfo> ret;
    int n = *reinterpret_cast<const int*>(pData);
    int cur = sizeof(int);
    for (int i = 0; i < n; i++) {
        AttrInfo attr;
        int size = attr.load(pData);
        ret.push_back(attr);
        cur += size;
    }
    return ret;
}

int AttrInfo::dumpAttrs(char* pData, const std::vector<AttrInfo>& attrs) {
    *reinterpret_cast<int*>(pData) = (int)attrs.size();
    int cur = sizeof(int);
    for (const AttrInfo& attr: attrs) {
        int size = attr.dump(pData + cur);
        cur += size;
    }
    return cur;
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
                info += ",";
            info += "Has Default";
        }
        if (attr.isPrimary()) {
            if (info != "")
                info += ",";
            info += "Primary";
        }
        if (attr.isForeign()) {
            if (info != "")
                info += ",";
            info.append("Foreign ").append(attr.refTable).append(".").append(attr.refAttr);
        }
        if (info != "") {
            info.insert(0, "(");
            info.append(")");
        }
        info.append(" [");
        for (auto& x: attr.linkedForeign) {
            info.append(x.first).append(".").append(x.second).append(" ");
        }
        info.append("]");
        os << attr.attrName << info;
    }
    return os;
}

int AttrInfo::getMaxLen() const {
    return mxLen ? mxLen : getDefaultLen(type);
}
