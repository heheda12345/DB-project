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
#include "../redbase.h"
#include "../rm/rm.h"
#include "../ix/ix.h"
#include "../sm/sm.h"
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
};
std::ostream& operator << (std::ostream& os, const Item& item);
std::ostream& operator << (std::ostream& os, const TableLine& items);
RC formatItem(const TableInfo& table, TableLine & items);
std::vector<std::string> formatIndex(const std::vector<AttrInfo>& as, const std::vector<std::string>& attrNames, const TableLine& attrValues);

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

std::vector<TableLine> select(const std::vector<TableLine>& values, const std::vector<SingleWhere>& conds);

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

RC CompileWheres(std::vector<SingleWhere>& conds, const std::vector<RawSingleWhere> &rawConds, const std::vector<AttrInfo>& attrs, const std::string& tbName_i);

//
// QL_Manager: query language (DML)
//
class QL_Manager {
public:
    QL_Manager (): smm(SM_Manager::instance()), ixm(IX_Manager::instance()), rmm(RM_Manager::instance()) {}
    ~QL_Manager() = default;

    // RC Select  (int nSelAttrs,           // # attrs in select clause
    //     const RelAttr selAttrs[],        // attrs in select clause
    //     int   nRelations,                // # relations in from clause
    //     const char * const relations[],  // relations in from clause
    //     int   nConditions,               // # conditions in where clause
    //     const Condition conditions[]);   // conditions in where clause

    RC Insert(const std::string& tbName, const TableLine& values_i);
    RC Delete(const std::string& tbName, const std::vector<RawSingleWhere>& rawConds);
    RC Desc(const std::string& tbName);


    RC GetAllItems(const std::string& tbName, std::vector<TableLine>& values);
    void PrintTable(const TableInfo& table, const std::vector<TableLine>& values);
    std::vector<RID> SearchIndex(const std::string& tbName, const std::string& idxName, const std::vector<std::string> & values, CompOp compOp);
    bool ExistInIndex(const std::string& tbName, const TableInfo& table, const std::string& idxName, const TableLine& value); // for index
    bool ExistInIndex(const TableInfo& table, const std::string& idxName, const TableLine & value, const std::string& refTbName, const std::string& refIdxName); // for foreign

    bool CanAddPrimaryKey(const std::string& tbName, const std::vector<std::string>& attrNames) { return true; }
    bool CanAddForeignKey() { return true; }
    bool CanChangeCol() { return true; }
    bool CanCreateIndex() { return true; }
    bool CanAddUniqueKey() { return true; }

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
#endif