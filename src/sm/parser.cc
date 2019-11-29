#include "parser.h"

std::ostream &operator<<(std::ostream &s, const RelAttr &ra) {
    return s;
}

std::ostream &operator<<(std::ostream &s, const Value &v) {
    return s;
}

std::ostream &operator<<(std::ostream &s, const Condition &c) {
    return s;
}

std::ostream &operator<<(std::ostream &s, const CompOp &op) {
    return s;
}
std::ostream &operator<<(std::ostream &s, const AttrType &at) {
    return s;
}

void RBparse(PF_Manager &pfm, SM_Manager &smm, QL_Manager &qlm) {

}

void PrintError(RC rc) {
    printf("parser error %d\n", rc);
}