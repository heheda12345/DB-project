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

    AttrInfo(const AttrType& _type, unsigned short _mxLen, const std::string& _attrName, bool notNull, bool hasDefault, bool canChange, const std::string& _dVal);

    AttrInfo(): type(NO_TYPE), flag(0), mxLen(0) {}

    void setNotNullFlag(bool b);
    bool isNotNull() const;

    void setDefaultFlag(bool b);
    bool hasDefault() const;

    void setCanChangeFlag(bool b);
    bool canChange() const;

    int load(const char* pData);
    int dump(char* pData) const;
    int getSize() const;

    static std::vector<AttrInfo> loadAttrs(const char* pData);
    static int dumpAttrs(char* pData, const std::vector<AttrInfo>& vec);
    static int getAttrsSize(const std::vector<AttrInfo>& vec);
    
    int getMaxLen() const;
    
    static int getRecordSize(const std::vector<AttrInfo>& attrs);
    static int getPos(const std::vector<AttrInfo>& attrs, std::string attrName);

    static std::vector<int> getAllMxLen(const std::vector<AttrInfo>& attrs);
    static std::vector<AttrType> getAllType(const std::vector<AttrInfo>& attrs);

    static std::vector<int> mapMxLen(const std::vector<AttrInfo>& attrs, const std::vector<std::string>& attrNames);
    static std::vector<AttrType> mapType(const std::vector<AttrInfo>& attrs, const std::vector<std::string>& attrNames);
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
    static int getPos(const std::vector<ForeignKeyInfo> &fKeys, const std::string& fkName);
    static bool isForeignKey(const std::vector<ForeignKeyInfo> &fKeys, const std::string& attrName);
};

std::ostream& operator << (std::ostream& os, const ForeignKeyInfo& fKey);

struct IndexInfo {
    std::string idxName;
    int idxID;
    std::vector<std::string> attrs;

    int load(const char* pData);
    int dump(char* pData) const;
    int getSize() const;

    static std::vector<IndexInfo> loadIndexes(const char* pData);
    static int dumpIndexes(char* pData, const std::vector<IndexInfo>& vec);
    static int getIndexesSize(const std::vector<IndexInfo>& vec);
    static int getPos(const std::vector<IndexInfo> &idxKeys, const std::string& idxName);
    static int getNextId(const std::vector<IndexInfo> &idxKeys);
};

std::ostream& operator << (std::ostream& os, const IndexInfo& fKey);

struct TableInfo {
    std::vector<AttrInfo> attrs;
    std::vector<std::string> primaryKeys;
    std::vector<ForeignKeyInfo> foreignGroups;
    std::vector<ForeignKeyInfo> linkedBy;
    std::vector<IndexInfo> uniqueGroups;
    std::vector<IndexInfo> indexes;
    int load(const char* pData);
    int dump(char* pData) const;
    int getSize() const;

    bool hasPrimary() const {
        return primaryKeys.size() != 0;
    }

    bool linkedByOthers() const {
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

    RC AddForeignKey(const std::string& tbName, const ForeignKeyInfo& fKey);
    RC DropForeignKey(const std::string& tbName, const std::string& fkName);

    RC AddUniqueKey(const std::string& tbName, const std::string& pkName, const std::vector<std::string>& pKeys);
    RC DropUniqueKey(const std::string& tbName, const std::string& pkName);

    RC AddAttr(const std::string& tbName, const AttrInfo& attr);
    RC DropAttr(const std::string& tbName, const std::string& attrName);
    RC ChangeAttr(const std::string& tbName, const std::string& attrName, const AttrInfo& newAttr);

    RC CreateIndex(const std::string& tbName, const std::string& idxName, const std::vector<std::string>& attrNames);
    RC DropIndex(const std::string& tbName, const std::string& idxName);


    RC GetTable(const std::string& relName, TableInfo& table);
    RC UpdateTable(const std::string& tbName, const TableInfo& table);
    RC ShuffleForeign(const std::string& srcTbName, ForeignKeyInfo &key, const std::vector<std::string>& refAttrs);
    RC ShuffleForeign(const TableInfo& srcTable, ForeignKeyInfo &key, const std::vector<std::string>& refAttrs);
    RC LinkForeign(const std::string& reqTb, const ForeignKeyInfo &key);
    RC DropForeignLink(const std::string& refTb, const std::string& fkName);

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
#define SM_FOREIGN_NOT_MATCH    (START_SM_ERR - 8)
#define SM_DUMPLICATED          (START_SM_ERR - 9)
#define SM_NO_SUCH_KEY          (START_SM_ERR - 10)
#define SM_IS_PRIMARY           (START_SM_ERR - 11)
#define SM_IS_FOREIGN           (START_SM_ERR - 12)
#define SM_REQUIRE_NOT_NULL     (START_SM_ERR - 13)
#define SM_LASTERROR            SM_ERROR



#endif