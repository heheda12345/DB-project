#include "sm.h"
#include <iostream>
using namespace std;

SM_Manager::SM_Manager() : rmm(RM_Manager::instance()), ixm(IX_Manager::instance()) {

}

bool SM_Manager::usingDb() const {
    return curTable != "";
}

RC SM_Manager::CreateDb(const std::string& dbName) {
    char dir[1000]; 
    if (usingDb()) {
        sprintf(dir, "../%s", dbName.c_str());
    } else {
        sprintf(dir, "%s", dbName.c_str());
    }
    if (TryMkDir(dir)) {
        printf("[Fail] DB %s exists!\n", dbName.c_str());
    } else {
        printf("[Succ] Create db %s\n", dbName.c_str());
    }
    return OK_RC;
}

RC SM_Manager::UseDb(const std::string& dbName) {
    char dir[1000]; 
    if (usingDb()) {
        sprintf(dir, "../%s", dbName.c_str());
    } else {
        sprintf(dir, "%s", dbName.c_str());
    }
    if (curTable == dbName) {
        printf("[Fail] Already in %s\n", dbName.c_str());
    } else if (DirExist(dir)) {
        chdir(dir);
        printf("[Succ] Switch to %s\n", dbName.c_str());
        curTable = dbName;
    } else {
        printf("[Fail] No db named %s\n", dbName.c_str());
    }
    return OK_RC;
}

RC SM_Manager::DropDb(const std::string& dbName) {
    char dir[1000]; 
    if (usingDb()) {
        sprintf(dir, "../%s", dbName.c_str());
    } else {
        sprintf(dir, "%s", dbName.c_str());
    }
    if (!DirExist(dir)) {
        printf("[Fail] No db named %s\n", dbName.c_str());
    } else {
        if (curTable == dbName) {
            chdir("..");
            curTable = "";
            sprintf(dir, "%s", dbName.c_str());
            printf("[Info] Using db %s, exit first\n", dbName.c_str());
        }
        RmDir(dir);
        printf("[Succ] Drop db %s\n", dbName.c_str());
    }
    return OK_RC;
}

RC SM_Manager::ShowAllDb() {
    if (usingDb())
        system("ls ..");
    else
        system("ls");
    return OK_RC;
}

RC SM_Manager::CreateTable(const std::string& relName, const std::vector<AttrInfo>& attributes) {
    if (!usingDb()) {
        return SM_DB_NOT_OPEN;
    }
    char header[AttrInfo::getAttrsSize(attributes)];
    AttrInfo::dumpAttrs(header, attributes);
    RC rc = rmm.CreateFile(relName.c_str(), AttrInfo::getRecordSize(attributes),
        header, AttrInfo::getAttrsSize(attributes));
    RMRC(rc, SM_ERROR);
    return OK_RC;
}

RC SM_Manager::DropTable(const std::string& relName) {
    if (!usingDb()) {
        return SM_DB_NOT_OPEN;
    }
    RC rc = rmm.DestroyFile(relName.c_str());
    RMRC(rc, SM_ERROR);
    return OK_RC;
}

RC SM_Manager::ShowTable(const std::string& relName) {
    vector<AttrInfo> attrs;
    RC rc = GetAttrs(relName, attrs);
    SMRC(rc, SM_ERROR);
    cout << attrs << endl;
    return OK_RC;
}

RC SM_Manager::ShowTables() {
    if (!usingDb()) {
        return SM_DB_NOT_OPEN;
    }
    std::string dir = usingDb() ? ".." : ".";
    auto tables = getAllTable(dir.c_str());
    for (auto& tb: tables) {
        vector<AttrInfo> attrs;
        RC rc = GetAttrs(tb, attrs);
        SMRC(rc, SM_ERROR);
        assert(rc == OK_RC);
        cout << tb << std::string(": ") << attrs << endl; 
    }
    return OK_RC;
}

RC SM_Manager::GetAttrs(const std::string& relName, std::vector<AttrInfo>& attributes) {
    if (!usingDb()) {
        return SM_DB_NOT_OPEN;
    }
    RM_FileHandle handle;
    RC rc = rmm.OpenFile(relName.c_str(), handle);
    RMRC(rc, SM_ERROR);
    int size;
    rc = handle.GetMetaSize(size);
    RMRC(rc, SM_ERROR);
    char header[size];
    rc = handle.GetMeta(header, size);
    attributes = AttrInfo::loadAttrs(header);
    return OK_RC;
}