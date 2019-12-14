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

RC SM_Manager::CreateTable(const std::string& relName, const TableInfo& table) {
    if (!usingDb()) {
        return SM_DB_NOT_OPEN;
    }
    char header[table.getSize()];
    table.dump(header);
    RC rc = rmm.CreateFile(relName.c_str(), AttrInfo::getRecordSize(table.attrs),
        header, table.getSize());
    RMRC(rc, SM_ERROR);
    for (auto& fKey: table.foreignGroups) {
        rc = LinkForeign(relName, fKey);
        assert(rc == OK_RC);
        SMRC(rc, SM_ERROR);
    }
    if (table.hasPrimary()) {
        RC rc = CreateIndex(relName, "@Primary", table.primaryKeys);
        assert(rc == OK_RC);
        SMRC(rc, SM_ERROR);
    }
    return OK_RC;
}

RC SM_Manager::DropTable(const std::string& relName) {
    if (!usingDb()) {
        return SM_DB_NOT_OPEN;
    }
    TableInfo table;
    RC rc = GetTable(relName, table);
    SMRC(rc, SM_ERROR);
    if (table.linkedByOthers())
        return SM_OTHERS_FOREIGN;
    rc = rmm.DestroyFile(relName.c_str());
    RMRC(rc, SM_ERROR);
    if (table.hasPrimary()) {
        RC rc = DropIndex(relName, "@Primary");
        assert(rc == OK_RC);
        SMRC(rc, SM_ERROR);
    }
    return OK_RC;
}

RC SM_Manager::ShowTable(const std::string& relName) {
    TableInfo table;
    RC rc = GetTable(relName, table);
    SMRC(rc, SM_ERROR);
    cout << table << endl;
    return OK_RC;
}

RC SM_Manager::ShowTables() {
    if (!usingDb()) {
        return SM_DB_NOT_OPEN;
    }
    auto tables = getAllTable(".");
    for (auto& tb: tables) {
        TableInfo table;
        RC rc = GetTable(tb, table);
        SMRC(rc, SM_ERROR);
        assert(rc == OK_RC);
        cout << tb << std::string(": ") << table << endl; 
    }
    return OK_RC;
}


RC SM_Manager::AddPrimaryKey(const std::string& tbName, const std::vector<std::string>& attrNames) {
    if (!usingDb()) {
        return SM_DB_NOT_OPEN;
    }
    TableInfo table;
    RC rc = GetTable(tbName, table);
    SMRC(rc, SM_ERROR);
    if (table.hasPrimary())
        return SM_HAS_PRIMARY;
    if (isDumplicated(attrNames)) {
        return SM_DUMPLICATED;
    }
    for (auto name: attrNames) {
        int idx = AttrInfo::getPos(table.attrs, name);
        if (idx == -1) {
            return SM_NO_SUCH_ATTR;
        }
    }
    table.primaryKeys = attrNames;
    table.setPrimaryNotNull();
    rc = UpdateTable(tbName, table);
    SMRC(rc, SM_ERROR);
    rc = CreateIndex(tbName, "@Primary", table.primaryKeys);
    assert(rc == OK_RC);
    SMRC(rc, SM_ERROR);
    return OK_RC;
}

RC SM_Manager::DropPrimaryKey(const std::string& tbName) {
    if (!usingDb()) {
        return SM_DB_NOT_OPEN;
    }
    TableInfo table;
    RC rc = GetTable(tbName, table);
    SMRC(rc, SM_ERROR);
    if (!table.hasPrimary())
        return SM_NO_PRIMARY;
    if (table.linkedByOthers())
        return SM_OTHERS_FOREIGN;
    table.primaryKeys.clear();
    rc = UpdateTable(tbName, table);
    SMRC(rc, SM_ERROR);
    rc = DropIndex(tbName, "@Primary");
    assert(rc == OK_RC);
    SMRC(rc, SM_ERROR);
    return OK_RC;
}

RC SM_Manager::AddForeignKey(const std::string& tbName, const ForeignKeyInfo& fKey) {
    if (!usingDb()) {
        return SM_DB_NOT_OPEN;
    }
    TableInfo table;
    RC rc = GetTable(tbName, table);
    SMRC(rc, SM_ERROR);
    table.foreignGroups.push_back(fKey);
    rc = UpdateTable(tbName, table);
    assert(rc == OK_RC);
    SMRC(rc, SM_ERROR);
    rc = LinkForeign(tbName, fKey);
    assert(rc == OK_RC);
    SMRC(rc, SM_ERROR);
    return OK_RC;
}

RC SM_Manager::DropForeignKey(const std::string& reqTb, const std::string& fkName) {
    if (!usingDb()) {
        return SM_DB_NOT_OPEN;
    }
    TableInfo table;
    RC rc = GetTable(reqTb, table);
    SMRC(rc, SM_ERROR);
    int idx = ForeignKeyInfo::getPos(table.foreignGroups, fkName);
    if (idx == -1) {
        return SM_NO_SUCH_KEY;
    }
    std::string refTb = table.foreignGroups[idx].refTable;
    table.foreignGroups.erase(table.foreignGroups.begin() + idx);
    rc = UpdateTable(reqTb, table);
    SMRC(rc, SM_ERROR);
    rc = DropForeignLink(refTb, fkName);
    assert(rc == OK_RC);
    SMRC(rc, SM_ERROR);
    return OK_RC;
}

RC SM_Manager::AddAttr(const std::string& tbName, const AttrInfo& attr) {
    if (!usingDb()) {
        return SM_DB_NOT_OPEN;
    }
    TableInfo table;
    RC rc = GetTable(tbName, table);
    SMRC(rc, SM_ERROR);
    if (AttrInfo::getPos(table.attrs, attr.attrName) != -1) {
        return SM_DUMPLICATED;
    }
    table.attrs.push_back(attr);
    rc = UpdateTable(tbName, table);
    SMRC(rc, SM_ERROR);
    return OK_RC;
}

RC SM_Manager::DropAttr(const std::string& tbName, const std::string& attrName) {
    if (!usingDb()) {
        return SM_DB_NOT_OPEN;
    }
    TableInfo table;
    RC rc = GetTable(tbName, table);
    SMRC(rc, SM_ERROR);
    if (findName(table.primaryKeys, attrName) != -1) {
        return SM_IS_PRIMARY;
    }
    if (ForeignKeyInfo::isForeignKey(table.foreignGroups, attrName)) {
        return SM_IS_FOREIGN;
    }
    int idx = AttrInfo::getPos(table.attrs, attrName);
    if (idx == -1) {
        return SM_NO_SUCH_KEY;
    }
    table.attrs.erase(table.attrs.begin() + idx);
    rc = UpdateTable(tbName, table);
    SMRC(rc, SM_ERROR);
    return OK_RC;
}

RC SM_Manager::ChangeAttr(const std::string& tbName, const std::string& attrName, const AttrInfo& newAttr) {
    if (!usingDb()) {
        return SM_DB_NOT_OPEN;
    }
    TableInfo table;
    RC rc = GetTable(tbName, table);
    SMRC(rc, SM_ERROR);
    if (findName(table.primaryKeys, attrName) != -1) {
        return SM_IS_PRIMARY;
    }
    if (ForeignKeyInfo::isForeignKey(table.foreignGroups, attrName)) {
        return SM_IS_FOREIGN;
    }
    if (AttrInfo::getPos(table.attrs, newAttr.attrName) != -1) {
        return SM_DUMPLICATED;
    }
    int idx = AttrInfo::getPos(table.attrs, attrName);
    if (idx == -1) {
        return SM_NO_SUCH_KEY;
    }
    table.attrs[idx] = newAttr;
    rc = UpdateTable(tbName, table);
    SMRC(rc, SM_ERROR);
    return OK_RC;
}

RC SM_Manager::CreateIndex(const std::string& tbName, const std::string& idxName, const std::vector<std::string>& attrNames) {
    if (!usingDb()) {
        return SM_DB_NOT_OPEN;
    }
    TableInfo table;
    RC rc = GetTable(tbName, table);
    SMRC(rc, SM_ERROR);
    
    for (auto name: attrNames) {
        int idx = AttrInfo::getPos(table.attrs, name);
        if (idx == -1) {
            return SM_NO_SUCH_ATTR;
        }
        if (!table.attrs[idx].isNotNull()) {
            return SM_REQUIRE_NOT_NULL;
        }
    }
    
    if (IndexInfo::getPos(table.indexes, idxName) != -1) {
        return SM_DUMPLICATED;
    }

    IndexInfo idx;
    idx.idxName = idxName;
    idx.idxID = IndexInfo::getNextId(table.indexes);
    idx.attrs = attrNames;

    rc = ixm.CreateIndex(tbName.c_str(), idx.idxID,
        AttrInfo::mapType(table.attrs, attrNames),
        AttrInfo::mapMxLen(table.attrs, attrNames));
    IXRC(rc, SM_ERROR);

    table.indexes.push_back(idx);
    rc = UpdateTable(tbName, table);
    SMRC(rc, SM_ERROR);
    return OK_RC;
}

RC SM_Manager::DropIndex(const std::string& tbName, const std::string& idxName) {
    if (!usingDb()) {
        return SM_DB_NOT_OPEN;
    }
    TableInfo table;
    RC rc = GetTable(tbName, table);
    SMRC(rc, SM_ERROR);

    int pos = IndexInfo::getPos(table.indexes, idxName);
    if (pos == -1) {
        return SM_NO_SUCH_KEY;
    }
    
    rc = ixm.DestroyIndex(tbName.c_str(), table.indexes[pos].idxID);
    IXRC(rc, SM_ERROR);

    table.indexes.erase(table.indexes.begin() + pos);
    rc = UpdateTable(tbName, table);
    SMRC(rc, SM_ERROR);
    return OK_RC;
}

RC SM_Manager::GetTable(const std::string& relName, TableInfo& table) {
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
    int loaded = table.load(header);
    assert(loaded == size);
    rc = rmm.CloseFile(handle);
    RMRC(rc, SM_ERROR);
    return OK_RC;
}

RC SM_Manager::UpdateTable(const std::string& tbName, const TableInfo& table) {
    if (!usingDb()) {
        return SM_DB_NOT_OPEN;
    }
    RM_FileHandle handle;
    RC rc = rmm.OpenFile(tbName.c_str(), handle);
    RMRC(rc, SM_ERROR);
    char header[table.getSize()];
    int size = table.dump(header); assert(size == table.getSize());
    rc = handle.SetMeta(header, table.getSize());
    RMRC(rc, SM_ERROR);
    rc = rmm.CloseFile(handle);
    RMRC(rc, SM_ERROR);
    return OK_RC;
}

RC SM_Manager::ShuffleForeign(const std::string& srcTbName, ForeignKeyInfo &key, const std::vector<std::string>& refAttrs) {
    if (!usingDb()) {
        return SM_DB_NOT_OPEN;
    }
    TableInfo table;
    RC rc = GetTable(srcTbName, table);
    SMRC(rc, SM_ERROR);
    return ShuffleForeign(table, key, refAttrs);
}

RC SM_Manager::ShuffleForeign(const TableInfo& srcTable, ForeignKeyInfo &key, const std::vector<std::string>& refAttrs) {
    if (!usingDb()) {
        return SM_DB_NOT_OPEN;
    }
    TableInfo table;
    RC rc = GetTable(key.refTable, table);
    SMRC(rc, SM_ERROR);
    std::vector<std::string> srcAttrs;
    if (key.attrs.size() != refAttrs.size() || refAttrs.size() != table.primaryKeys.size())
        return SM_FOREIGN_NOT_MATCH;
    if (key.fkName != "@@" &&
        (ForeignKeyInfo::getPos(srcTable.foreignGroups, key.fkName) != -1 ||
         ForeignKeyInfo::getPos(table.linkedBy, key.fkName) != -1)) {
        return SM_DUMPLICATED;
    }
    for (auto& x: table.primaryKeys) {
        int idx = findName(refAttrs, x);
        if (idx == -1) {
            return SM_FOREIGN_NOT_MATCH;
        }
        int idxSrc = AttrInfo::getPos(srcTable.attrs, key.attrs[idx]);
        int idxDst = AttrInfo::getPos(table.attrs, x);
        assert(idxDst != -1);
        if (idxSrc == -1 || idxDst == -1 || srcTable.attrs[idxSrc].type != table.attrs[idxDst].type) {
            return SM_FOREIGN_NOT_MATCH;
        }
        srcAttrs.push_back(key.attrs[idx]);
    }
    key.attrs = srcAttrs;
    return OK_RC;
}

// bool SM_Manager::ExistAttr(const std::string& relName, const std::string& attrName, AttrType type) {
//     std::vector<AttrInfo> attrs;
//     RC rc = GetAttrs(relName, attrs);
//     RMRC(rc, 0);
//     int idx = AttrInfo::getPos(attrs, attrName);
//     return idx != -1 && (type == NO_TYPE || attrs[idx].type == type);
// }

RC SM_Manager::LinkForeign(const std::string& reqTb, const ForeignKeyInfo &key) {
    if (!usingDb()) {
        return SM_DB_NOT_OPEN;
    }
    TableInfo table;
    RC rc = GetTable(key.refTable, table);
    SMRC(rc, SM_ERROR);
    ForeignKeyInfo refKey(key);
    refKey.refTable = reqTb;
    table.linkedBy.push_back(refKey);
    rc = UpdateTable(key.refTable, table);
    SMRC(rc, SM_ERROR);
    return OK_RC;
}

RC SM_Manager::DropForeignLink(const std::string& refTb, const std::string& fkName) {
    if (!usingDb()) {
        return SM_DB_NOT_OPEN;
    }
    TableInfo table;
    RC rc = GetTable(refTb, table);
    SMRC(rc, SM_ERROR);
    int idx = ForeignKeyInfo::getPos(table.linkedBy, fkName);
    if (idx == -1)
        return SM_NO_SUCH_KEY;
    table.linkedBy.erase(table.linkedBy.begin() + idx);
    rc = UpdateTable(refTb, table);
    SMRC(rc, SM_ERROR);
    return OK_RC;
}