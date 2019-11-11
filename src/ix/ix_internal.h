#pragma once

#include "ix.h"
#include "ix_btree.h"
#include "ix_error.h"
class IX_IndexHandle;

typedef IX_BTNode_T<IX_BTKEY, IX_IndexHandle> IX_BTNode;
typedef IX_BTree_T<IX_BTKEY, IX_BTNode, IX_IndexHandle> IX_BT;
