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
#include "../redbase.h"  // Please don't change these lines
#include "../rm/rm.h"
#include "../ix/ix.h"
#include "../rm/rm_rid.h"
#include "../utils/utils.h"

#define MAX_DB_NAME 255

struct AttrInfo {
    AttrType type;
    unsigned char flag;
    unsigned short mxLen;
    std::string attrName;
    std::string refTable;
    std::string refAttr;
    std::string dVal;

    AttrInfo(const AttrType& _type, unsigned short _mxLen, const std::string& _attrName, bool notNull, bool isPrimary, bool hasDefault, const std::string& _dVal);

    AttrInfo(const AttrType& _type, unsigned short _mxLen, const std::string& _attrName, bool notNull, bool hasDefault, const std::string& _dVal, const std::string &_refTable, const std::string& _refAttr);

    AttrInfo(): type(NO_TYPE), flag(0), mxLen(0) {}

    void setNotNullFlag(bool b);
    bool isNotNull() const;

    void setPrimaryFlag(bool b);
    bool isPrimary() const;

    void setForeignFlag(bool b);
    RC setForeign(const std::string& _refTable, const std::string& _refAttr);
    bool isForeign() const;

    void setDefaultFlag(bool b);
    bool hasDefault() const;

    int load(const char* pData);
    int dump(char* pData) const;

    static std::vector<AttrInfo> loadAttrs(const char* pData);
    static void dumpAttrs(char* pData, const std::vector<AttrInfo>& attrs);
    
    int getMaxLen() const;
    int getAttrSize() const;
    
    static int getAttrsSize(const std::vector<AttrInfo>& attrs) {
        int ret = sizeof(int);
        for (auto& attr: attrs) {
            ret += attr.getAttrSize();
        }
        return ret;
    }
    static int getRecordSize(const std::vector<AttrInfo>& attrs) {
        int ret = 0;
        for (auto& attr: attrs) {
            ret += attr.mxLen;
        }
        return ret;
    }

    static int getIndex(const std::vector<AttrInfo>& attrs, std::string attrName) {
        for (int i=0; i<(int)attrs.size(); i++) {
            if (attrs[i].attrName == attrName)
                return i;
        }
        return -1;
    }
};

std::ostream& operator << (std::ostream& os, const std::vector<AttrInfo>& attrs);

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

    RC CreateTable(const std::string& relName, const std::vector<AttrInfo>& attributes);
    RC DropTable(const std::string& relName);
    RC ShowTable(const std::string& relName);
    RC ShowTables();

    RC GetAttrs(const std::string& relName, std::vector<AttrInfo>& attributes);
    bool ExistAttr(const std::string& relName, const std::string& attrName) { return true; } // SOS
    // RC CreateIndex(const char *relName,           // create an index for
    //                const char *attrName);         //   relName.attrName
    // RC DropTable  (const char *relName);          // destroy a relation

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

    RM_FileHandle relcatFH;
    RM_FileHandle attrcatFH;
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
#define SM_LASTERROR            SM_ERROR



#endif