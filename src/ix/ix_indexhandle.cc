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
    // printf("getting %lld %d\n", pos.GetPageNum(), pos.GetSlotNum());
    RM_Record record;
    RC rc = fh.GetRec(pos, record);
    RMRC(rc, IX_BTNode())
    char* st;
    rc = record.GetData(st);
    RMRC(rc, IX_BTNode());
    IX_BTNode ret;
    ret.load(st, header.attrLength, header.btm);
    ret.pos = pos;
    // printf("[get]"); ret.outit();
    return ret;
}

IX_BTNode IX_IndexHandle::loadRoot() {
    return get(RID(header.rootPage, header.rootSlot));
}

void IX_IndexHandle::setRoot(const RID& rid) {
    printf("[SetRoot] (%lld %d)\n", rid.GetPageNum(), rid.GetSlotNum());
    header.rootPage = rid.GetPageNum();
    header.rootSlot = rid.GetSlotNum();
    RC rc = fh.SetMeta(reinterpret_cast<char*>(&header), sizeof(IX_IndexHandle::Header));
    if (rc) {
        IX_PrintError(rc);
        return;
    }
}

void IX_IndexHandle::update(IX_BTNode& node) {
    // printf("[update]"); node.outit();
    RM_Record record;
    RC rc = fh.GetRec(node.pos, record);
    if (rc) {
        IX_PrintError(rc);
        return;
    }
    char* st;
    rc = record.GetData(st);
    if (rc) {
        IX_PrintError(rc);
        return;
    }
    node.dump(st, header.attrLength, header.btm);
    rc = fh.UpdateRec(record);
    if (rc) {
        IX_PrintError(rc);
        return;
    }
}

RID IX_IndexHandle::newNode(IX_BTNode &tr) {
    RID rid;
    char st[header.nodeSize];
    tr.dump(st, header.attrLength, header.btm);
    RC rc = fh.InsertRec(st, rid);
    RMRC(rc, RID());
    tr.pos = rid;
    // printf("new Node at rid (%lld %d)\n", rid.GetPageNum(), rid.GetSlotNum());
    return rid;
}

void IX_IndexHandle::deleteNode(IX_BTNode &tr) {
    RC rc = fh.DeleteRec(tr.pos);
    if (rc) {
        IX_PrintError(rc);
        return;
    }
}

void IX_IndexHandle::init(){
    int loaded;
    fh.GetMeta(reinterpret_cast<char*>(&header), loaded);
    // printf("load header(%d) %lld %d %d\n", loaded, header.rootPage, header.rootSlot, header.nodeSize);
    assert(loaded == sizeof(Header));
    if (hasInit)
        delete bTree;
    bTree = new IX_BTree(*this);
    hasInit = true;
}