#include "ql.h"
#include <iomanip>
using namespace std;

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
                std::string st = item.value;
                if (st.length() > PRINT_WIDTH - 5)
                    st = std::string(st, 0, PRINT_WIDTH - 5).append("...");
                os << st;
                break;
            }
        }
    }
    return os;
}

std::ostream& operator << (std::ostream& os, const std::vector<Item>& items) {
    os << "|";
    for (auto& item: items) {
        os << setiosflags(ios::left) << " " << setw(PRINT_WIDTH) << item << "|";
    }
    return os;
}