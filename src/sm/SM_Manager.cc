#include "sm.h"

SM_Manager::SM_Manager() : rmm(RM_Manager::instance()), ixm(IX_Manager::instance()) {

}


RC SM_Manager::CreateDb(const std::string& dbName) {
    char dir[1000]; 
    if (curTable != "") {
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
    if (curTable != "") {
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
    if (curTable != "") {
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
    if (curTable == "")
        system("ls");
    else
        system("ls ..");
    return OK_RC;
}