#include "ix.h"

RC IX_IndexHandle::InsertEntry(void *pData, const RID &rid) {
    IX_BTKEY key = IX_BTKEY((char*)pData, header.attrLength, header.attrType);
    RC rc = bTree->insert(key);
    IXRC(rc, rc)
    return OK_RC;
}

RC IX_IndexHandle::DeleteEntry(void *pData, const RID &rid) {
    IX_BTKEY key = IX_BTKEY((char*)pData, header.attrLength, header.attrType);
    RC rc = bTree->remove(key);
    IXRC(rc, rc)
    return OK_RC;
}

RC IX_IndexHandle::ForcePages() {
    RC rc = fh.ForcePages();
    RMRC(rc, IX_RM)
    return OK_RC;
}

IX_BTNode IX_IndexHandle::get(RID pos) {
    RM_Record record;
    RC rc = fh.GetRec(pos, record);
    if (rc) {
        printf("ix error %d\n", rc);
        return IX_BTNode();
    }
    char* st;
    rc = record.GetData(st);
    if (rc) {
        printf("ix error %d\n", rc);
        return IX_BTNode();
    }
    IX_BTNode ret;
    ret.load(st, header.attrLength, header.btm);
    ret.pos = pos;
    return ret;
}

void IX_IndexHandle::update(IX_BTNode& node) {
    RM_Record record;
    RC rc = fh.GetRec(node.pos, record);
    if (rc) {
        printf("ix error %d\n", rc);
        return;
    }
    char* st;
    rc = record.GetData(st);
    if (rc) {
        printf("ix error %d\n", rc);
        return;
    }
    node.dump(st, header.attrLength, header.btm);

    rc = fh.UpdateRec(record);
    if (rc) {
        printf("ix error %d\n", rc);
        return;
    }
}

RID IX_IndexHandle::newNode(IX_BTNode &tr) {
    RID rid;
    char st[header.nodeSize];
    tr.dump(st, header.attrLength, header.btm);
    RC rc = fh.InsertRec(st, rid);
    if (rc) {
        printf("ix error %d\n", rc);
        return RID();
    }
    tr.pos = rid;
    return rid;
}