//
// ql.h
//   Query Language Component Interface
//

// This file only gives the stub for the QL component

#ifndef QL_H
#define QL_H

#include <stdlib.h>
#include <string.h>
#include "../redbase.h"
#include "../rm/rm.h"
#include "../ix/ix.h"
#include "../sm/sm.h"

//
// QL_Manager: query language (DML)
//
class QL_Manager {
public:
    QL_Manager (SM_Manager &smm, IX_Manager &ixm, RM_Manager &rmm);
    ~QL_Manager();                       // Destructor

    // RC Select  (int nSelAttrs,           // # attrs in select clause
    //     const RelAttr selAttrs[],        // attrs in select clause
    //     int   nRelations,                // # relations in from clause
    //     const char * const relations[],  // relations in from clause
    //     int   nConditions,               // # conditions in where clause
    //     const Condition conditions[]);   // conditions in where clause

    // RC Insert  (const char *relName,     // relation to insert into
    //     int   nValues,                   // # values
    //     const Value values[]);           // values to insert

    // RC Delete  (const char *relName,     // relation to delete from
    //     int   nConditions,               // # conditions in where clause
    //     const Condition conditions[]);   // conditions in where clause

    // RC Update  (const char *relName,     // relation to update
    //     const RelAttr &updAttr,          // attribute to update
    //     const int bIsValue,              // 1 if RHS is a value, 0 if attribute
    //     const RelAttr &rhsRelAttr,       // attr on RHS to set LHS equal to
    //     const Value &rhsValue,           // or value to set attr equal to
    //     int   nConditions,               // # conditions in where clause
    //     const Condition conditions[]);   // conditions in where clause

    bool CanAddPrimaryKey(const std::string& tbName, const std::vector<std::string>& attrNames) { return true; }
    bool CanAddForeignKey() { return true; }
    bool CanChangeCol() { return true; }
    bool CanCreateIndex() { return true; }
    bool CanAddUniqueKey() { return true; }

    static QL_Manager& instance() { 
        static QL_Manager ins(SM_Manager::instance(), IX_Manager::instance(), RM_Manager::instance());
        return ins;
    }

private:
};

//
// Print-error function
//
void QL_PrintError(RC rc);

#endif