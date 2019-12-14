#include "ix.h"
#include "ix_btree.h"

IX_IndexScan::IX_IndexScan(): state(UNSTART) {

};

RC IX_IndexScan::OpenScan(IX_IndexHandle &indexHandle,
                          CompOp      compOp,
                          const std::vector<std::string> &value,
                          ClientHint  pinHint) {
    if (state != UNSTART) {
        return IX_SCAN_OPENED;
    }
    handle = &indexHandle;
    while (!entrys.empty()) {
        entrys.pop();
    }
    IX_BTKEY key(value, indexHandle.header.attrLength, indexHandle.header.attrType, RID()); // NOTE: pass an invalid rid
    RC rc = search(RID(handle->header.rootPage, handle->header.rootSlot), compOp, key, true);
    IXRC(rc, IX_BTREE)
    state = RUNNING;
    printf("Finish Open Scan\n");
    return OK_RC;
}          

RC IX_IndexScan::GetNextEntry(RID &rid) {
    if (!entrys.empty()) {
        rid = entrys.front(); entrys.pop();
        return OK_RC;
    } else {
        return IX_EOF;
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
    // printf("search (%lld %d), needCheck %d\n", pos.GetPageNum(), pos.GetSlotNum(), needCheck);
    RC rc;
    IX_BTNode node = handle->get(pos);
    // node.outit();
    if (!needCheck) {
        for (int i=0; i<node.key.size(); i++) {
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
            for (int i=0; i<node.key.size(); i++) {
                rc = search(node.child[i], compOp, key, false);
                IXRC(rc, IX_BTREE)
                entrys.push(node.key[i].rid);
            }
            search(*--node.child.end(), compOp, key, false);
            IXRC(rc, IX_BTREE)
            return OK_RC;
        }
        case EQ_OP: {
            bool isFirst = true;
            for (int i=0; i<node.key.size(); i++) {
                key.rid = node.key[i].rid;
                int xx = node.key[i].cmp(key);
                switch (xx) {
                    case -1: {
                        break;
                    }
                    case 0: {
                        rc = search(node.child[i], compOp, key, isFirst);
                        IXRC(rc, IX_BTREE)
                        isFirst = false;
                        entrys.push(node.key[i].rid);
                        break;
                    }
                    case 1: {
                        rc = search(node.child[i], compOp, key, true);
                        IXRC(rc, IX_BTREE)
                        return OK_RC;
                    }
                }
            }
            rc = search(*--node.child.end(), compOp, key, true);
            IXRC(rc, IX_BTREE)
            return OK_RC;
        }
        case NE_OP: {
            bool isFirst = true;
            for (int i=0; i<node.key.size(); i++) {
                key.rid = node.key[i].rid;
                int xx = node.key[i].cmp(key);
                switch (xx) {
                    case -1: {
                        rc = search(node.child[i], compOp, key, false);
                        IXRC(rc, IX_BTREE)
                        entrys.push(node.key[i].rid);
                        break;
                    }
                    case 0: {
                        rc = search(node.child[i], compOp, key, true);
                        IXRC(rc, IX_BTREE)
                        break;
                    }
                    case 1: {
                        rc = search(node.child[i], compOp, key, isFirst);
                        IXRC(rc, IX_BTREE)
                        entrys.push(node.key[i].rid);
                        isFirst = false;
                        break;
                    }
                }
            }
            rc = search(*--node.child.end(), compOp, key, isFirst);
            IXRC(rc, IX_BTREE)
            return OK_RC;
        }
        case LE_OP: {
            for (int i=0; i<node.key.size(); i++) {
                key.rid = node.key[i].rid;
                int xx = node.key[i].cmp(key);
                switch (xx) {
                    case -1: {
                        rc = search(node.child[i], compOp, key, false);
                        IXRC(rc, IX_BTREE)
                        entrys.push(node.key[i].rid);
                        break;
                    }
                    case 0: {
                        rc = search(node.child[i], compOp, key, false);
                        IXRC(rc, IX_BTREE)
                        entrys.push(node.key[i].rid);
                        break;
                    }
                    case 1: {
                        rc = search(node.child[i], compOp, key, true);
                        IXRC(rc, IX_BTREE)
                        return OK_RC;
                    }
                }
            }
            rc = search(*--node.child.end(), compOp, key, true);
            IXRC(rc, IX_BTREE)
            return OK_RC;
        }
        case LT_OP: {
            for (int i=0; i<node.key.size(); i++) {
                key.rid = node.key[i].rid;
                int xx = node.key[i].cmp(key);
                switch (xx) {
                    case -1: {
                        rc = search(node.child[i], compOp, key, false);
                        IXRC(rc, IX_BTREE)
                        entrys.push(node.key[i].rid);
                        break;
                    }
                    case 0: {
                        rc = search(node.child[i], compOp, key, true);
                        IXRC(rc, IX_BTREE)
                        return OK_RC;
                    }
                    case 1: {
                        rc = search(node.child[i], compOp, key, true);
                        IXRC(rc, IX_BTREE)
                        return OK_RC;
                    }
                }
            }
            rc = search(*--node.child.end(), compOp, key, true);
            IXRC(rc, IX_BTREE)
            return OK_RC;
        }
        case GE_OP: {
            bool isFirst = true;
            for (int i=0; i<node.key.size(); i++) {
                key.rid = node.key[i].rid;
                int xx = node.key[i].cmp(key);
                switch (xx) {
                    case -1: {
                        break;
                    }
                    case 0: {
                        rc = search(node.child[i], compOp, key, isFirst);
                        IXRC(rc, IX_BTREE)
                        entrys.push(node.key[i].rid);
                        isFirst = false;
                        break;
                    }
                    case 1: {
                        rc = search(node.child[i], compOp, key, isFirst);
                        IXRC(rc, IX_BTREE)
                        entrys.push(node.key[i].rid);
                        isFirst = false;
                        break;
                    }
                }
            }
            rc = search(*--node.child.end(), compOp, key, isFirst);
            IXRC(rc, IX_BTREE)
            return OK_RC;
        }
        case GT_OP: {
            bool isFirst = true;
            for (int i=0; i<node.key.size(); i++) {
                key.rid = node.key[i].rid;
                int xx = node.key[i].cmp(key);
                switch (xx) {
                    case -1: {
                        break;
                    }
                    case 0: {
                        break;
                    }
                    case 1: {
                        rc = search(node.child[i], compOp, key, isFirst);
                        IXRC(rc, IX_BTREE)
                        entrys.push(node.key[i].rid);
                        isFirst = false;
                        break;
                    }
                }
            }
            rc = search(*--node.child.end(), compOp, key, isFirst);
            IXRC(rc, IX_BTREE)
            return OK_RC;
        }
    }
    assert(false);
    return IX_INVALIDCOMP;
}
