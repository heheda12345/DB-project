#include "ql.h"
using namespace std;

 RC QL_Manager::Insert(const std::string& tbName, const std::vector<Item>& values_i) {
     TableInfo table;
     RC rc = smm.GetTable(tbName, table);
     SMRC(rc, QL_INVALID_TABLE);
     std::vector<Item> values = values_i;
     rc = formatItem(table, values);
     if (rc) return rc;
     cout << values << endl;
     return OK_RC;
 }