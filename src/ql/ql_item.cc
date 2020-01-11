#include "ql.h"
#include <iomanip>
using namespace std;

int Item::dump(char* pData, const AttrInfo& a) const {
    int cur = 0;
    *reinterpret_cast<char*>(pData + cur) = (char)isNull; cur += sizeof(char);
    *reinterpret_cast<int*>(pData + cur) = (int) value.length(); cur += sizeof(int);
    memcpy(pData+cur, value.c_str(), value.length()); cur += a.mxLen;
    // printf("[Item] dump(%d) %d %d\n", cur, (int)isNull, (int)value.length());
    return cur;
}

int Item::load(const char* pData, const AttrInfo& a) {
    int cur = 0;
    isNull = *reinterpret_cast<const char*>(pData + cur); cur += sizeof(char);
    int len = *reinterpret_cast<const int*>(pData + cur); cur += sizeof(int);
    assert(len <= a.mxLen);
    value = std::string(pData + cur, len); cur += a.mxLen;
    type = isNull ? NO_TYPE : a.type;
    // printf("[Item] load(%d) %d %d\n", cur, (int)isNull, len);
    return cur;
}

int Item::getSize(const AttrInfo& a) const {
    // printf("[Item] getsize %d\n", (int)(sizeof(char) + sizeof(int) + a.mxLen));
    return sizeof(char) + sizeof(int) + a.mxLen;
}

int Item::dumpTableLine(char* pData, const TableLine& items, const std::vector<AttrInfo>& as) {
    assert(items.size() == as.size());
    int cur = 0;
    for (int i=0; i < as.size(); i++) {
        cur += items[i].dump(pData + cur, as[i]);
    }
    // printf("[Line] dump %d\n", cur);
    return cur;
}

TableLine Item::loadTableLine(const char* pData, const std::vector<AttrInfo>& as) {
    TableLine ret;
    int cur = 0;
    for (auto& a: as) {
        Item item;
        cur += item.load(pData + cur, a);
        ret.push_back(item);
    }
    // printf("[Line] load %d\n", cur);
    return ret;
}

int Item::getLineSize(const std::vector<AttrInfo>& as) {
    // printf("[Line] getSize %d\n", AttrInfo::getRecordSize(as));
    return AttrInfo::getRecordSize(as);
}
Item Item::NullItem() {
    Item ret;
    ret.value = std::string();
    ret.isNull = 1;
    ret.type = NO_TYPE;
    return ret;
}

std::ostream& operator << (std::ostream& os, const Item& item) {
    if (item.isNull) {
        os << "NULL";
    } else {
        switch (item.type) {
            case INT: {
                int x = *reinterpret_cast<const int*>(item.value.c_str());
                os << x;
                break;
            }
            case DATE: {
                int x = *reinterpret_cast<const int*>(item.value.c_str());
                int year = x / 10000, month = x %10000 / 100, day = x % 100;
                std::string sy = std::to_string(year), sm = std::to_string(month), sd = std::to_string(day);
                while (sy.length() < 4) { sy = sy.insert(0, "0"); }
                while (sm.length() < 2) { sm = sm.insert(0, "0"); }
                while (sd.length() < 2) { sd = sd.insert(0, "0"); }
                std::string st = sy.append("-").append(sm).append("-").append(sd);
                os << st;
                break;
            }
            case FLOAT: {
                float x = *reinterpret_cast<const float*>(item.value.c_str());
                os << x;
                break;
            }
            case STRING: {
                std::string st = cutForPrint(item.value);
                os << st;
                break;
            }
        }
    }
    return os;
}

std::ostream& operator << (std::ostream& os, const TableLine& items) {
    os << "|";
    for (auto& item: items) {
        os << setiosflags(ios::left) << " " << setw(PRINT_WIDTH) << item << "|";
    }
    return os;
}

RC formatItem(const TableInfo& table, TableLine& items) {
    if (table.attrs.size() != items.size())
        return QL_LEN_NOT_MATCH;
    int n = table.attrs.size();
    for (int i=0; i<n; i++) {
        AttrInfo a = table.attrs[i];
        Item& item = items[i];
        if (a.hasDefault() && item.isNull) {
            item.value = a.dVal;
            item.type = a.type;
            item.isNull = 0;
        }
        if (a.isNotNull() && item.isNull) {
            return QL_REQUIRE_NOT_NULL;
        }
        if (item.type != NO_TYPE && a.type != item.type) {
            return QL_TYPE_NOT_MATCH;
        }
        if (item.type != NO_TYPE && item.value.length() > a.getMaxLen()) {
            return QL_ATTR_TO_LONG;
        }
    }
    return OK_RC;
}

RC toTableLine(const TableInfo& table, TableLine &items, const vector<std::string>& raws) {
    if (table.attrs.size() != raws.size())
        return QL_TYPE_NOT_MATCH;
    items.clear();
    // assume no null value;
    for (int i = 0; i < table.attrs.size(); i++) {
        Item item;
        switch (table.attrs[i].type) {
            case INT: {
                int x;
                try { x = stoi(raws[i]); } catch (const std::exception& e) { return QL_TYPE_NOT_MATCH; }
                item.type = INT;
                item.value = std::string(reinterpret_cast<char*>(&x), sizeof(int));
                item.isNull = false;
                break;
            }
            case FLOAT: {
                float x;
                try { x = stof(raws[i]); } catch (const std::exception& e) { return QL_TYPE_NOT_MATCH; }
                item.type = FLOAT;
                item.value = std::string(reinterpret_cast<char*>(&x), sizeof(float));
                item.isNull = false;
                break;
            }
            case STRING: {
                item.type = STRING;
                item.value = raws[i];
                item.isNull = false;
                break;
            }
            case DATE: {
                std::string dt = raws[i];
                if (dt.length() != 10 || dt[4] != '-' || dt[7] != '-') {
                    return QL_TYPE_NOT_MATCH;
                }
                int x;
                try {
                    x = stoi(raws[i].substr(0, 4)) * 10000 + stoi(raws[i].substr(5, 2)) * 100 + stoi(raws[i].substr(8, 2)); 
                } catch (const std::exception& e) { return QL_TYPE_NOT_MATCH; }
                item.type = DATE;
                item.value = std::string(reinterpret_cast<char*>(&x), sizeof(int));
                item.isNull = false;
                break;
            }
            default: {
                assert(false);
            }
        }
        items.push_back(item);
    }
};

std::vector<std::string> formatIndex(const std::vector<AttrInfo>& as, const std::vector<std::string>& attrNames, const TableLine& value) {
    assert(as.size() == value.size());
    std::vector<std::string> ret;
    for (auto& s: attrNames) {
        int idx = AttrInfo::getPos(as, s);
        assert(idx != -1);
        ret.push_back(value[idx].value);
    }
    return ret;
}

bool allNull(const std::vector<AttrInfo>& as, const std::vector<std::string>& attrNames, const TableLine& value) {
    assert(as.size() == value.size());
    for (auto& s: attrNames) {
        int idx = AttrInfo::getPos(as, s);
        assert(idx != -1);
        if (!value[idx].isNull) {
            return 0;
        }
    }
    return 1;
}

bool hasNull(const std::vector<AttrInfo>& as, const std::vector<std::string>& attrNames, const TableLine& value) {
    assert(as.size() == value.size());
    for (auto& s: attrNames) {
        int idx = AttrInfo::getPos(as, s);
        assert(idx != -1);
        if (value[idx].isNull)
            return 1;
    }
    return 0;
}