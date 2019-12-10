#include "ix.h"
#include "ix_btree.h"

RC IX_IndexHandle::InsertEntry(void *pData, const RID &rid) {
    IX_BTKEY key = IX_BTKEY((char*)pData, header.attrLength, header.attrType, rid);
    RC rc = bTree->insert(key);
    IXRC(rc, rc)
    return OK_RC;
}

RC IX_IndexHandle::DeleteEntry(void *pData, const RID &rid) {
    IX_BTKEY key = IX_BTKEY((char*)pData, header.attrLength, header.attrType, rid);
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
        RMRC(rc, IX_BTNode())
    }
    char* st;
    rc = record.GetData(st);
    if (rc) {
        RMRC(rc, IX_BTNode())
    }
    IX_BTNode ret;
    ret.load(st, header.attrLength, header.btm);
    ret.pos = pos;
    return ret;
}

IX_BTNode IX_IndexHandle::loadRoot() {
    return get(RID(header.rootPage, header.rootSlot));
}

void IX_IndexHandle::update(IX_BTNode& node) {
    printf("saver.update\n");
    RM_Record record;
    RC rc = fh.GetRec(node.pos, record);
    if (rc) {
        printf("ix error %d\n", rc);
        return;
    }
    printf("get rec end\n");
    char* st;
    rc = record.GetData(st);
    if (rc) {
        printf("ix error %d\n", rc);
        return;
    }
    node.dump(st, header.attrLength, header.btm);
    printf("get data end\n");
    rc = fh.UpdateRec(record);
    if (rc) {
        printf("ix error %d\n", rc);
        return;
    }
    printf("update end\n");
}

RID IX_IndexHandle::newNode(IX_BTNode &tr) {
    RID rid;
    char st[header.nodeSize];
    tr.dump(st, header.attrLength, header.btm);
    RC rc = fh.InsertRec(st, rid);
    if (rc) {
        RMRC(rc, RID());
    }
    tr.pos = rid;
    return rid;
}

void IX_IndexHandle::init(){
    int loaded;
    fh.GetMeta(reinterpret_cast<char*>(&header), loaded);
    // printf("load header(%d) %lld %d %d\n", loaded, header.rootPage, header.rootSlot, header.nodeSize);
    assert(loaded == sizeof(Header));
    bTree = new IX_BTree(*this);
}