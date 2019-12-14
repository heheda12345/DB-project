//
// sm.h
//   Data Manager Component Interface
//

#ifndef SM_H
#define SM_H

// Please do not include any other files than the ones below in this file.

#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>
#include <iostream>
#include "../redbase.h"  // Please don't change these lines
#include "../rm/rm.h"
#include "../ix/ix.h"
#include "../rm/rm_rid.h"
#include "../utils/utils.h"

struct AttrInfo {
    AttrType type;
    unsigned char flag;
    unsigned short mxLen;
    std::string attrName;
    std::string dVal;

    AttrInfo(const AttrType& _type, unsigned short _mxLen, const std::string& _attrName, bool notNull, bool hasDefault, const std::string& _dVal);

    AttrInfo(): type(NO_TYPE), flag(0), mxLen(0) {}

    void setNotNullFlag(bool b);
    bool isNotNull() const;

    void setDefaultFlag(bool b);
    bool hasDefault() const;

    int load(const char* pData);
    int dump(char* pData) const;
    int getSize() const;

    static std::vector<AttrInfo> loadAttrs(const char* pData);
    static int dumpAttrs(char* pData, const std::vector<AttrInfo>& vec);
    static int getAttrsSize(const std::vector<AttrInfo>& vec);
    
    int getMaxLen() const;
    
    static int getRecordSize(const std::vector<AttrInfo>& attrs) {
        int ret = 0;
        for (auto& attr: attrs) {
            ret += attr.mxLen;
        }
        return ret;
    }

    static int getPos(const std::vector<AttrInfo>& attrs, std::string attrName) {
        for (int i=0; i<(int)attrs.size(); i++) {
            if (attrs[i].attrName == attrName)
                return i;
        }
        return -1;
    }
};

std::ostream& operator << (std::ostream& os, const std::vector<AttrInfo>& attrs);

struct ForeignKeyInfo {
    std::string fkName;
    std::string refTable;
    std::vector<std::string> attrs;

    int load(const char* pData);
    int dump(char* pData) const;
    int getSize() const;

    static std::vector<ForeignKeyInfo> loadForeigns(const char* pData);
    static int dumpForeigns(char* pData, const std::vector<ForeignKeyInfo>& vec);
    static int getForeignsSize(const std::vector<ForeignKeyInfo>& vec);

};

struct IndexInfo {
    std::string idxName;
    int idxID;
    std::vector<std::string> attrs;

    int load(const char* pData) { return 0; }
    int dump(char* pData) const { return 0; }
    int getSize() const { return 0; }

    static std::vector<IndexInfo> loadIndexes(const char* pData) { return std::vector<IndexInfo>(); }
    static int dumpIndexes(char* pData, const std::vector<IndexInfo>& vec) { return 0; }
    static int getIndexesSize(const std::vector<IndexInfo>& vec) { return 0; }
};

struct TableInfo {
    std::vector<AttrInfo> attrs;
    std::vector<std::string> primaryKeys;
    std::vector<ForeignKeyInfo> foreignGroups;
    std::vector<ForeignKeyInfo> linkedBy;
    std::vector<IndexInfo> indexes;
    int load(const char* pData);
    int dump(char* pData) const;
    int getSize() const;

    bool hasPrimary() {
        return primaryKeys.size() != 0;
    }

    bool linkedByOthers() {
        return linkedBy.size() != 0;
    }

    void setPrimaryNotNull();

    // static std::vector<TableInfo> loadtables(const char* pData);
    // static int dumpTables(char* pData, const std::vector<TableInfo>& attrs);
    // static int getTablesSize(const std::vector<TableInfo>& attrs);
};

std::ostream& operator << (std::ostream& os, const TableInfo& table);
//
// SM_Manager: provides data management
//
class SM_Manager {
public:
    ~SM_Manager   () = default;                             // Destructor

    RC UseDb      (const std::string& dbName);    // Use the database
    RC CreateDb   (const std::string& dbName);    // Create the database
    RC DropDb     (const std::string& dbName);    // Drop the database
    RC ShowAllDb  ();
    bool usingDb() const;

    RC CreateTable(const std::string& relName, const TableInfo& table);
    RC DropTable(const std::string& relName);
    RC ShowTable(const std::string& relName);
    RC ShowTables();

    RC AddPrimaryKey(const std::string& tbName, const std::vector<std::string>& attrNames);
    RC DropPrimaryKey(const std::string& tbName);

    // RC AddForeignKey(const std::string& reqTb, const std::string& reqAttr, const std::string& dstTb, const std::string& dstAttr);
    // RC DropForeignKey(const std::string& reqTb, const std::string& reqAttr);

    RC GetTable(const std::string& relName, TableInfo& table);
    RC UpdateTable(const std::string& tbName, const TableInfo& table);
    // bool ExistAttr(const std::string& relName, const std::string& attrName, AttrType type = NO_TYPE);
    // bool LinkForeign(const std::string& reqTb, const std::string& reqAttr, const std::string& dstTb, const std::string& dstAttr);
    // RC GetForeignDst(const std::string& reqTb, std::string& reqAttr, std::string& dstTb, std::string& dstAttr);

    // RC CreateIndex(const char *relName,           // create an index for
    //                const char *attrName);         //   relName.attrName

    // RC DropIndex  (const char *relName,           // destroy index on
    //                const char *attrName);         //   relName.attrName
    // RC Load       (const char *relName,           // load relName from
    //                const char *fileName);         //   fileName
    // RC Help       ();                             // Print relations in db
    // RC Help       (const char *relName);          // print schema of relName

    // RC Print      (const char *relName);          // print relName contents

    // RC Set        (const char *paramName,         // set parameter to
    //                const char *value);            //   value

    static SM_Manager& instance() {
        static SM_Manager ins;
        return ins;
    }
private:
    friend class QL_Manager;
    static const int NO_INDEXES = -1;
    static const PageNum INVALID_PAGE = -1;
    static const SlotNum INVALID_SLOT = -1;
    SM_Manager();
    RM_Manager &rmm;
    IX_Manager &ixm;
    std::string curTable = "";
};

//
// Print-error function
//
void SM_PrintError(RC rc);

#define SMRC(rc, ret_rc) { \
   if (rc != 0) { \
      SM_PrintError(rc); \
      return ret_rc;  \
   } \
}

#define SM_CANNOTCLOSE          (START_SM_WARN + 0) // invalid RID
#define SM_BADRELNAME           (START_SM_WARN + 1)
#define SM_BADREL               (START_SM_WARN + 2)
#define SM_BADATTR              (START_SM_WARN + 3)
#define SM_INVALIDATTR          (START_SM_WARN + 4)
#define SM_INDEXEDALREADY       (START_SM_WARN + 5)
#define SM_NOINDEX              (START_SM_WARN + 6)
#define SM_SYS_ERR              (START_SM_WARN + 7)
#define SM_LASTWARN             SM_NOINDEX

#define SM_INVALIDDB            (START_SM_ERR - 0)
#define SM_ERROR                (START_SM_ERR - 1) // error
#define SM_DB_NOT_OPEN          (START_SM_ERR - 2)
#define SM_NO_SUCH_ATTR         (START_SM_ERR - 3)
#define SM_OTHERS_FOREIGN       (START_SM_ERR - 4)
#define SM_NOT_FOREIGN          (START_SM_ERR - 5)
#define SM_HAS_PRIMARY          (START_SM_ERR - 6)
#define SM_NO_PRIMARY           (START_SM_ERR - 7)
#define SM_LASTERROR            SM_ERROR



#endif