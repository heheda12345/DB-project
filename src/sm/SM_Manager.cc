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
    // for (auto& attr: attributes) {
    //     if (attr.isForeign()) {
    //         rc = LinkForeign(relName, attr.attrName, attr.refTable, attr.refAttr);
    //         SMRC(rc, SM_ERROR);
    //     }
    // }
    return OK_RC;
}

RC SM_Manager::DropTable(const std::string& relName) {
    if (!usingDb()) {
        return SM_DB_NOT_OPEN;
    }
    TableInfo table;
    RC rc = GetTable(relName, table);
    SMRC(rc, SM_ERROR);
    // for (auto& attr: attrs) {
    //     if (!attr.linkedForeign.empty()) {
    //         return SM_OTHERS_FOREIGN;
    //     }
    // }
    rc = rmm.DestroyFile(relName.c_str());
    RMRC(rc, SM_ERROR);
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
    for (auto name: attrNames) {
        int idx = AttrInfo::getPos(table.attrs, name);
        if (idx == -1) {
            return SM_NO_SUCH_ATTR;
        }
    }
    table.primaryKeys = attrNames;
    rc = UpdateTable(tbName, table);
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
    return OK_RC;
}

// RC SM_Manager::AddForeignKey(const std::string& reqTb, const std::string& reqAttr, const std::string& dstTb, const std::string& dstAttr) {
//     if (!usingDb()) {
//         return SM_DB_NOT_OPEN;
//     }
//     vector<AttrInfo> attrs;
//     RC rc = GetAttrs(reqTb, attrs);
//     SMRC(rc, SM_ERROR);
//     int idx = AttrInfo::getPos(attrs, reqAttr);
//     assert(idx >= 0);
//     if (idx == -1)
//         return SM_NO_SUCH_ATTR;
//     attrs[idx].setForeign(dstTb, dstAttr);
//     rc = UpdateAttrs(reqTb, attrs);
//     SMRC(rc, SM_ERROR);
//     return OK_RC;
// }

// RC SM_Manager::DropForeignKey(const std::string& reqTb, const std::string& reqAttr) {
//     if (!usingDb()) {
//         return SM_DB_NOT_OPEN;
//     }
//     vector<AttrInfo> attrs;
//     RC rc = GetAttrs(reqTb, attrs);
//     SMRC(rc, SM_ERROR);
//     int idx = AttrInfo::getPos(attrs, reqAttr);
//     assert(idx >= 0);
//     if (idx == -1)
//         return SM_NO_SUCH_ATTR;
//     attrs[idx].setForeignFlag(0);
//     rc = UpdateAttrs(reqTb, attrs);
//     SMRC(rc, SM_ERROR);
//     return OK_RC;
// }

// RC SM_Manager::GetForeignDst(const std::string& reqTb, std::string& reqAttr, std::string& dstTb, std::string& dstAttr) {
//     if (!usingDb()) {
//         return SM_DB_NOT_OPEN;
//     }
//     vector<AttrInfo> attrs;
//     RC rc = GetAttrs(reqTb, attrs);
//     SMRC(rc, SM_ERROR);
//     int idx = AttrInfo::getPos(attrs, reqAttr);
//     if (idx == -1) {
//         return SM_NO_SUCH_ATTR;
//     }
//     if (!attrs[idx].isForeign()) {
//         return SM_NOT_FOREIGN;
//     }
//     dstTb = attrs[idx].refTable;
//     dstAttr = attrs[idx].refAttr;
//     return OK_RC;
// }

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

// bool SM_Manager::ExistAttr(const std::string& relName, const std::string& attrName, AttrType type) {
//     std::vector<AttrInfo> attrs;
//     RC rc = GetAttrs(relName, attrs);
//     RMRC(rc, 0);
//     int idx = AttrInfo::getPos(attrs, attrName);
//     return idx != -1 && (type == NO_TYPE || attrs[idx].type == type);
// }

// bool SM_Manager::LinkForeign(const std::string& reqTb, const std::string& reqAttr, const std::string& dstTb, const std::string& dstAttr) {
//     RM_FileHandle handle;
//     RC rc = rmm.OpenFile(dstTb.c_str(), handle);
//     assert(rc == OK_RC);
//     SMRC(rc, SM_ERROR);
//     int size;
//     rc = handle.GetMetaSize(size);
//     assert(rc == OK_RC);
//     SMRC(rc, SM_ERROR);
//     char header[size];
//     rc = handle.GetMeta(header, size);
//     std::vector<AttrInfo> attrs;
//     attrs = AttrInfo::loadAttrs(header);
//     int idx = AttrInfo::getPos(attrs, dstAttr);
//     attrs[idx].linkedForeign.push_back(make_pair(reqTb, reqAttr));
//     char headerNew[AttrInfo::getAttrsSize(attrs)];
//     size = AttrInfo::dumpAttrs(headerNew, attrs);
//     rc = handle.SetMeta(headerNew, size);
//     assert(rc == OK_RC);
//     SMRC(rc, SM_ERROR);
//     rc = rmm.CloseFile(handle);
//     assert(rc == OK_RC);
//     SMRC(rc, SM_ERROR);
//     return 0;
// }