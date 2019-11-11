#pragma once

#include "../rm/rm_rid.h"
#include "../redbase.h"
#include "ix_error.h"
#include <string>
#include <vector>
#include <cstring>

const int IX_NODE_SIZE = 3;

struct IX_BTKEY {
    RID rid;
    AttrType ty;
    std::string attr;

    IX_BTKEY(char* pData, int attrLen) { // from saver
        rid = *reinterpret_cast<RID*>(pData);
        ty = *reinterpret_cast<AttrType*>(pData + sizeof(RID));
        attr = std::string(pData + sizeof(RID) + sizeof(AttrType), attrLen);
    }

    IX_BTKEY(char* pData, int attrLen, AttrType ty): rid(), ty(ty), attr(pData, attrLen) {}

    void toCharArray(char* pData) {
        *reinterpret_cast<RID*>(pData) = rid;
        *reinterpret_cast<AttrType*>(pData + sizeof(RID)) = ty;
        memcpy(pData + sizeof(RID) + sizeof(AttrType), attr.c_str(), attr.length());
    }

    int cmp(const IX_BTKEY &that) const {
        assert(ty == that.ty);
        assert(attr.length() == that.attr.length());
        switch (ty) {
            case INT: {
                int l = stoi(attr), r = stoi(that.attr);
                if (l != r)
                    return l < r ? -1 : 1;
            }
            case FLOAT: {
                char l[attr.length()], r[that.attr.length()];
                memcpy(l, attr.c_str(), attr.length());
                memcpy(r, that.attr.c_str(), that.attr.length());

                float fl = *reinterpret_cast<float*>(l), fr = *reinterpret_cast<float*>(r);
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
        if (rid < that.rid)
            return -1;
        if (rid == that.rid)
            return 0;
        return 1;
    }

    bool operator < (const IX_BTKEY& that) {
        return cmp(that) == -1;
    }

    bool operator == (const IX_BTKEY& that) {
        return cmp(that) == 0;
    }

    // static bool check(const IX_BTKEY &a, CompOp op, const IX_BTKEY &b) {

    // }

    static int getSize(int attrLen) {
        return sizeof(RID) + sizeof(AttrType) + attrLen;
    }

    static int search(const std::vector<IX_BTKEY> vec, IX_BTKEY e) {
        for (int i=0; i<vec.size(); i++)
            if (e < vec[i])
                return i-1;
        return vec.size() - 1;
    }
};

// warning: not do error check
template<typename T, typename Saver>
struct IX_BTNode_T {
    RID parent;
    RID pos;
    std::vector<T> key;
    std::vector<RID> child;

    IX_BTNode_T(): parent(), pos(){
        child.push_back(RID());
    }

    IX_BTNode_T(Saver &saver): IX_BTNode_T(){
        pos = saver.newNode(*this);
    }

    IX_BTNode_T(Saver &saver, T e, RID lc = RID(), RID rc = RID()): parent(), pos(){
        key.push_back(e);
        child.push_back(lc);
        child.push_back(rc);
        pos = saver.newNode(*this);
        if (lc.isValid()) {
            IX_BTNode_T lst = saver.get(lc);
            lst.parent = pos;
            saver.update(lst);
        }
        if (rc.isValid()) {
            IX_BTNode_T rst = saver.get(rc);
            rst.parent = pos;
            saver.update(rst);
        }
    }

    IX_BTNode_T(Saver &saver, RID pos): IX_BTNode_T(saver.get(pos)) {}

    static RC getSize(int attrLen, int m) {
        return sizeof(int) + sizeof(RID) + sizeof(RID) + (m-1) * attrLen + m * sizeof(RID);
        // int is child.size()
    }

    void dump(char* pData, int attrLen, int m) {
        *reinterpret_cast<int*>(pData) = child.size();
        *reinterpret_cast<RID*>(pData + sizeof(int)) = parent;
        for (int i = 0, start = sizeof(int) + sizeof(RID); i < key.size(); i++, start += T::getSize(attrLen)) {
            key[i].toCharArray(pData + start);
        }
        for (int i=0, start = sizeof(int) + sizeof(RID) + (m-1) * T::getSize(attrLen); i < child.size(); i++, start += sizeof(RID)) {
            *reinterpret_cast<RID*>(pData + start) = child[i];
        }
    }

    void load(char* pData, int attrLen, int m) {
        int n = *reinterpret_cast<int*>(pData);
        parent = *reinterpret_cast<RID*>(pData + sizeof(int));
        for (int i = 0, start = sizeof(int) + sizeof(RID); i < m-1; i++, start += T::getSize(attrLen)) {
            key.push_back(T(pData + start, attrLen));
        }
        for (int i=0, start = sizeof(int) + sizeof(RID) + (m-1) * T::getSize(attrLen); i < m; i++, start += sizeof(RID)) {
            child.push_back(*reinterpret_cast<RID*>(pData + start));
        }
    }

    // static void updateNodeParent(char *pData, RID parent) {
    //     *reinterpret_cast<RID*>(pData + sizeof(int)) = parent;
    // }
};

template<typename T, typename Node, typename Saver> // T is IX_BTKEY, Node is IX_BTNode, Saver is ix_indexhandle
class IX_BTree_T{
protected:
    int _order;
    RID _root;
    Node _hot;
    Saver& saver;
public:
    IX_BTree_T(Saver& saver): _order(saver.header.btm), _root(saver.loadRoot().rid), _hot(), saver(saver) {}

    RC search(T& e, RID& ret) {
        ret = RID();
        Node v = saver.get(_root);
        _hot.pos = RID();
        while (v.pos.isValid()) {
            int r = T::search(v.key, e);
            if (0 <= r && e == v.key[r]) {
                ret = v.pos;
                return IX_ENTRYEXISTS;
            }
            _hot = v; v = saver.get(v.child[r+1]);
        }
        return IX_KEYNOTFOUND;
    }

    RC insert(T& e) {
        RID v;
        RC rc = search(e, v);
        if (rc == IX_ENTRYEXISTS) {
            return IX_ENTRYEXISTS;
        }
        if (rc != IX_KEYNOTFOUND) {
            IXRC(rc, IX_BTREE);
        }
        assert(_hot.pos.isValid());
        int r = T::search(_hot.key, e);
        _hot.key.insert(_hot.key.begin() + r + 1, e);
        _hot.child.insert(_hot.child.begin() + r + 2, RID());
        // _size ++;
        saver.update(_hot);
        solveOverflow(_hot);
    }

    RC remove(T& e) {
        RID vid;
        RC rc = search(e, vid);
        if (rc == IX_KEYNOTFOUND) {
            return IX_KEYNOTFOUND;
        }
        if (rc != IX_ENTRYEXISTS) {
            IXRC(rc, IX_BTREE);
        }
        assert(vid.isValid());
        assert(_hot.pos.isValid());
        Node v = saver.get(vid);
        int r = T::search(_hot.key, e) + 1;
        if (v.child[0].isValid()) { // not leaf
            Node u = saver.get(v.child[r+1]);
            while (u.child[0].isValid()) {
                u = saver.get(v.child[0]);
            }
            v.key[r] = u.key[0]; v = u; r = 0;
        }
        v.key.erase(v.key.begin() + r); v.child.erase(v.child.begin() + r + 1);
        // _size--;
        saver.update(v);
        solveUnderflow(v);
        return OK_RC;
    }
private:
    void solveOverflow(Node& v) {
        if (_order >= v.child.size())
            return;
        int s = _order >> 1;
        Node u(saver);
        u.child[0] = v.child[s];
        // SOS push_back and pop_back
        // TODO LOOP is ugly, use range_insert instead
        for (int j=1; j<_order-s; j++) {
            u.child.push_back(v.child[s+j]);
            u.key.push_back(v.key[s+j]);
        }
        for (int j=1; j<_order-s; j++) {
            v.child.pop_back();
            v.key.pop_back();
        }
        v.child.pop_back();

        if(u.child[0].isValid()) {
            for (RID pos:u.child) {
                Node x = saver.get(pos);
                x.parent = u.pos;
                saver.update(x);
            }
        }

        Node p = v.parent.isValid() ? saver.get(v.parent) : Node(saver);
        if (!v.parent.isValid()) {
            p.child.push_back(v.pos);
            v.parent = p.pos; 
        }

        int r = T::search(p.key, v.key[0]) + 1;
        p.key.insert(p.key.begin() + r, v.key[s]);
        v.key.erase(v.key.begin() + s);
        p.child.insert(p.child.begin() + r + 1, u.pos);
        u.parent = p.pos;

        saver.update(u);
        saver.update(v);
        saver.update(p);
        solveOverflow(p);
    }

    void solveUnderflow(Node& v) {
        if ((_order+1) / 2 <= v.child.size())
            return;
        Node p = v.parent.isValid() ? saver.get(v.parent) : Node(saver);
        if (!v.parent.isValid()) {
            // TODO
        }
        int r = 0;
        while (!(p.child[r] == v.pos))
            r++;
        if (0 < r) {
            Node ls = saver.get(p.child[r-1]);
            if ((_order + 1)/2 < ls.child.size()) {
                v.key.insert(v.key.begin(), p.key[r-1]);
                p.key[r-1] = *--ls.key.end();
                ls.key.pop_back();
                v.child.insert(v.child.begin(), *--ls.child.end());
                ls.child.pop_back();
                if (v.child[0].isValid()) {
                    Node s = saver.get(v.child[0]);
                    s.parent = v.pos;
                    saver.update(s);
                }
                saver.update(p);
                saver.update(v);
                saver.update(ls);
                return;
            }
        }
        if (p.child.size() - 1 > r) {
            // SOS NOT FLIP NOW
            Node ls = saver.get(p.child[r-1]);
            if ((_order + 1)/2 < ls.child.size()) {
                v.key.insert(v.key.begin(), p.key[r-1]);
                p.key[r-1] = *--ls.key.end();
                ls.key.pop_back();
                v.child.insert(v.child.begin(), *--ls.child.end());
                ls.child.pop_back();
                if (v.child[0].isValid()) {
                    Node s = saver.get(v.child[0]);
                    s.parent = v.pos;
                    saver.update(s);
                }
                saver.update(p);
                saver.update(v);
                saver.update(ls);
                return;
            }
        }
        if (0 < r) {
            Node ls = saver.get(p.child[r-1]);
            ls.key.push_back(*--p.key.end());
            p.key.pop_back();
            p.child.erase(p.child.begin() + r);
            ls.child.push_back(v.child[0]);
            v.child.erase(v.child.begin());
            if ((*--ls.child.end()).isValid()) {
                Node x = saver.get(*--ls.child.end());
                x.parent = ls.pos;
                saver.update(x);
            }
            ls.key.insert(ls.key.end(), v.key.begin(), v.key.end());
            ls.child.insert(ls.child.end(), v.child.begin(), --v.child.end());
            for (RID pos: v.child) {
                if (pos.isValid()) {
                    Node x = saver.get(pos);
                    x.parent = ls.pos;
                    saver.update(x);
                }
            }
            saver.update(p);
            saver.update(v);
            saver.update(ls);
        } else {
            // SOS NOT FLIP
            Node ls = saver.get(p.child[r-1]);
            ls.key.push_back(*--p.key.end());
            p.key.pop_back();
            p.child.erase(p.child.begin() + r);
            ls.child.push_back(v.child[0]);
            v.child.erase(v.child.begin());
            if ((*--ls.child.end()).isValid()) {
                Node x = saver.get(*--ls.child.end());
                x.parent = ls.pos;
                saver.update(x);
            }
            ls.key.insert(ls.key.end(), v.key.begin(), v.key.end());
            ls.child.insert(ls.child.end(), v.child.begin(), --v.child.end());
            for (RID pos: v.child) {
                if (pos.isValid()) {
                    Node x = saver.get(pos);
                    x.parent = ls.pos;
                    saver.update(x);
                }
            }
            saver.update(p);
            saver.update(v);
            saver.update(ls);
        }
        solveUnderflow(p);
    }
};