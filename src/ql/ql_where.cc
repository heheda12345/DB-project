#include "ql.h"
using namespace std;

bool SingleWhere::satisfy(const TableLine& value) const {
    switch (whereType) {
        case TY_OP_COL: {
            assert(idx1 >= 0 && idx1 < value.size());
            assert(idx2 >= 0 && idx2 < value.size());
            if (value[idx1].isNull || value[idx2].isNull)
                return 0;
            return satisfyOp(value[idx1].value, value[idx2].value, ty, op);
        }
        case TY_OP_VALUE: {
            assert(idx1 >= 0 && idx1 < value.size());
            if (value[idx1].isNull)
                return 0;
            return satisfyOp(value[idx1].value, this->val, ty, op);
        }
        case TY_NOT_NULL: {
            assert(idx1 >= 0 && idx1 < value.size());
            return !value[idx1].isNull;
        }
        case TY_IS_NULL: {
            assert(idx1 >= 0 && idx1 < value.size());
            return value[idx1].isNull;
        }
        case TY_LIKE: {
            assert(false);
            return false;
        }
    }
    assert(false);
    return false;
}

std::vector<TableLine> select(const std::vector<TableLine>& values, const std::vector<SingleWhere>& conds) {
    std::vector<TableLine> ret;
    for (auto& v: values) {
        bool ok = 1;
        for (auto &c : conds) {
            if (!c.satisfy(v)) {
                ok = 0;
                break;
            }
        }
        if (ok) {
            ret.push_back(v);
        }
    }
    return ret;
}

RC RawSingleWhere::Compile(SingleWhere& where, const vector<AttrInfo>& attrs, const std::string& tbName_i) const {
    if (hasError)
        return QL_PRE_ERROR;
    where.whereType = whereType;
    where.idx1 = AttrInfo::getPos(attrs, idx1);
    if (where.idx1 == -1)
        return QL_NO_SUCH_KEY;
    if (tbName != "" && tbName_i != "" && tbName != tbName_i) {
        return QL_NAME_NOT_MATCH;
    }
    where.ty = attrs[where.idx1].type;
    switch (whereType) {
        case SingleWhere::TY_OP_COL: {
            where.idx2 = AttrInfo::getPos(attrs, idx2);
            where.op = op;
            if (where.idx2 == -1)
                return QL_NO_SUCH_KEY;
            if (attrs[where.idx2].type != where.ty)
                return QL_TYPE_NOT_MATCH;
            break;
        }
        case SingleWhere::TY_OP_VALUE: {
            if (this->ty != where.ty)
                return QL_TYPE_NOT_MATCH;
            where.val = this->value;
            where.op = op;
            break;
        }
        case SingleWhere::TY_NOT_NULL: {
            break;
        }
        case SingleWhere::TY_IS_NULL: {
            break;
        }
        case SingleWhere::TY_LIKE: {
            assert(false);
            return false;
        }
        default: {
            assert(false);
            break;
        }
    }
    return OK_RC;
}

RC CompileWheres(std::vector<SingleWhere>& conds, const std::vector<RawSingleWhere> &rawConds, const std::vector<AttrInfo>& attrs, const std::string& tbName_i) {
    conds.clear();
    for (auto& raw: rawConds) {
        SingleWhere cond;
        RC rc = raw.Compile(cond, attrs, tbName_i);
        QLRC(rc, rc);
        conds.push_back(cond);
    }
    return OK_RC;
}