//
// ql.h
//   Query Language Component Interface
//

// This file only gives the stub for the QL component

#ifndef QL_H
#define QL_H

#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <map>
#include "../redbase.h"
#include "../rm/rm.h"
#include "../ix/ix.h"
#include "../sm/sm.h"

#define MUST_SUCC assert(rc == OK_RC)

class Item;

typedef std::vector<Item> TableLine;
struct Item {
    std::string value;
    bool isNull;
    AttrType type;
    int dump(char* pData, const AttrInfo& a) const;
    int load(const char* pData, const AttrInfo& a);
    int getSize(const AttrInfo& a) const;
    static int dumpTableLine(char* pData, const TableLine& items, const std::vector<AttrInfo>& as);
    static TableLine loadTableLine(const char* pData, const std::vector<AttrInfo>& as);
    static int getLineSize(const std::vector<AttrInfo>& as);
    static Item NullItem();
};
std::ostream& operator << (std::ostream& os, const Item& item);
std::ostream& operator << (std::ostream& os, const TableLine& items);
RC formatItem(const TableInfo& table, TableLine & items);
RC toTableLine(const TableInfo& table, TableLine &items, const std::vector<std::string>& raws);
std::vector<std::string> formatIndex(const std::vector<AttrInfo>& as,
                                     const std::vector<std::string>& attrNames,
                                     const TableLine& attrValues);
bool allNull(const std::vector<AttrInfo>& as,
             const std::vector<std::string>& attrNames,
             const TableLine& attrValues);
bool hasNull(const std::vector<AttrInfo>& as,
             const std::vector<std::string>& attrNames,
             const TableLine& attrValues);

struct SingleWhere {
    enum Type {
        TY_OP_COL, TY_OP_VALUE, TY_NOT_NULL, TY_IS_NULL, TY_LIKE
    } whereType;
    AttrType ty;
    CompOp op;
    int idx1, idx2;
    std::string val;
    bool satisfy(const TableLine& value) const;
};

std::pair<std::vector<TableLine>, std::vector<RID>> select(const std::vector<TableLine>& values,
                                                           const std::vector<RID>& rids,
                                                           const std::vector<SingleWhere>& conds);

struct RawSingleWhere {
    SingleWhere::Type whereType;
    std::string tbName;
    std::string idx1;
    std::string idx2;
    std::string like;
    std::string value;
    AttrType ty;
    CompOp op;
    bool hasError;
    RC Compile(SingleWhere& where, const std::vector<AttrInfo>& attrs, const std::string& tbName_i) const;
};

RC CompileWheres(std::vector<SingleWhere>& conds,
                 const std::vector<RawSingleWhere> &rawConds,
                 const std::vector<AttrInfo>& attrs,
                 const std::string& tbName_i);
RC CompileWheres(std::map<std::string,
                 std::vector<SingleWhere>>& conds,
                 const std::vector<RawSingleWhere> &rawConds,
                 const std::map<std::string,
                 std::vector<AttrInfo>>& attrs);

struct DualWhere {
    std::string tbName1;
    std::string tbName2;
    int idx1;
    int idx2;
};

struct RawDualWhere {
    std::string tbName1;
    std::string tbName2;
    std::string idx1;
    std::string idx2;
    bool hasError;
    RC compile(DualWhere& where, const std::map<std::string, std::vector<AttrInfo>>& attrs) const;
};
RC CompileDualWheres(std::vector<DualWhere>& conds,
                     const std::vector<RawDualWhere>& rawConds,
                     const std::map<std::string, std::vector<AttrInfo>>& attrs);

typedef std::pair<std::string, std::string> RawTbAttr;

struct ExprNode {
    enum ExprNodeType {
        VALUE, COL, ADD, SUB, MUL, DIV
    };
    Item compute(const TableLine& in) const; // read "in" only in "col" node

    ExprNodeType nodeType;
    AttrType attrType;
    int idx; // for col
    std::string str; // for value
    std::vector<ExprNode> sons; // for binary op
};

struct RawExprNode {
    ExprNode::ExprNodeType nodeType;
    bool hasError;
    AttrType attrType; // value
    std::string str; // leaf
    std::vector<RawExprNode> sons; // binOp, sons.size == 2
    RC Compile(ExprNode& node, const std::vector<AttrInfo>& attrs) const;
};

struct SetJob {
    ExprNode expr;
    int target;
};

std::vector<TableLine> DoSetJobs(const std::vector<TableLine>& values, const std::vector<SetJob>& setJobs);

struct RawSetJob {
    RawExprNode expr;
    std::string target;
};

RC CompileSetJobs(std::vector<SetJob>& setJobs, const std::vector<RawSetJob>& rawJobs, const std::vector<AttrInfo>& attrs);

//
// QL_Manager: query language (DML)
//
class QL_Manager {
public:
    QL_Manager (): smm(SM_Manager::instance()), ixm(IX_Manager::instance()), rmm(RM_Manager::instance()) {}
    ~QL_Manager() = default;

    RC Insert(const std::string& tbName, const TableLine& values_i);
    RC Insert(const std::string& tbName, const std::vector<std::string>& rawValues);

    RC Delete(const std::string& tbName, const std::vector<RawSingleWhere>& rawConds);
    
    RC Update(const std::string& tbName, const std::vector<RawSetJob> &rawJobs, const std::vector<RawSingleWhere>& rawConds);
    
    RC Select(const std::vector<std::string>& tbNames,
                    std::vector<RawTbAttr>& rawSelectors, 
              const std::vector<RawSingleWhere>& singleConds,
              const std::vector<RawDualWhere>& rawDualConds,
                    GatherOp gOp);
    
    RC Desc(const std::string& tbName);


    RC GetAllItems(const std::string& tbName,
                   std::vector<TableLine>& values,
                   std::vector<RID>& rids);

    void PrintTable(const std::vector<AttrInfo>& attrs,
                    const std::vector<TableLine>& values);

    std::vector<RID> SearchIndex(const std::string& tbName,
                                 const std::string& idxName,
                                 const std::vector<std::string> & values,
                                 CompOp compOp);
                                 
    bool ExistInIndex(const std::string& tbName,
                      const TableInfo& table,
                      const std::string& idxName,
                      const TableLine& value); // for unique index

    bool ExistInIndex(const TableInfo& table,
                      const std::string& idxName,
                      const TableLine & value,
                      const std::string& refTbName,
                      const std::string& refIdxName); // for foreign, allow null

    bool CanUpdate(const std::string &tbName,
                   const TableInfo& table,
                   const std::string& idxName,
                   const std::vector<TableLine>& toUpdate,
                   const std::vector<RID>& rids);

    void Join(std::vector<AttrInfo>& joinedInfo,
              std::vector<TableLine>& joinedValue,
              const std::map<std::string, std::vector<TableLine>>& values,
              const std::map<std::string, std::vector<AttrInfo>>& attrss,
              const std::vector<DualWhere>& dualConds,
              const std::vector<RawTbAttr>& toShow);

    bool TableIsEmpty(const std::string& tbName);
    bool IsUnique(const std::string& tbName,
                  const std::vector<std::string>& attrNames);
    
    bool CanAddPrimaryKey(const std::string& tbName,
                          const std::vector<std::string>& attrNames);
    bool CanAddForeignKey(const std::string& tbName,
                          const ForeignKeyInfo& fKey);
    bool CanChangeCol(const std::string& tbName);
    bool CanCreateIndex(const std::string& tbName);
    bool CanAddUniqueKey(const std::string& tbName,
                         const std::vector<std::string>& attrNames);

    static QL_Manager& instance() { 
        static QL_Manager ins;
        return ins;
    }


private:
    SM_Manager &smm;
    IX_Manager &ixm;
    RM_Manager &rmm;
};

//
// Print-error function
//
void QL_PrintError(RC rc);

#define QLRC(rc, ret_rc) { \
   if (rc != 0) { \
      QL_PrintError(rc); \
      return ret_rc;  \
   } \
}

#define QL_INVAILD_VALUE (START_QL_WARN + 1)
#define QL_INVALID_TABLE (START_QL_WARN + 2)
#define QL_LEN_NOT_MATCH (START_QL_WARN + 3)
#define QL_REQUIRE_NOT_NULL (START_QL_WARN + 4)
#define QL_TYPE_NOT_MATCH (START_QL_WARN + 5)
#define QL_ATTR_TO_LONG (START_QL_WARN + 6)
#define QL_ERROR (START_QL_WARN + 7)
#define QL_DUMPLICATED (START_QL_WARN + 8)
#define QL_NOT_IN_FOREIGN (START_QL_WARN + 9)
#define QL_NO_SUCH_KEY (START_QL_WARN + 10)
#define QL_PRE_ERROR (START_QL_WARN + 11)
#define QL_INVALID_WHERE (START_QL_WARN + 12)
#define QL_NAME_NOT_MATCH (START_QL_WARN + 13)
#define QL_LINKED_BY_OTHERS (START_QL_WARN + 14)
#define QL_NOTHING_IS_FOUND (START_QL_WARN + 15)
#define QL_REGEX_ERROR (START_QL_WARN + 16)
#endif