#include "ql.h"
#include <iomanip>
using namespace std;

RC QL_Manager::Insert(const std::string& tbName, const TableLine& values_i) {
    TableInfo table;
    RC rc = smm.GetTable(tbName, table);
    SMRC(rc, QL_INVALID_TABLE);
    TableLine value = values_i;
    rc = formatItem(table, value);
    if (rc) return rc;

    // check primary key
    if (table.hasPrimary()) {
        if (ExistInIndex(tbName, table, "@Primary", value))
            return QL_DUMPLICATED;
    }
    // check foreign key
    for (auto& fi: table.foreignGroups) {
        if (!ExistInIndex(table, fi.fkName, value, fi.refTable, "@Primary"))
            return QL_NOT_IN_FOREIGN;
    }
    // check unique key
    for (auto& pi: table.uniqueGroups) {
        if (ExistInIndex(tbName, table, std::string("@Unique.").append(pi.idxName), value))
            return QL_DUMPLICATED;
    }

    RM_FileHandle handle;
    rc = rmm.OpenFile(tbName.c_str(), handle);
    RMRC(rc, QL_ERROR);
    char pool[Item::getLineSize(table.attrs)];
    int size = Item::dumpTableLine(pool, value, table.attrs);
    assert(size == Item::getLineSize(table.attrs));

    RID rid;
    rc = handle.InsertRec(pool, rid);
    assert(rc == OK_RC);
    RMRC(rc, QL_ERROR);
    rc = rmm.CloseFile(handle);
    assert(rc == OK_RC);
    RMRC(rc, QL_ERROR);

    for (auto& idx: table.indexes) {
        IX_IndexHandle handle;
        rc = ixm.OpenIndex(tbName.c_str(), idx.idxID, handle);
        assert(rc == OK_RC);
        IXRC(rc, QL_ERROR);
        rc = handle.InsertEntry(formatIndex(table.attrs, idx.attrs, value), rid);
        assert(rc == OK_RC);
        IXRC(rc, QL_ERROR);
        rc = ixm.CloseIndex(handle);
        assert(rc == OK_RC);
        IXRC(rc, QL_ERROR);
    }
    return OK_RC;
}

RC QL_Manager::Delete(const std::string& tbName, const vector<RawSingleWhere>& RawConds) {
    TableInfo table;
    RC rc = smm.GetTable(tbName, table);
    SMRC(rc, QL_INVALID_TABLE);
    if (table.linkedByOthers()) {
        return QL_LINKED_BY_OTHERS;
    }
    std::vector<SingleWhere> conds;
    rc = CompileWheres(conds, RawConds, table.attrs, tbName);
    QLRC(rc, rc);
    std::vector<TableLine> values;
    std::vector<RID> rids;
    rc = GetAllItems(tbName, values, rids);
    auto selected = select(values, rids, conds);
    auto& selectedValues = selected.first;
    auto& selectedRids = selected.second;
    PrintTable(table, selectedValues);

    RM_FileHandle handle;
    rc = rmm.OpenFile(tbName.c_str(), handle); MUST_SUCC;
    for (auto& rid: selectedRids) {
        rc = handle.DeleteRec(rid); MUST_SUCC;
    }
    rc = rmm.CloseFile(handle); MUST_SUCC;

    for (auto& idx: table.indexes) {
        IX_IndexHandle handle;
        rc = ixm.OpenIndex(tbName.c_str(), idx.idxID, handle); MUST_SUCC;
        IXRC(rc, QL_ERROR);
        assert(selectedValues.size() == selectedRids.size());
        for (int i = 0; i <selectedValues.size(); i++) {
            rc = handle.DeleteEntry(formatIndex(table.attrs, idx.attrs, selectedValues[i]), selectedRids[i]);
            assert(rc == OK_RC);
            IXRC(rc, QL_ERROR);
        }
        rc = ixm.CloseIndex(handle); MUST_SUCC;
        IXRC(rc, QL_ERROR);
    }
    return OK_RC;
}

RC QL_Manager::Update(const std::string& tbName, const std::vector<RawSetJob> &rawJobs, const std::vector<RawSingleWhere>& rawConds) {
    TableInfo table;
    RC rc = smm.GetTable(tbName, table);
    SMRC(rc, QL_INVALID_TABLE);
    if (table.linkedByOthers()) {
        return QL_LINKED_BY_OTHERS;
    }
    std::vector<SingleWhere> conds;
    rc = CompileWheres(conds, rawConds, table.attrs, tbName);
    QLRC(rc, rc);
    // printf("where compiled!\n");

    std::vector<SetJob> setJobs;
    rc = CompileSetJobs(setJobs, rawJobs, table.attrs);
    QLRC(rc, rc);
    // printf("set compiled!\n");

    std::vector<TableLine> values;
    std::vector<RID> rids;
    rc = GetAllItems(tbName, values, rids);
    // printf("get all items!\n");
    auto selected = select(values, rids, conds);
    auto& selectedValues = selected.first;
    auto& selectedRids = selected.second;
    auto updated = DoSetJobs(selectedValues, setJobs);

    // TODO check constraints
    RM_FileHandle handle;
    assert(updated.size() == selectedRids.size());
    rc = rmm.OpenFile(tbName.c_str(), handle); MUST_SUCC;
    for (int i = 0; i < selectedValues.size(); i++) {
        char pool[Item::getLineSize(table.attrs)];
        int size = Item::dumpTableLine(pool, updated[i], table.attrs);
        RM_Record record(pool, size, selectedRids[i]);
        rc = handle.UpdateRec(record); MUST_SUCC;
    }
    rc = rmm.CloseFile(handle); MUST_SUCC;

    for (auto& idx: table.indexes) {
        IX_IndexHandle handle;
        rc = ixm.OpenIndex(tbName.c_str(), idx.idxID, handle); MUST_SUCC;
        assert(selectedValues.size() == selectedRids.size());
        for (int i = 0; i <selectedValues.size(); i++) {
            rc = handle.DeleteEntry(formatIndex(table.attrs, idx.attrs, selectedValues[i]), selectedRids[i]); MUST_SUCC;
            rc = handle.InsertEntry(formatIndex(table.attrs, idx.attrs, updated[i]), selectedRids[i]);
            MUST_SUCC;
        }
        rc = ixm.CloseIndex(handle); MUST_SUCC;
        IXRC(rc, QL_ERROR);
    }
    return OK_RC;
}

RC QL_Manager::Desc(const std::string& tbName) {
    TableInfo table;
    RC rc = smm.GetTable(tbName, table);
    SMRC(rc, QL_INVALID_TABLE);
    std::vector<TableLine> values;
    std::vector<RID> rids;
    rc = GetAllItems(tbName, values, rids);
    QLRC(rc, QL_ERROR);
    PrintTable(table, values);
    return OK_RC;
}

RC QL_Manager::GetAllItems(const std::string& tbName, std::vector<TableLine>& values, std::vector<RID>& rids) {
    TableInfo table;
    RC rc = smm.GetTable(tbName, table);
    SMRC(rc, QL_INVALID_TABLE);
    values.clear();
    rids.clear();
    RM_FileHandle handle;
    rc = rmm.OpenFile(tbName.c_str(), handle);
    RMRC(rc, QL_ERROR);
    RM_FileScan scanner;
    rc = scanner.OpenScan(handle);
    RM_Record record;
    while (1) {
        rc = scanner.GetNextRec(record);
        if (rc == RM_EOF)
            break;
        assert(rc ==OK_RC);
        RMRC(rc, QL_ERROR);
        char* pData;
        rc = record.GetData(pData);
        assert(rc == OK_RC);
        RMRC(rc, QL_ERROR);
        TableLine value = Item::loadTableLine(pData, table.attrs);
        values.push_back(value);
        RID rid;
        rc = record.GetRid(rid);
        assert(rc == OK_RC);
        RMRC(rc, QL_ERROR);
        rids.push_back(rid);
    }
    rc = scanner.CloseScan();
    RMRC(rc, QL_ERROR);
    rc = rmm.CloseFile(handle);
    RMRC(rc, QL_ERROR);
    
    return OK_RC;
}

void QL_Manager::PrintTable(const TableInfo& table, const std::vector<TableLine>& values) {
    cout << endl;
    int n = table.attrs.size();
    std::string line; while (line.length() < PRINT_WIDTH + 1) line.append("-");
    cout << "+"; for (int i = 0; i < n; i++) cout << line << "+"; cout << endl;
    cout << "|";
    for (auto& attr: table.attrs) {
        cout << setiosflags(ios::left) << " " << setw(PRINT_WIDTH) << cutForPrint(attr.attrName) << "|";
    }
    cout << endl;
    cout << "+"; for (int i = 0; i < n; i++) cout << line << "+"; cout << endl;
    for (auto& value: values) {
        cout << value << endl;
    }
    cout << "+"; for (int i = 0; i < n; i++) cout << line << "+"; cout << endl;
}

std::vector<RID> QL_Manager::SearchIndex(const std::string& tbName, const std::string& idxName, const std::vector<std::string> & toSearch, CompOp compOp) {
    TableInfo table;
    RC rc = smm.GetTable(tbName, table); 
    assert(rc == OK_RC);
    int pos = IndexInfo::getPos(table.indexes, idxName);
    assert(pos != -1);
    auto& indexInfo = table.indexes[pos];
    vector<RID> rids;
    IX_IndexHandle handle;
    rc = ixm.OpenIndex(tbName.c_str(), indexInfo.idxID, handle);
    assert(rc == OK_RC);
    rc = IX_IndexScan::GetEntries(handle, compOp, toSearch, rids);
    assert(rc == OK_RC);
    // printf("[result] rid.size = %d\n", (int)rids.size());
    return rids;
}

bool QL_Manager::ExistInIndex(const TableInfo& table, const std::string& fkName, const TableLine& value, const std::string& refTbName, const std::string& refIdxName) {
    int pos = ForeignKeyInfo::getPos(table.foreignGroups, fkName);
    // printf("searching %s\n", fkName.c_str());
    assert(pos != -1);
    auto toSearch = formatIndex(table.attrs, table.foreignGroups[pos].attrs, value);
    return SearchIndex(refTbName, refIdxName, toSearch, EQ_OP).size() > 0;
}

bool QL_Manager::ExistInIndex(const std::string& tbName, const TableInfo& table, const std::string& idxName, const TableLine& value) {
    int pos = IndexInfo::getPos(table.indexes, idxName);
    // printf("searching %s\n", idxName.c_str());
    assert(pos != -1);
    auto toSearch = formatIndex(table.attrs, table.indexes[pos].attrs, value);
    return SearchIndex(tbName, idxName, toSearch, EQ_OP).size() > 0;
}
