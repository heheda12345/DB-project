#include "tree.h"
#include "../utils/utils.h"
#include <assert.h>
using namespace std;
using namespace Parser;

const int asst = true; // assert(ast) if unimplemented
string TreeNode::curTable = "";

void Parser::ShowDatabases::visit() {
    if (curTable == "")
        system("ls");
    else
        system("ls ..");
    
}

void Parser::CreateDatabase::visit() {
    if (TryMkDir(dbName->c_str())) {
        printf("[Fail] DB %s exists!\n", dbName->c_str());
    } else {
        printf("[Succ] Create db %s\n", dbName->c_str());
    }
}

void Parser::UseDatabase::visit() {
    char dir[1000]; 
    if (curTable != "") {
        sprintf(dir, "../%s", dbName->c_str());
    } else {
        sprintf(dir, "%s", dbName->c_str());
    }
    if (curTable == *dbName) {
        printf("[Fail] Already in %s\n", dbName->c_str());
    } else if (DirExist(dir)) {
        chdir(dir);
        printf("[Succ] Switch to %s\n", dbName->c_str());
        curTable = *dbName;
    } else {
        printf("[Fail] No db named %s\n", dbName->c_str());
    }
}

void Parser::DropDatabase::visit() {
    char dir[1000]; 
    if (curTable != "") {
        sprintf(dir, "../%s", dbName->c_str());
    } else {
        sprintf(dir, "%s", dbName->c_str());
    }
    if (!DirExist(dir)) {
        printf("[Fail] No db named %s\n", dbName->c_str());
    } else {
        if (curTable == *dbName) {
            chdir("..");
            curTable = "";
            sprintf(dir, "%s", dbName->c_str());
            printf("[Info] Using db %s, exit first\n", dbName->c_str());
        }
        RmDir(dir);
        printf("[Succ] Drop db %s\n", dbName->c_str());
    }
    assert(asst);
}

void Parser::ShowTables::visit() {
    printf("show tables");
    assert(asst);
}

void Parser::CreateTable::visit() {
    printf("CreateTable");
    assert(asst);
}

void Parser::DropTable::visit() {
    printf("DropTable");
    assert(asst);
}

void Parser::Desc::visit() {
    printf("DropTable");
    assert(asst);
}


void Parser::DescTable::visit() {
    printf("DescTable");
    assert(asst);
}

void Parser::InsertValue::visit() {
    printf("InsertValue");
    assert(asst);
}

void Parser::DeleteValue::visit() {
    printf("DeleteValue");
    assert(asst);
}

void Parser::UpdateValue::visit() {
    printf("UpdateValue");
    assert(asst);
}

void Parser::SelectValue::visit() {
    printf("SelectValue");
    assert(asst);
}

void Parser::CreateIndex::visit() {
    printf("CreateIndex");
    assert(asst);
}

void Parser::DropIndex::visit() {
    printf("DropIndex");
    assert(asst);
}

void Parser::AddIndex::visit() {
    printf("AddIndex");
    assert(asst);
}

void Parser::AddField::visit() {
    printf("AddField");
    assert(asst);
}

void Parser::DropCol::visit() {
    printf("DropCol");
    assert(asst);
}

void Parser::ChangeCol::visit() {
    printf("ChangeCol");
    assert(asst);
}

void Parser::RenameTable::visit() {
    printf("RenameTable");
    assert(asst);
}

void Parser::DropPrimaryKey::visit() {
    printf("DropPrimaryKey");
    assert(asst);
}