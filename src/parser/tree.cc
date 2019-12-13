#include "tree.h"
#include "../utils/utils.h"
#include "../sm/sm.h"
#include <assert.h>
using namespace std;
using namespace Parser;

const int asst = true; // assert(ast) if unimplemented

void Parser::ShowDatabases::visit() {
    SM_Manager::instance().ShowAllDb();
}

void Parser::CreateDatabase::visit() {
    SM_Manager::instance().CreateDb(*dbName);
}

void Parser::UseDatabase::visit() {
    SM_Manager::instance().UseDb(*dbName);
}

void Parser::DropDatabase::visit() {
    SM_Manager::instance().DropDb(*dbName);
}

void Parser::ShowTables::visit() {
    if (!SM_Manager::instance().usingDb()) {
        printf("[Fail] Use a database first!\n");
        return;
    }
    RC rc = SM_Manager::instance().ShowTables();
    if (rc != OK_RC) {
        printf("[Fail] Can not show the tables!\n");
        return;
    }
}

void Parser::CreateTable::visit() {
    if (!SM_Manager::instance().usingDb()) {
        printf("[Fail] Use a database first!\n");
        return;
    }
    if (tbName->length() > MAXNAME) {
        printf("[Fail] Table name %s is too long\n", tbName->c_str());
        return;
    }
    vector<AttrInfo> attrs;
    for (auto& f: *fields) {
        if (f->ty == Field::Simple) {
            assert(f->attr.type != NO_TYPE);
            if (AttrInfo::getIndex(attrs, f->attr.attrName) != -1) {
                printf("[Fail] Attrbute name %s duplicated!\n", f->attr.attrName.c_str());
                return;
            }
            if (f->attr.attrName.length() >= MAXNAME) {
                printf("[Fail] Attribute name %s is too long\n", f->attr.attrName.c_str());
                return;
            }
            attrs.push_back(f->attr);
        }
    }
    vector<string> pCols;
    vector<string> fCols;
    vector<string> refTables;
    vector<string> refAttrs;
    for (Field* f: *fields) {
        if (f->ty == Field::Primary) {
            for (Column* col: *(f->columns)) {
                pCols.push_back(*(col->colName));
            }
        } else if (f->ty == Field::Foreign) {
            fCols.push_back(*(f->colName));
            refTables.push_back(*(f->tbName));
            refAttrs.push_back(*(f->othercol));
        }
    }

    for (auto s: pCols) {
        int idx = AttrInfo::getIndex(attrs, s);
        if (idx == -1) {
            printf("[Fail] No key named %s\n", s.c_str());
            return;
        }
        attrs[idx].setPrimaryFlag(1);
    }

    assert(fCols.size() == refTables.size() && refTables.size() == refAttrs.size());
    for (int i = 0; i < (int)fCols.size(); i++) {
        int idx = AttrInfo::getIndex(attrs, fCols[i]);
        if (idx == -1) {
            printf("[Fail] No key named %s\n", fCols[i].c_str());
            return;
        }
        if (!SM_Manager::instance().ExistAttr(refTables[i], refAttrs[i])) {
            printf("[Fail] No attribute %s.%s", refTables[i].c_str(), refAttrs[i].c_str());
            return;
        }
        if (attrs[idx].isForeign()) {
            printf("[Fail] Attr %s can have at most one foreign key\n", fCols[i]);
            return;
        }
        attrs[idx].setForeign(refTables[i], refAttrs[i]);
    }

    RC rc = SM_Manager::instance().CreateTable(*tbName, attrs);
    if (rc) {
        printf("[Fail] Can not create table %s\n", tbName->c_str());
        return;
    }
    printf("[Succ] Table %s created!\n", tbName->c_str());
}

void Parser::DropTable::visit() {
    if (!SM_Manager::instance().usingDb()) {
        printf("[Fail] Use a database first!\n");
        return;
    }
    RC rc = SM_Manager::instance().DropTable(*tbName);
    if (rc != OK_RC) {
        printf("[Fail] Can not drop table %s!\n", tbName->c_str());
        return;
    }
    printf("[Succ] Table %s dropped!\n", tbName->c_str());
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