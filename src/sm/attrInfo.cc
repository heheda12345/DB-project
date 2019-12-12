#include "sm.h"

 AttrInfo::AttrInfo(const AttrType& _type, unsigned short _mxLen, const std::string& _attrName, bool notNull, bool isPrimary): type(_type), flag(0), mxLen(_mxLen), attrName(_attrName), refTable(""), refAttr("") {
    assert(attrName.length() < MAXNAME);
    setNotNull(notNull);
    setPrimary(isPrimary);
    setForeign(0);
}

AttrInfo::AttrInfo(const AttrType& _type, unsigned short _mxLen, const std::string& _attrName, bool notNull, const std::string &_refTable, const std::string& _refAttr): type(_type), flag(0), mxLen(_mxLen), attrName(_attrName), refTable(_refTable), refAttr(_refAttr) {
    assert(attrName.length() < MAXNAME);
    assert(refTable.length() < MAXNAME);
    assert(refAttr.length() < MAXNAME);
    setNotNull(notNull);
    setPrimary(0);
    setForeign(1);
}

void AttrInfo::setNotNull(bool b) { setBit(flag, 0, b); }
bool AttrInfo::isNotNull() const { return getBit(flag, 0); }

void AttrInfo::setPrimary(bool b) {
    if (b) {
        assert(!isForeign());
    }
    setBit(flag, 1, b);
}
bool AttrInfo::isPrimary() const { return getBit(flag, 1); }

void AttrInfo::setForeign(bool b) { 
    if (b) {
        assert(!isPrimary());
    }
    setBit(flag, 2, b);
}
bool AttrInfo::isForeign() const { assert(!isPrimary()); return getBit(flag, 2); }

void AttrInfo::load(const char* pData) {
    type = (AttrType)*reinterpret_cast<const unsigned char*>(pData);
    flag = *reinterpret_cast<const unsigned char*>(pData + sizeof(char));
    mxLen = *reinterpret_cast<const unsigned short*>(pData + sizeof(char) * 2);
    attrName = std::string(pData + sizeof(char) * 2 + sizeof(short));
    refTable = std::string(pData + sizeof(char) * 2 + sizeof(short) + MAXNAME);
    refAttr = std::string(pData + sizeof(char) * 2 + sizeof(short) + MAXNAME * 2);
}

void AttrInfo::dump(char* pData) {
    *reinterpret_cast<unsigned char*>(pData) = (unsigned char) type;
    *reinterpret_cast<unsigned char*>(pData + sizeof(char)) = flag;
    *reinterpret_cast<unsigned short*>(pData + sizeof(char) * 2) = mxLen;
    dumpString(pData + sizeof(char) * 2 + sizeof(short), attrName);
    dumpString(pData + sizeof(char) * 2 + sizeof(short) + MAXNAME, refTable);
    dumpString(pData + sizeof(char) * 2 + sizeof(short) + MAXNAME * 2, refAttr);
}
