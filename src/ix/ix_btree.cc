#include "ix_btree.h"
#include "ix.h"

IX_BTKEY::IX_BTKEY(char* pData, int attrLen) { // from saver
        rid.loadFrom(pData);
        ty = *reinterpret_cast<AttrType*>(pData + RID::getSize());
        attr = std::string(pData + RID::getSize() + sizeof(AttrType), attrLen);
    }

IX_BTKEY::IX_BTKEY(char* pData, int attrLen, AttrType ty, const RID& rid): rid(rid), ty(ty), attr(pData, attrLen) {}

void IX_BTKEY::toCharArray(char* pData) {
    rid.dumpTo(pData);
    *reinterpret_cast<AttrType*>(pData + RID::getSize()) = ty;
    memcpy(pData + RID::getSize() + sizeof(AttrType), attr.c_str(), attr.length());
}

int IX_BTKEY::cmp(const IX_BTKEY &that) const {
    // printf("cmp %d %d\n", ty, that.ty);
    assert(ty == that.ty);
    assert(attr.length() == that.attr.length());
    switch (ty) {
        case INT: {
            int l = *reinterpret_cast<const int*>(attr.c_str()),
                r = *reinterpret_cast<const int*>(that.attr.c_str());
            // printf("l %d r %d\n", l, r);
            if (l != r)
                return l < r ? -1 : 1;
        }
        case FLOAT: {
            float fl = *reinterpret_cast<const float*>(attr.c_str()),
                  fr = *reinterpret_cast<const float*>(that.attr.c_str());
            if (fl != fr)
                return fl < fr ? -1 : 1;
        }
        case STRING: {
            std::string l = attr, r = that.attr;
            for (int i=0; i<l.length(); i++)
                if (l[i] != r[i])
                    return l[i] < r[i] ? -1 : 1;
        }
    }
    if (rid.isValid() && that.rid.isValid()) {
        if (rid < that.rid)
            return -1;
        if (rid == that.rid)
            return 0;
        return 1;
    } else {
        return 0;
    }
}

int IX_BTKEY::getSize(int attrLen) {
    return RID::getSize() + sizeof(AttrType) + attrLen;
}

int IX_BTKEY::search(const std::vector<IX_BTKEY> vec, IX_BTKEY e) {
    // printf("search size %d\n", vec.size());
    // for (auto x: vec) { printf("(%d: %lld %d) ", *reinterpret_cast<const int*>(x.attr.c_str()), x.rid.GetPageNum(), x.rid.GetSlotNum()); } printf("\n");
    for (int i=0; i<vec.size(); i++)
        if (e < vec[i])
            return i-1;
    return vec.size() - 1;
}


IX_BTNode::IX_BTNode(): parent(), pos(){
        child.push_back(RID());
    }

IX_BTNode::IX_BTNode(IX_IndexHandle &saver): IX_BTNode(){
    pos = saver.newNode(*this);
}

IX_BTNode::IX_BTNode(IX_IndexHandle &saver, IX_BTKEY e, RID lc, RID rc): parent(), pos(){
    key.push_back(e);
    child.push_back(lc);
    child.push_back(rc);
    pos = saver.newNode(*this);
    if (lc.isValid()) {
        IX_BTNode lst = saver.get(lc);
        lst.parent = pos;
        saver.update(lst);
    }
    if (rc.isValid()) {
        IX_BTNode rst = saver.get(rc);
        rst.parent = pos;
        saver.update(rst);
    }
}

IX_BTNode::IX_BTNode(IX_IndexHandle &saver, RID pos): IX_BTNode(saver.get(pos)) {}

RC IX_BTNode::getSize(int attrLen, int m) {
    return sizeof(int) + RID::getSize() + RID::getSize() + m * IX_BTKEY::getSize(attrLen) + (m+1) * RID::getSize();
    // int is child.size()
}

void IX_BTNode::dump(char* pData, int attrLen, int m) {
    // printf("dump(%d) size %d\n", m, (int)child.size());
    assert(key.size() <= m);
    assert(child.size() == key.size() + 1);
    *reinterpret_cast<int*>(pData) = child.size();
    parent.dumpTo(pData + sizeof(int));
    pos.dumpTo(pData + sizeof(int) + RID::getSize());
    for (int i = 0, start = sizeof(int) + RID::getSize() + RID::getSize(); i < key.size(); i++, start += IX_BTKEY::getSize(attrLen)) {
        key[i].toCharArray(pData + start);
    }
    for (int i=0, start = sizeof(int) + RID::getSize() + RID::getSize() + m * IX_BTKEY::getSize(attrLen); i < child.size(); i++, start += RID::getSize()) {
        child[i].dumpTo(pData + start);
    }
}

void IX_BTNode::load(char* pData, int attrLen, int m) {
    int n = *reinterpret_cast<int*>(pData);
    // printf("load(%d) size %d\n", m, n);
    assert(n <= m+1);
    parent.loadFrom(pData + sizeof(int));
    pos.loadFrom(pData + sizeof(int) + RID::getSize());
    key.clear(); child.clear();
    for (int i = 0, start = sizeof(int) + RID::getSize() + RID::getSize(); i < n-1; i++, start += IX_BTKEY::getSize(attrLen)) {
        key.push_back(IX_BTKEY(pData + start, attrLen));
    }
    for (int i=0, start = sizeof(int) + RID::getSize() + RID::getSize() + m * IX_BTKEY::getSize(attrLen); i < n; i++, start += RID::getSize()) {
        RID rid(pData + start);
        child.push_back(RID());
        *--child.end() = rid;
    }
    // printf("load(%d) size %d: %d %d\n", m, n, (int)key.size(), (int)child.size());
}

void IX_BTNode::outit() {
    printf("(%lld %d): %d %d\n", this->pos.GetPageNum(), this->pos.GetSlotNum(), this->key.size(), this->child.size());
    for (auto key: this->key) {
        printf("%d ", *reinterpret_cast<const int*>(key.attr.c_str()));
    }
    printf("\n");
    for (auto child: this->child) {
        if (child.isValid())
            printf("(%lld %d) ", child.GetPageNum(), child.GetSlotNum());
        else
            printf("(x)");
    }
    printf("\n");
}

IX_BTree::IX_BTree(IX_IndexHandle& saver): _order(saver.getHeader().btm), _root(saver.loadRoot().pos), _hot(), saver(saver) {
    printf("BTree info %d %d %d\n", _order, _root.GetPageNum(), _root.GetSlotNum());
}

RC IX_BTree::search(IX_BTKEY& e, RID& ret) {
    // printf("start search\n");
    ret = RID();
    RID cur = _root;
    while (cur.isValid()) {
        // printf("cur %lld %d\n", cur.GetPageNum(), cur.GetSlotNum());
        IX_BTNode v = saver.get(cur);
        int r = IX_BTKEY::search(v.key, e);
        if (0 <= r && e == v.key[r]) {
            ret = v.pos;
            return IX_ENTRYEXISTS;
        }
        _hot = v; cur = v.child[r+1];
    }
    return IX_KEYNOTFOUND;
}

RC IX_BTree::insert(IX_BTKEY& e) {
    // printf("[[[[[[insert]]]]]] %d %lld %d\n", *reinterpret_cast<const int*>(e.attr.c_str()), e.rid.GetPageNum(), e.rid.GetSlotNum());
    RID v;
    RC rc = search(e, v);
    if (rc == IX_ENTRYEXISTS) {
        return IX_ENTRYEXISTS;
    }
    if (rc != IX_KEYNOTFOUND) {
        IXRC(rc, IX_BTREE);
    }
    assert(_hot.pos.isValid());
    // printf("%d not found, start insert\n", *e.attr.c_str());
    int r = IX_BTKEY::search(_hot.key, e);
    _hot.key.insert(_hot.key.begin() + r + 1, e);
    _hot.child.insert(_hot.child.begin() + r + 2, RID());
    saver.update(_hot);
    // printf("insert end, try overflow\n");
    solveOverflow(_hot);
    return OK_RC;
}

RC IX_BTree::remove(IX_BTKEY& e) {
    // printf("[[[[[[remove]]]]]] %d %lld %d\n", *reinterpret_cast<const int*>(e.attr.c_str()), e.rid.GetPageNum(), e.rid.GetSlotNum());
    RID vid;
    RC rc = search(e, vid);
    if (rc == IX_KEYNOTFOUND) {
        return IX_KEYNOTFOUND;
    }
    if (rc != IX_ENTRYEXISTS) {
        IXRC(rc, IX_BTREE);
    }
    // printf("found entry, hot (%lld %d) v (%lld %d) start delete\n", _hot.pos.GetPageNum(), _hot.pos.GetSlotNum(), vid.GetPageNum(), vid.GetSlotNum());
    assert(vid.isValid());
    assert(_hot.pos.isValid()); // hot is not used in remove
    IX_BTNode v = saver.get(vid);
    int r = IX_BTKEY::search(v.key, e);
    if (v.child[0].isValid()) { // not leaf
        IX_BTNode u = saver.get(v.child[r+1]);
        while (u.child[0].isValid()) {
            u = saver.get(u.child[0]);
        }
        v.key[r] = u.key[0];
        saver.update(v);
        v = u; r = 0;
    }
    v.key.erase(v.key.begin() + r); v.child.erase(v.child.begin() + r + 1);
    // _size--;
    saver.update(v);
    // printf("remove end, try underflow\n");
    solveUnderflow(v);
    return OK_RC;
}

void IX_BTree::solveOverflow(IX_BTNode& v) {
    if (_order >= v.child.size())
        return;
    // printf("solve overflow\n");
    int s = _order >> 1;
    IX_BTNode u(saver);
    u.child[0] = v.child[s+1];
    // SOS push_back and pop_back
    // TODO LOOP is ugly, use range_insert instead
    for (int j=1; j<_order-s; j++) {
        u.child.push_back(v.child[s+j+1]);
        u.key.push_back(v.key[s+j]);
    }
    for (int j=1; j<_order-s; j++) {
        v.child.pop_back();
        v.key.pop_back();
    }
    v.child.pop_back();

    if(u.child[0].isValid()) {
        for (RID pos:u.child) {
            IX_BTNode x = saver.get(pos);
            x.parent = u.pos;
            saver.update(x);
        }
    }

    IX_BTNode p;
    if (v.parent.isValid())
        p = saver.get(v.parent);
    else
        p = IX_BTNode(saver);
        
    if (!v.parent.isValid()) {
        _root = p.pos;
        p.child[0] = v.pos;
        v.parent = p.pos; 
        saver.setRoot(_root);
    }

    int r = IX_BTKEY::search(p.key, v.key[0]) + 1;
    p.key.insert(p.key.begin() + r, v.key[s]);
    v.key.erase(v.key.begin() + s);
    p.child.insert(p.child.begin() + r + 1, u.pos);
    u.parent = p.pos;

    saver.update(u);
    saver.update(v);
    saver.update(p);
    solveOverflow(p);
    // printf("solve overflow end\n");
}

void IX_BTree::solveUnderflow(IX_BTNode& v) {
    if ((_order+1) / 2 <= v.child.size())
        return;
    // printf("solve underflow\n");
    IX_BTNode p = v.parent.isValid() ? saver.get(v.parent) : IX_BTNode(saver);
    if (!v.parent.isValid()) {
        if (!v.key.size() && v.child[0].isValid()) {
            _root = v.child[0];
            IX_BTNode newRoot = saver.get(_root);
            newRoot.parent = RID();
            saver.deleteNode(v);
            saver.update(newRoot);
            saver.setRoot(_root);
        }
        // printf("solve underflow end, root\n");
        return;
    }
    int r = 0;
    while (!(p.child[r] == v.pos)) {
        r++;
    }
    // printf("r %d\n", r);
    if (0 < r) {
        IX_BTNode ls = saver.get(p.child[r-1]);
        if ((_order + 1)/2 < ls.child.size()) {
            v.key.insert(v.key.begin(), p.key[r-1]);
            p.key[r-1] = *--ls.key.end();
            ls.key.pop_back();
            v.child.insert(v.child.begin(), *--ls.child.end());
            ls.child.pop_back();
            if (v.child[0].isValid()) {
                IX_BTNode s = saver.get(v.child[0]);
                s.parent = v.pos;
                saver.update(s);
            }
            saver.update(p);
            saver.update(v);
            saver.update(ls);
            // printf("solve underflow finish, left\n");
            return;
        }
    }
    if (p.child.size() > r + 1) {
        IX_BTNode rs = saver.get(p.child[r+1]);
        if ((_order + 1)/2 < rs.child.size()) {
            v.key.push_back(p.key[r]);
            p.key[r] = rs.key[0];
            rs.key.erase(rs.key.begin());
            v.child.push_back(rs.child[0]);
            rs.child.erase(rs.child.begin());
            if ((*--v.child.end()).isValid()) {
                IX_BTNode s = saver.get(*--v.child.end());
                s.parent = v.pos;
                saver.update(s);
            }
            saver.update(p);
            saver.update(v);
            saver.update(rs);
            // printf("solve underflow finish, right\n");
            return;
        }
    }
    if (0 < r) {
        IX_BTNode ls = saver.get(p.child[r-1]);
        ls.key.push_back(p.key[r-1]);
        p.key.erase(p.key.begin() + r - 1);
        p.child.erase(p.child.begin() + r);
        ls.child.push_back(v.child[0]);
        v.child.erase(v.child.begin());
        if ((*--ls.child.end()).isValid()) {
            IX_BTNode x = saver.get(*--ls.child.end());
            x.parent = ls.pos;
            saver.update(x);
        }
        ls.key.insert(ls.key.end(), v.key.begin(), v.key.end());
        ls.child.insert(ls.child.end(), v.child.begin(), v.child.end());
        for (RID pos: v.child) {
            if (pos.isValid()) {
                IX_BTNode x = saver.get(pos);
                x.parent = ls.pos;
                saver.update(x);
            }
        }
        saver.update(p);
        saver.deleteNode(v);
        saver.update(ls);
    } else {
        IX_BTNode rs = saver.get(p.child[r+1]);
        rs.key.insert(rs.key.begin(), p.key[r]);
        p.key.erase(p.key.begin() + r);
        p.child.erase(p.child.begin() + r);
        rs.child.insert(rs.child.begin(), *--v.child.end());
        v.child.pop_back();
        if (rs.child[0].isValid()) {
            IX_BTNode x = saver.get(rs.child[0]);
            x.parent = rs.pos;
            saver.update(x);
        }
        rs.key.insert(rs.key.begin(), v.key.begin(), v.key.end());
        rs.child.insert(rs.child.begin(), v.child.begin(), v.child.end());
        for (RID pos: v.child) {
            if (pos.isValid()) {
                IX_BTNode x = saver.get(pos);
                x.parent = rs.pos;
                saver.update(x);
            }
        }
        saver.update(p);
        saver.deleteNode(v);
        saver.update(rs);
    }
    solveUnderflow(p);
    // printf("solve underflow end\n");
}