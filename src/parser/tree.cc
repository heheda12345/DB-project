#include "tree.h"
#include "../utils/utils.h"
#include "../sm/sm.h"
#include "../ql/ql.h"
#include <assert.h>
#include <fstream>
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
    TableInfo table;
    for (auto& f: *fields) {
        if (f->ty == Field::Simple) {
            assert(f->attr.type != NO_TYPE);
            if (AttrInfo::getPos(table.attrs, f->attr.attrName) != -1) {
                printf("[Fail] Attrbute name %s duplicated!\n", f->attr.attrName.c_str());
                return;
            }
            if (f->attr.attrName.length() >= MAXNAME) {
                printf("[Fail] Attribute name %s is too long\n", f->attr.attrName.c_str());
                return;
            }
            if (f->hasError) {
                printf("[Fail] Invalid Attr\n");
                return;
            }
            table.attrs.push_back(f->attr);
        }
    }
    for (Field* f: *fields) {
        if (f->ty == Field::Primary) {
            if (table.primaryKeys.size() != 0) {
                printf("[Fail] At most one primary key!\n");
                return;
            }
            for (Column* col: *(f->columns)) {
                table.primaryKeys.push_back(*(col->colName));
            }
            if (isDumplicated(table.primaryKeys)) {
                printf("[Fail] Dumplicated primary key!\n");
                return;
            }
            for (auto s: table.primaryKeys) {
                int idx = AttrInfo::getPos(table.attrs, s);
                if (idx == -1) {
                    printf("[Fail] No key named %s\n", s.c_str());
                    return;
                }
            }
            table.setPrimaryNotNull();
        }  else if (f->ty == Field::Foreign) {
            ForeignKeyInfo fKey;
            fKey.fkName = std::string("@@").append(std::to_string(table.foreignGroups.size())).append("-").append(*tbName);
            fKey.refTable = *(f->tbName);
            if (fKey.refTable == *tbName) {
                printf("[Fail] Cannot link with self\n");
                return;
            }
            fKey.attrs.push_back(*(f->colName));
            vector<std::string> refAttrs;
            refAttrs.push_back(*(f->othercol));
            RC rc = SM_Manager::instance().ShuffleForeign(table, fKey, refAttrs);
            if (rc != OK_RC) {
                printf("[Fail] Invalid keys\n");
                return;
            }
            table.foreignGroups.push_back(fKey);
        }
    }
    RC rc = SM_Manager::instance().CreateTable(*tbName, table);
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
    if (!SM_Manager::instance().usingDb()) {
        printf("[Fail] Use a database first!\n");
        return;
    }
    QL_Manager::instance().Desc(*tbName);
}


void Parser::DescTable::visit() {
    if (!SM_Manager::instance().usingDb()) {
        printf("[Fail] Use a database first!\n");
        return;
    }
    printf("DescTable");
    assert(asst);
}

void Parser::InsertValue::visit() {
    if (!SM_Manager::instance().usingDb()) {
        printf("[Fail] Use a database first!\n");
        return;
    }
    vector<Item> items;
    for (auto& val: *values) {
        if (val->hasError) {
            printf("[Fail] Invalid value!\n");
            return;
        }
        items.push_back(val->toItem());
    }
    RC rc = QL_Manager::instance().Insert(*tbName, items);
    if (rc != OK_RC) {
        printf("[Fail] Can not insert!\n");
        return;
    } else {
        printf("[Succ] Insert 1 item to table %s!\n", tbName->c_str());
        return;
    }
}

void Parser::DeleteValue::visit() {
    if (!SM_Manager::instance().usingDb()) {
        printf("[Fail] Use a database first!\n");
        return;
    }
    vector<RawSingleWhere> conds;
    for (auto& where: *wheres) {
        if (!where->inSingle) {
            printf("[Fail] One table only!\n");
            return;
        }
        if (where->singleWhere.hasError) {
            printf("[Fail] Invalid clause!\n");
            return;
        }
        conds.push_back(where->singleWhere);
    }
    
    RC rc = QL_Manager::instance().Delete(*tbName, conds);
    if (rc != OK_RC) {
        printf("[Fail] Cannot delete!\n");
        return;
    } else {
        printf("[Succ] Succussfully delete value in %s\n", tbName->c_str());
        return;
    }
}

void Parser::UpdateValue::visit() {
    if (!SM_Manager::instance().usingDb()) {
        printf("[Fail] Use a database first!\n");
        return;
    }

    vector<RawSetJob> jobs;
    for (auto& set: *sets) {
        if (set->node->hasError) {
            printf("[Fail] Invalid set clause!\n");
            return;
        }
        RawSetJob job;
        job.target = *set->colName;
        job.expr = set->node->node;
        jobs.push_back(job);
    }
    
    vector<RawSingleWhere> conds;
    for (auto& where: *wheres) {
        if (!where->inSingle) {
            printf("[Fail] One table only!\n");
            return;
        }
        if (where->singleWhere.hasError) {
            printf("[Fail] Invalid where clause!\n");
            return;
        }
        conds.push_back(where->singleWhere);
    }

    RC rc = QL_Manager::instance().Update(*tbName, jobs, conds);
    if (rc != OK_RC) {
        printf("[Fail] Cannot update\n");
        return;
    } else {
        printf("[Succ] Succussfully update value in %s\n", tbName->c_str());
    }
    assert(asst);
}

void Parser::SelectValue::visit() {
    if (!SM_Manager::instance().usingDb()) {
        printf("[Fail] Use a database first!\n");
        return;
    }

    std::vector<std::string> tbNames;
    for (auto& table: *tables) {
        tbNames.push_back(*table->tbName);
    }
    if (isDumplicated(tbNames)) {
        printf("[Fail] Dumplicated table names\n");
        return;
    }
    
    vector<RawSingleWhere> singleConds;
    vector<RawDualWhere> dualConds;
    for (auto& where: *wheres) {
        if (where->inSingle) {
            if (where->singleWhere.hasError) {
                printf("[Fail] Invalid where clause!\n");
                return;
            }
            if (where->singleWhere.tbName == "") {
                if (tbNames.size() == 1) {
                    where->singleWhere.tbName = tbNames[0];
                } else {
                    printf("[Fail] Please provide table name\n");
                    return;
                }
            }
            singleConds.push_back(where->singleWhere);
        } else {
            if (where->dualWhere.hasError) {
                printf("[Fail] Invalid where clause!\n");
                return;
            }
            dualConds.push_back(where->dualWhere);
        }
    }
    if (selector->gOp != NO_GOP) {
        if (selector->attrNames.size() != 1) {
            printf("[Fail] Provide one and only one selector when gathering\n");
            return;
        }
    }

    if (tbNames.size() == 1) {
        for (auto& attr: selector->attrNames) {
            if (attr.first == "")
                attr.first = tbNames[0];
        }
    }
    RC rc = QL_Manager::instance().Select(tbNames, selector->attrNames, singleConds, dualConds, selector->gOp);
    if (rc != OK_RC) {
        printf("[Fail] Cannot select!\n");
        return;
    }
    printf("[Succ] Select end\n");
}

void Parser::CopyFrom::visit() {
    if (!SM_Manager::instance().usingDb()) {
        printf("[Fail] Use a database first!\n");
        return;
    }

    ifstream is;
    is.open(path->c_str());
    char* buffer = new char[1000010];
    RC rc;
    int cnt = 0;
    while (is.getline(buffer, 1e6)) {
        vector<string> items;
        int l = strlen(buffer);
        int last = 0;
        if (cnt % 100 == 99)
            printf("%s\n", buffer);
        cnt++;
        for (int i = 0; i < l; i++) {
            if (buffer[i] == '|') {
                items.push_back(std::string(buffer + last, i - last));
                last = i + 1;
            }
        }
        // for (auto& st: items) {
        //     printf("%s\n", st.c_str());
        // }
        rc = QL_Manager::instance().Insert(*tbName, items);
        if (rc) {
            printf("[Fail] %s: %s\n", tbName->c_str(), buffer);
        }
    }
    delete[] buffer;
    is.close();

    if (rc != OK_RC) {
        return;
    }
    printf("[Succ] Load Table %s End\n", tbName->c_str());
}


void Parser::AddIndex::visit() {
    if (!SM_Manager::instance().usingDb()) {
        printf("[Fail] Use a database first!\n");
        return;
    }
    vector<string> attrNames;
    for (auto& col: *columns) {
        attrNames.push_back(*(col->colName));
    }
    if (!QL_Manager::instance().CanCreateIndex(*tbName)) {
        printf("[Fail] Invalid data exists\n");
        return;
    }
    RC rc = SM_Manager::instance().CreateIndex(*tbName, *idxName, attrNames);
    if (rc != OK_RC) {
        printf("[Fail] Cannot create index %s.%s\n", tbName->c_str(), idxName->c_str());
        return;
    } else {
        printf("[Succ] Index %s.%s created!\n", tbName->c_str(), idxName->c_str());
        return;
    }
}

void Parser::DropIndex::visit() {
    if (!SM_Manager::instance().usingDb()) {
        printf("[Fail] Use a database first!\n");
        return;
    }
    RC rc = SM_Manager::instance().DropIndex(*tbName, *idxName);
    if (rc != OK_RC) {
        printf("[Fail] Cannot drop index %s.%s\n", tbName->c_str(), idxName->c_str());
        return;
    } else {
        printf("[Succ] Index %s.%s dropped!\n", tbName->c_str(), idxName->c_str());
        return;
    }
}

void Parser::AddField::visit() {
    if (!SM_Manager::instance().usingDb()) {
        printf("[Fail] Use a database first!\n");
        return;
    }
    switch (field->ty) {
        case Field::Simple: {
            assert(field->attr.type != NO_TYPE);
            if (field->attr.attrName.length() >= MAXNAME) {
                printf("[Fail] Attribute name %s is too long\n", field->attr.attrName.c_str());
                return;
            }
            if (field->hasError) {
                printf("[Fail] Invalid Attr\n");
                return;
            }
            RC rc = SM_Manager::instance().AddAttr(*tbName, field->attr);
            if (rc != OK_RC) {
                printf("[Fail] Can not add col!\n");
                return;
            } else {
                printf("[Succ] col %s added to %s\n", field->attr.attrName.c_str(), tbName->c_str());
            }
            break;
        }
        case Field::Primary: {
            vector<string> attrNames;
            for (auto& col: *(field->columns)) {
                attrNames.push_back(*(col->colName));
            }
            if (!QL_Manager::instance().CanAddPrimaryKey(*tbName)) {
                printf("[Fail] Invalid data exists\n");
                return;
            }
            RC rc = SM_Manager::instance().AddPrimaryKey(*tbName, attrNames);
            if (rc != OK_RC) {
                printf("[Fail] Cannot add these primary keys to %s\n", tbName->c_str());
                return;
            } else {
                printf("[Succ] Primary keys added!\n");
                return;
            }
        }
        case Field::Foreign: {
            printf("[Fail] please provide fkName by using add Constraint\n");
            return;
        }
    }
}

void Parser::DropCol::visit() {
    if (!SM_Manager::instance().usingDb()) {
        printf("[Fail] Use a database first!\n");
        return;
    }
    RC rc = SM_Manager::instance().DropAttr(*tbName, *colName);
    if (rc != OK_RC) {
        printf("[Fail] Cannot drop col %s.%s\n", tbName->c_str(), colName->c_str());
        return;
    } else {
        printf("[Succ] col %s.%s dropped!\n", tbName->c_str(), colName->c_str());
        return;
    }
}

void Parser::ChangeCol::visit() {
    if (!SM_Manager::instance().usingDb()) {
        printf("[Fail] Use a database first!\n");
        return;
    }
    assert(field->attr.type != NO_TYPE);
    if (field->attr.attrName.length() >= MAXNAME) {
        printf("[Fail] Attribute name %s is too long\n", field->attr.attrName.c_str());
        return;
    }
    if (field->hasError) {
        printf("[Fail] Invalid Attr\n");
        return;
    }
    if (!QL_Manager::instance().CanChangeCol(*tbName)) {
        printf("[Fail] Invalid data in table\n");
    }
    RC rc = SM_Manager::instance().ChangeAttr(*tbName, *colName, field->attr);
    if (rc != OK_RC) {
        printf("[Fail] Can not change\n");
        return;
    } else {
        printf("[Succ] col %s change to %s\n", colName->c_str(), field->attr.attrName.c_str());
        return;
    }
}

void Parser::DropPrimaryKey::visit() {
    if (!SM_Manager::instance().usingDb()) {
        printf("[Fail] Use a database first!\n");
        return;
    }
    RC rc = SM_Manager::instance().DropPrimaryKey(*tbName);
    if (rc != OK_RC) {
        printf("[Fail] Cannot drop primary key of %s\n", tbName->c_str());
    } else {
        printf("[Succ] Primary key in %s dropped!\n", tbName->c_str());
    }
}

void Parser::AddForeignKey::visit() {
    std::string srcTb(*tbName);
    ForeignKeyInfo key;
    key.fkName = std::string(*fkName);
    key.refTable = std::string(*refTable);
    if (key.fkName == srcTb) {
        printf("[Fail] Cannot link with self\n");
        return;
    }
    for (Column* col: *srcCols) {
        key.attrs.push_back(*(col->colName));
    }
    std::vector<std::string> refAttrs;
    for (Column* col: *refCols) {
        refAttrs.push_back(*(col->colName));
    }
    if (!QL_Manager::instance().CanAddForeignKey(*tbName)) {
        printf("[Fail] Invalid value in table!\n");
        return;
    }
    RC rc = SM_Manager::instance().ShuffleForeign(srcTb, key, refAttrs);
    if (rc != OK_RC) {
        printf("[Fail] Not match!\n");
        return;
    }
    rc = SM_Manager::instance().AddForeignKey(srcTb, key);
    if (rc != OK_RC) {
        printf("[Fail] Can not add!");
        return;
    }
    printf("[Succ] foreign key %s added to %s\n", key.fkName.c_str(), srcTb.c_str());
}

void Parser::DropForeignKey::visit() {
    std::string srcTb(*tbName);
    std::string fk(*fkName);
    RC rc = SM_Manager::instance().DropForeignKey(srcTb, fk);
    if (rc != OK_RC) {
        printf("[Fail] Can not drop!");
        return;
    }
    printf("[Succ] foreigh key %s in %s is dropped\n", fk.c_str(), srcTb.c_str());
}

void Parser::AddUniqueKey::visit() {
    std::string srcTb(*tbName);
    std::string fk = std::string(*fkName);
    vector<string> attrs;
    for (Column* col: *cols) {
        attrs.push_back(*(col->colName));
    }
    if (!QL_Manager::instance().CanAddUniqueKey(*tbName)) {
        printf("[Fail] Invalid value in table!\n");
        return;
    }
    RC rc = SM_Manager::instance().AddUniqueKey(srcTb, fk, attrs);
    if (rc != OK_RC) {
        printf("[Fail] Can not add!");
        return;
    }
    printf("[Succ] unique key %s added to %s\n", fk.c_str(), srcTb.c_str());
}

void Parser::DropUniqueKey::visit() {
    std::string srcTb(*tbName);
    std::string fk(*fkName);
    RC rc = SM_Manager::instance().DropUniqueKey(srcTb, fk);
    if (rc != OK_RC) {
        printf("[Fail] Can not drop!");
        return;
    }
    printf("[Succ] unique key %s in %s is dropped\n", fk.c_str(), srcTb.c_str());
}