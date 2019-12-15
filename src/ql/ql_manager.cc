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

    vector<TableLine> items;
    rc = GetAllItems(tbName, items);
    QLRC(rc, QL_ERROR);
    return OK_RC;
}

RC QL_Manager::GetAllItems(const std::string& tbName, std::vector<TableLine>& values) {
    TableInfo table;
    RC rc = smm.GetTable(tbName, table);
    SMRC(rc, QL_INVALID_TABLE);
    values.clear();
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
    }
    rc = scanner.CloseScan();
    RMRC(rc, QL_ERROR);
    rc = rmm.CloseFile(handle);
    RMRC(rc, QL_ERROR);
    
    PrintTable(table, values);
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