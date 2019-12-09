#include "ix.h"
#include "ix_btree.h"

IX_IndexScan::IX_IndexScan(){

};

RC IX_IndexScan::OpenScan(IX_IndexHandle &indexHandle,
                          CompOp      compOp,
                          void        *value,
                          ClientHint  pinHint) {
    if (state != UNSTART) {
        return IX_SCAN_OPENED;
    }
    handle = &indexHandle;
    while (!entrys.empty()) {
        entrys.pop();
    }
    IX_BTKEY key((char*)value, indexHandle.header.attrLength, indexHandle.header.attrType);
    RC rc = search(RID(handle->header.rootPage, handle->header.rootSlot), compOp, key, true);
    IXRC(rc, IX_BTREE)
    state = RUNNING;
    return OK_RC;
}          

RC IX_IndexScan::GetNextEntry(RID &rid) {
    if (!entrys.empty()) {
        rid = entrys.front();
        return OK_RC;
    } else {
        return IX_SCAN_END;
    }
}

RC IX_IndexScan::CloseScan() {
    if (state != RUNNING)
        return IX_SCAN_CLOSED;
    else {
        state = UNSTART;
        while (!entrys.empty()) {
            entrys.pop();
        }
        return OK_RC;
    }
}

RC IX_IndexScan::search(RID pos, CompOp compOp, IX_BTKEY &key, bool needCheck) { //key.rid can always change
    if (!pos.isValid())
        return OK_RC;
    RC rc;
    IX_BTNode node = handle->get(pos);
    if (!needCheck) {
        for (int i=0; i<node.child.size(); i++) {
            rc = search(node.child[i], compOp, key, false);
            IXRC(rc, IX_BTREE)
            entrys.push(node.key[i].rid);
        }
        search(*--node.child.end(), compOp, key, false);
        IXRC(rc, IX_BTREE)
        return OK_RC;
    }
    switch (compOp) {
        case NO_OP: {
            for (int i=0; i<node.child.size(); i++) {
                rc = search(node.child[i], compOp, key, false);
                IXRC(rc, IX_BTREE)
                entrys.push(node.key[i].rid);
            }
            search(*--node.child.end(), compOp, key, false);
            IXRC(rc, IX_BTREE)
            return OK_RC;
        }
        case EQ_OP: { // SOS
            int last = node.key.size();
            bool need = true;
            for (int i=0; i<node.key.size(); i++) {
                key.rid = node.key[i].rid;
                int xx = node.key[i].cmp(key);
                if (xx >= 0) {
                    rc = search(node.child[i], compOp, key, need);
                    IXRC(rc, IX_BTREE)
                }
                if (xx == 0)
                    entrys.push(node.key[i].rid);
                if (xx > 0) {
                    last = i;
                    break;
                }
            }
            search(node.child[last], compOp, key, true);
            return OK_RC;
        }
        case NE_OP: {
            for (int i=0; i<node.child.size(); i++) {
                key.rid = node.key[i].rid;
                int xx = node.key[i].cmp(key);
                rc = search(node.child[i], compOp, key, true);
                IXRC(rc, IX_BTREE)
                if (xx != 0)
                    entrys.push(node.key[i].rid);
            }
            search(*--node.child.end(), compOp, key, false);
            IXRC(rc, IX_BTREE)
            return OK_RC;
        }
        case LE_OP: {
            int last = node.key.size();
            for (int i=0; i<node.key.size(); i++) {
                key.rid = node.key[i].rid;
                int xx = node.key[i].cmp(key);
                if (xx >= 0) {
                    rc = search(node.child[i], compOp, key, false);
                    IXRC(rc, IX_BTREE)
                }
                if (xx <= 0)
                    entrys.push(node.key[i].rid);
                if (xx > 0) {
                    last = i;
                    break;
                }
            }
            rc = search(node.child[last], compOp, key, true);
            IXRC(rc, IX_BTREE)
            return OK_RC;
        }
        case LT_OP: {
            int last = node.key.size();
            for (int i=0; i<node.key.size(); i++) {
                key.rid = node.key[i].rid;
                int xx = node.key[i].cmp(key);
                if (xx > 0) {
                    rc = search(node.child[i], compOp, key, false);
                    IXRC(rc, IX_BTREE)
                }
                if (xx < 0)
                    entrys.push(node.key[i].rid);
                if (xx >= 0) {
                    last = i;
                    break;
                }
            }
            rc = search(node.child[last], compOp, key, true);
            IXRC(rc, IX_BTREE)
            return OK_RC;
        }
        case GE_OP: {
            int last = 0;
            for (int i=node.key.size(); i>0; i--) {
                key.rid = node.key[i-1].rid;
                int xx = node.key[i-1].cmp(key);
                if (xx <= 0) {
                    rc = search(node.child[i], compOp, key, false);
                    IXRC(rc, IX_BTREE)
                }
                if (xx >= 0)
                    entrys.push(node.key[i].rid);
                if (xx >= 0) {
                    last = i;
                    break;
                }
            }
            rc = search(node.child[last], compOp, key, true);
            IXRC(rc, IX_BTREE)
            return OK_RC;
        }
        case GT_OP: {
            int last = 0;
            for (int i=node.key.size(); i>0; i--) {
                key.rid = node.key[i-1].rid;
                int xx = node.key[i-1].cmp(key);
                if (xx <= 0) {
                    rc = search(node.child[i], compOp, key, false);
                    IXRC(rc, IX_BTREE)
                }
                if (xx >= 0)
                    entrys.push(node.key[i].rid);
                if (xx >= 0) {
                    last = i;
                    break;
                }
            }
            rc = search(node.child[last], compOp, key, true);
            IXRC(rc, IX_BTREE)
            return OK_RC;
        }
    }
    assert(false);
    return IX_INVALIDCOMP;
}
