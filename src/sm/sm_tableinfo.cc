#include "sm.h"
using namespace std;

int TableInfo::load(const char* pData) {
    int cur = 0;
    attrs = AttrInfo::loadAttrs(pData + cur); cur += AttrInfo::getAttrsSize(attrs);
    int primarySize = *reinterpret_cast<const int*>(pData + cur); cur += sizeof(int);
    primaryKeys.clear();
    for (int i = 0; i < primarySize; i++) {
        primaryKeys.push_back(std::string(pData + cur)); cur += MAXNAME;
    }
    foreignGroups = ForeignKeyInfo::loadForeigns(pData + cur);
    cur += ForeignKeyInfo::getForeignsSize(foreignGroups);
    linkedBy = ForeignKeyInfo::loadForeigns(pData + cur);
    cur += ForeignKeyInfo::getForeignsSize(linkedBy);
    indexes = IndexInfo::loadIndexes(pData + cur);
    cur += IndexInfo::getIndexesSize(indexes);
    return cur;
}

int TableInfo::dump(char* pData) const {
    int cur = 0;
    AttrInfo::dumpAttrs(pData + cur, attrs); cur += AttrInfo::getAttrsSize(attrs);
    *reinterpret_cast<int*>(pData + cur) = primaryKeys.size(); cur += sizeof(int);
    for (auto& x: primaryKeys) {
        dumpString(pData + cur, x); cur += MAXNAME;
    }
    ForeignKeyInfo::dumpForeigns(pData + cur, foreignGroups);
    cur += ForeignKeyInfo::getForeignsSize(foreignGroups);
    ForeignKeyInfo::dumpForeigns(pData + cur, linkedBy);
    cur += ForeignKeyInfo::getForeignsSize(linkedBy);
    IndexInfo::dumpIndexes(pData + cur, indexes);
    cur += IndexInfo::getIndexesSize(indexes);
    return cur;
}

int TableInfo::getSize() const {
    return AttrInfo::getAttrsSize(attrs)
        + sizeof(int) + MAXNAME * primaryKeys.size()
        + ForeignKeyInfo::getForeignsSize(foreignGroups)
        + ForeignKeyInfo::getForeignsSize(linkedBy)
        + IndexInfo::getIndexesSize(indexes);
}

void TableInfo::setPrimaryNotNull() {
    for (auto& x: primaryKeys) {
        int idx = AttrInfo::getPos(attrs, x);
        assert(idx != -1);
        attrs[idx].setNotNullFlag(1);
    }
}

std::ostream& operator << (std::ostream& os, const TableInfo& table) {
    os << table.attrs << endl;
    if (table.primaryKeys.size() > 0) {
        os << std::string("Primary Key:");
        for (auto& x: table.primaryKeys)
            os << std::string(" ") << x;
        os << endl;
    }
    if (table.foreignGroups.size() > 0) {
        os << "Foreign Keys:" << endl;
        for (auto& x: table.foreignGroups)
            os << x << endl;
    }
    if (table.linkedBy.size() > 0) {
        os << "Linked by:" << endl;
        for (auto& x: table.linkedBy) {
            os << x << endl;
        }
    }
    if (table.indexes.size() > 0) {
        os << "Index:" << endl;
        for (auto& x: table.indexes) {
            os << x << endl;
        }
    }
    return os;
}