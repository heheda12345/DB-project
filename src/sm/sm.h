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

    AttrInfo(const AttrType& _type, unsigned short _mxLen, const std::string& _attrName, bool notNull, bool isPrimary);

    AttrInfo(const AttrType& _type, unsigned short _mxLen, const std::string& _attrName, bool notNull, const std::string &_refTable, const std::string& _refAttr);

    void setNotNull(bool b);
    bool isNotNull() const;

    void setPrimary(bool b);
    bool isPrimary() const;

    void setForeign(bool b);
    bool isForeign() const;

    void load(const char* pData);
    void dump(char* pData);

    static int getSize() {
        return MAXNAME * 3 + sizeof(unsigned char)*2 + sizeof(short);
    }
};

//
// SM_Manager: provides data management
//
class SM_Manager {
    friend class QL_Manager;
    static const int NO_INDEXES = -1;
    static const PageNum INVALID_PAGE = -1;
    static const SlotNum INVALID_SLOT = -1;
public:
    SM_Manager    (IX_Manager &ixm, RM_Manager &rmm);
    ~SM_Manager   ();                             // Destructor

    RC OpenDb     (const char *dbName);           // Open the database
    RC CloseDb    ();                             // close the database

    RC CreateTable(const char *relName,           // create relation relName
                   int        attrCount,          //   number of attributes
                   AttrInfo   *attributes);       //   attribute data
    RC CreateIndex(const char *relName,           // create an index for
                   const char *attrName);         //   relName.attrName
    RC DropTable  (const char *relName);          // destroy a relation

    RC DropIndex  (const char *relName,           // destroy index on
                   const char *attrName);         //   relName.attrName
    RC Load       (const char *relName,           // load relName from
                   const char *fileName);         //   fileName
    RC Help       ();                             // Print relations in db
    RC Help       (const char *relName);          // print schema of relName

    RC Print      (const char *relName);          // print relName contents

    RC Set        (const char *paramName,         // set parameter to
                   const char *value);            //   value

private:
//   bool isValidAttrType(AttrInfo attribute);
//   RC InsertRelCat(const char *relName, int attrCount, int recSize, RID &attrRID);
//   RC InsertAttrCat(const char *relName, AttrInfo attr, int offset, RID &nextRID);
//   RC GetRelEntry(const char *relName, RM_Record &relRec, RelCatEntry *&entry);
//   RC GetAttrEntry(RID &attrRID, RM_Record &attrRec, AttrCatEntry *&entry);

//   RC FindAttr(const char *relName, const char *attrName, RM_Record &attrRec, AttrCatEntry *&entry);
//   RC SetUpPrint(RelCatEntry* rEntry, DataAttrInfo *attributes);
//   RC SetUpRelCatAttributes(DataAttrInfo *attributes);
//   RC SetUpAttrCatAttributes(DataAttrInfo *attributes);

  RM_Manager &rmm;
  IX_Manager &ixm;

  RM_FileHandle relcatFH;
  RM_FileHandle attrcatFH;

  

};

//
// Print-error function
//
void SM_PrintError(RC rc);

#define SM_CANNOTCLOSE          (START_SM_WARN + 0) // invalid RID
#define SM_BADRELNAME           (START_SM_WARN + 1)
#define SM_BADREL               (START_SM_WARN + 2)
#define SM_BADATTR              (START_SM_WARN + 3)
#define SM_INVALIDATTR          (START_SM_WARN + 4)
#define SM_INDEXEDALREADY       (START_SM_WARN + 5)
#define SM_NOINDEX              (START_SM_WARN + 6)
#define SM_LASTWARN             SM_NOINDEX

#define SM_INVALIDDB            (START_SM_ERR - 0)
#define SM_ERROR                (START_SM_ERR - 1) // error
#define SM_LASTERROR            SM_ERROR



#endif