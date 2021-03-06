#pragma once

#include "../rm/rm_rid.h"
#include "../redbase.h"
#include "ix_error.h"
#include <string>
#include <vector>
#include <cstring>

const int IX_NODE_SIZE = 3;

class IX_IndexHandle;

struct IX_BTKEY {
    RID rid; // provided by users
    std::vector<AttrType> ty;
    std::vector<std::string> attr;

    IX_BTKEY(const char* pData, const std::vector<int>& attrLen);

    IX_BTKEY(const std::vector<std::string>& attr, const std::vector<int>& attrLen, const std::vector<AttrType>& ty, const RID& rid);

    void toCharArray(char* pData);

    int cmp(const IX_BTKEY &that) const;

    bool operator < (const IX_BTKEY& that) {
        assert(this->rid.isValid());
        assert(that.rid.isValid());
        return cmp(that) == -1;
    }

    bool operator == (const IX_BTKEY& that) {
        assert(this->rid.isValid());
        assert(that.rid.isValid());
        return cmp(that) == 0;
    }

    static bool matches(const IX_BTKEY& attr1, const IX_BTKEY& attr2);

    static int getSize(const std::vector<int>& attrLen);

    static int search(const std::vector<IX_BTKEY> vec, IX_BTKEY e);
};

// warning: not do error check in rm level
struct IX_BTNode {
    RID parent;
    RID pos;
    std::vector<IX_BTKEY> key;
    std::vector<RID> child;

    IX_BTNode();
    IX_BTNode(IX_IndexHandle &saver);
    IX_BTNode(IX_IndexHandle &saver, IX_BTKEY e, RID lc = RID(), RID rc = RID());
    IX_BTNode(IX_IndexHandle &saver, RID pos);

    static RC getSize(const std::vector<int>& attrLen, int m);
    void dump(char* pData, const std::vector<int>& attrLen, int m);
    void load(char* pData, const std::vector<int>& attrLen, int m);
    void outit();
};

class IX_BTree{
protected:
    int _order;
    RID _root;
    IX_BTNode _hot;
    IX_IndexHandle& saver;
public:
    IX_BTree(IX_IndexHandle& saver);

    RC search(IX_BTKEY& e, RID& ret);
    RC insert(IX_BTKEY& e);
    RC remove(IX_BTKEY& e);

private:
    void solveOverflow(IX_BTNode& v);
    void solveUnderflow(IX_BTNode& v);
};