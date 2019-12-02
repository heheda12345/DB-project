#include "tree.h"
#include <assert.h>
using namespace std;
using namespace Parser;

const int asst = true; // assert(ast) if unimplemented

void Parser::ShowDatabases::visit() {
    printf("ShowDatabases\n");
    assert(asst);
}

void Parser::CreateDatabase::visit() {
    printf("create database %s\n", dbName->c_str());
    assert(asst);
}

void Parser::UseDatabase::visit() {
    printf("visit database %s\n", dbName->c_str());
    assert(asst);
}

void Parser::DropDatabase::visit() {
    printf("drop database %s\n", dbName->c_str());
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