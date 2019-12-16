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

std::pair<std::vector<TableLine>, std::vector<RID>> select(const std::vector<TableLine>& values, const std::vector<RID>& rids, const std::vector<SingleWhere>& conds) {
    std::vector<TableLine> retValues;
    std::vector<RID> retRIDs;
    assert(values.size() == rids.size());
    for (int i = 0; i < values.size(); i++) {
        bool ok = 1;
        for (auto &c : conds) {
            if (!c.satisfy(values[i])) {
                ok = 0;
                break;
            }
        }
        if (ok) {
            retValues.push_back(values[i]);
            retRIDs.push_back(rids[i]);
        }
    }
    return make_pair(retValues, retRIDs);
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

RC CompileWheres(std::map<std::string, std::vector<SingleWhere>>& conds, const std::vector<RawSingleWhere> &rawConds, const std::map<std::string, std::vector<AttrInfo>>& attrs) {
    conds.clear();
    for (auto &raw: rawConds) {
        SingleWhere cond;
        if (attrs.find(raw.tbName) == attrs.end()) {
            return QL_NO_SUCH_KEY;
        }
        RC rc = raw.Compile(cond, attrs.find(raw.tbName)->second, raw.tbName);
        QLRC(rc, rc);
        conds[raw.tbName].push_back(cond);
    }
    return OK_RC;
}

RC RawDualWhere::compile(DualWhere& where, const std::map<std::string, std::vector<AttrInfo>>& attrs) const{
    if (hasError) {
        return QL_PRE_ERROR;
    }

    where.tbName1 = tbName1;
    auto x = attrs.find(tbName1);
    if (x == attrs.end()) {
        return QL_NO_SUCH_KEY;
    }
    int idx = AttrInfo::getPos(x->second, idx1);
    if (idx == -1) {
        return QL_NO_SUCH_KEY;
    }
    where.idx1 = idx;
    AttrType ty = x->second[idx].type;

    where.tbName2 = tbName2;
    x = attrs.find(tbName2);
    if (x == attrs.end()) {
        return QL_NO_SUCH_KEY;
    }
    idx = AttrInfo::getPos(x->second, idx2);
    if (idx == -1) {
        return QL_NO_SUCH_KEY;
    }
    where.idx2 = idx;
    if (ty != x->second[idx].type) {
        return QL_TYPE_NOT_MATCH;
    }

    return OK_RC;
}

RC CompileDualWheres(std::vector<DualWhere>& conds, const std::vector<RawDualWhere>& rawConds, const std::map<std::string, std::vector<AttrInfo>>& attrs) {
    conds.clear();
    for (auto& raw: rawConds) {
        DualWhere cond;
        RC rc = raw.compile(cond, attrs);
        QLRC(rc, rc);
        conds.push_back(cond);
    }
    return OK_RC;
}