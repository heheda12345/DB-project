#include "ql.h"
using namespace std;

RC RawExprNode::Compile(ExprNode& node, const std::vector<AttrInfo>& attrs) const {
    if (hasError) {
        return QL_PRE_ERROR;
    }
    node.nodeType = nodeType;
    switch (nodeType) {
        case ExprNode::VALUE: {
            node.attrType = attrType;
            node.str = str;
            break;
        }
        case ExprNode::COL: {
            int idx = AttrInfo::getPos(attrs, str);
            if (idx == -1) {
                return QL_NO_SUCH_KEY;
            }
            node.attrType = attrs[idx].type;
            node.idx = idx;
            break;
        }
        case ExprNode::ADD: // no break
        case ExprNode::SUB: // no break
        case ExprNode::MUL: // no break
        case ExprNode::DIV: {
            assert(sons.size() == 2);
            ExprNode l, r;
            RC rc = sons[0].Compile(l, attrs);
            if (rc != OK_RC)
                return rc;
            rc = sons[1].Compile(r, attrs);
            if (rc != OK_RC)
                return rc;
            if (l.attrType == NO_TYPE || r.attrType == NO_TYPE) {
                node.attrType = NO_TYPE;
            } else {
                if (l.attrType != r.attrType) {
                    return QL_TYPE_NOT_MATCH;
                }
                node.attrType = l.attrType;
                if (node.nodeType == ExprNode::DIV) {
                    if (node.attrType != FLOAT)
                        return QL_TYPE_NOT_MATCH;
                } else {
                    if (node.attrType != INT && node.attrType != FLOAT)
                        return QL_TYPE_NOT_MATCH;
                }
            }
            node.sons.clear();
            node.sons.push_back(l);
            node.sons.push_back(r);
            break;
        }
    }
    return OK_RC;
}

Item ExprNode::compute(const TableLine& in) const {
    if (attrType == NO_TYPE) {
        return Item::NullItem();
    }
    
    switch (nodeType) {
        case ExprNode::VALUE: {
            Item ret;
            ret.value = str;
            ret.isNull = 0;
            ret.type = attrType;
            return ret;
        }
        case ExprNode::COL: {
            return in[idx];
        }
        case ExprNode::ADD: {
            assert(sons.size() == 2);
            Item l = sons[0].compute(in), r = sons[1].compute(in);
            if (l.isNull || r.isNull)
                return Item::NullItem();
            Item ret;
            ret.value = ComputeAdd(l.value, r.value, attrType);
            ret.isNull = 0;
            ret.type = attrType;
            return ret;
        }
        case ExprNode::SUB: {
            assert(sons.size() == 2);
            Item l = sons[0].compute(in), r = sons[1].compute(in);
            if (l.isNull || r.isNull)
                return Item::NullItem();
            Item ret;
            ret.value = ComputeSub(l.value, r.value, attrType);
            ret.isNull = 0;
            ret.type = attrType;
            return ret;
        }
        case ExprNode::MUL: {
            assert(sons.size() == 2);
            Item l = sons[0].compute(in), r = sons[1].compute(in);
            if (l.isNull || r.isNull)
                return Item::NullItem();
            Item ret;
            ret.value = ComputeMul(l.value, r.value, attrType);
            ret.isNull = 0;
            ret.type = attrType;
            return ret;
        }
        case ExprNode::DIV: {
            assert(sons.size() == 2);
            Item l = sons[0].compute(in), r = sons[1].compute(in);
            if (l.isNull || r.isNull)
                return Item::NullItem();
            Item ret;
            ret.value = ComputeDiv(l.value, r.value, attrType);
            ret.isNull = 0;
            ret.type = attrType;
            return ret;
        }
    }
}

std::vector<TableLine> DoSetJobs(const std::vector<TableLine>& values, const std::vector<SetJob>& setJobs) {
    std::vector<TableLine> ret;
    for (auto& value: values) {
        TableLine output = value;
        for (auto& job: setJobs) {
            output[job.target] = job.expr.compute(value);
        }
        ret.push_back(output);
    }
    return ret;
}

RC CompileSetJobs(std::vector<SetJob>& setJobs, const std::vector<RawSetJob>& rawJobs, const std::vector<AttrInfo>& attrs) {
    vector<string> names;
    for (auto& job: rawJobs)
        names.push_back(job.target);
    if (isDumplicated(names)) {
        return QL_DUMPLICATED;
    }
    setJobs.clear();
    for (auto& rawJob: rawJobs) {
        SetJob job;
        int pos = AttrInfo::getPos(attrs, rawJob.target);
        if (pos == -1) {
            return QL_NO_SUCH_KEY;
        }
        job.target = pos;
        RC rc = rawJob.expr.Compile(job.expr, attrs);
        QLRC(rc, rc);
        setJobs.push_back(job);
    }
    return OK_RC;
}