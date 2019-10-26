#include "rm_internal.h"
#include <cstdio>

void RM_Attr::set(AttrType _attrType, int _attrLength, int _attrOffset, void* value) {
    if (ty == STRING)
        delete[] val.strValue;
    ty = value == nullptr ? NO_TYPE : _attrType;
    len = _attrLength;
    offset = _attrOffset;
    switch (ty) {
        case INT: {
            val.intValue = *reinterpret_cast<int*>(value);
            break;
        }
        case FLOAT: {
            val.floatValue = *reinterpret_cast<float*>(value);
            break;
        }
        case STRING: {
            val.strValue = new char[_attrLength];
            memcpy(val.strValue, value, _attrLength);
            break;
        }
    }
}

template<typename T>
bool Compute(T l, CompOp op, T r) {
    switch (op) {
        case NO_OP:
            return true;
        case EQ_OP:
            return l == r;
        case NE_OP:
            return l != r;
        case LT_OP:
            return l < r;
        case GT_OP:
            return l > r;
        case LE_OP:
            return l <= r;
        case GE_OP:
            return l >= r;
    }
    printf("unsupport op!");
    return false;
}

bool RM_Attr::Satisfy(CompOp op, const char* data) const {
    if (op == NO_OP)
        return true;
    switch (ty) {
        case INT: {
            int r = *reinterpret_cast<const int*>(data);
            return Compute(val.intValue, op, r);
            break;
        }
        case FLOAT: {
            float r = *reinterpret_cast<const float*>(data);
            return Compute(val.floatValue, op, r);
            break;
        }
        case STRING: {
            if (op == NE_OP)
                return !Satisfy(EQ_OP, data);
            if (op == GT_OP)
                return !Satisfy(LE_OP, data);
            if (op == LT_OP)
                return !Satisfy(GE_OP, data);
            const char* r = data + offset;
            for (int i=0; i<len; i++) {
                switch (op) {
                    case EQ_OP:
                        if (val.strValue[i] != r[i])
                            return false;
                        break;
                    case LE_OP:
                        if (val.strValue[i] > r[i])
                            return false;
                        break;
                    case GE_OP:
                        if (val.strValue[i] < r[i])
                            return false;
                        break;
                    default:
                        printf("unsupport op!\n");
                }
            }
            return true;
            break;
        }
    }
    printf("Should not reach here! RM_ATTR\n");
    return false;
}