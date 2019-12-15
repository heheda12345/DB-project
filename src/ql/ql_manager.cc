#include "ql.h"
using namespace std;

 RC QL_Manager::Insert(const std::string& tbName, const std::vector<Item>& values_i) {
    //  TableInfo ti;
    //  RC rc = smm.GetTable(tbName, ti);
    //  SMRC(rc, QL_INVALID_TABLE);
     std::vector<Item> values = values_i;
     cout << values << endl;

     return OK_RC;
 }