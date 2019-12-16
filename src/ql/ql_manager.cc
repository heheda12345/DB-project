#include "ql.h"
#include <iomanip>
#include <set>
#include <algorithm>
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

    std::vector<SetJob> setJobs;
    rc = CompileSetJobs(setJobs, rawJobs, table.attrs);
    QLRC(rc, rc);

    std::vector<TableLine> values;
    std::vector<RID> rids;
    rc = GetAllItems(tbName, values, rids);
    auto selected = select(values, rids, conds);
    auto& selectedValues = selected.first;
    auto& selectedRids = selected.second;
    auto updated = DoSetJobs(selectedValues, setJobs);

    // check primary key
    if (table.hasPrimary()) {
        if (!CanUpdate(tbName, table, "@Primary", updated, selectedRids))
            return QL_DUMPLICATED;
    }
    // check foreign key
    for (auto& fi: table.foreignGroups) {
        for (auto value: updated) {
            if (!ExistInIndex(table, fi.fkName, value, fi.refTable, "@Primary"))
                return QL_NOT_IN_FOREIGN;
        }
    }
    // check unique key
    for (auto& pi: table.uniqueGroups) {
        if (!CanUpdate(tbName, table, std::string("@Unique.").append(pi.idxName), updated, selectedRids))
            return QL_DUMPLICATED;
    }

    RM_FileHandle handle;
    assert(updated.size() == selectedRids.size());
    rc = rmm.OpenFile(tbName.c_str(), handle); MUST_SUCC;
    for (int i = 0; i < updated.size(); i++) {
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
            rc = handle.InsertEntry(formatIndex(table.attrs, idx.attrs, updated[i]), selectedRids[i]); MUST_SUCC;
        }
        rc = ixm.CloseIndex(handle); MUST_SUCC;
        IXRC(rc, QL_ERROR);
    }
    return OK_RC;
}

RC QL_Manager::Select(const std::vector<std::string>& tbNames, std::vector<RawTbAttr>& rawSelectors, const std::vector<RawSingleWhere>& rawSingleConds, const std::vector<RawDualWhere>& rawDualConds, GatherOp gOp) {
    map<string, TableInfo> tables;
    map<string, vector<AttrInfo>> attrss;

    // table
    for (auto& tbName: tbNames) {
        TableInfo table;
        RC rc = smm.GetTable(tbName, table);
        SMRC(rc, QL_INVALID_TABLE);
        tables[tbName] = table;
        attrss[tbName] = table.attrs;
    }

    // compile singleConds
    map<string, vector<SingleWhere>> singleConds;
    RC rc = CompileWheres(singleConds, rawSingleConds, attrss);
    QLRC(rc, rc);

    // compile dualConds
    vector<DualWhere> dualConds;
    rc = CompileDualWheres(dualConds, rawDualConds, attrss);
    QLRC(rc, rc);

    // compile selector
    for (auto& selector: rawSelectors) {
        // printf("selector %s.%s\n", selector.first.c_str(), selector.second.c_str());
        if (tables.find(selector.first) == tables.end()) {
            return QL_NO_SUCH_KEY;
        }
        if (AttrInfo::getPos(tables[selector.first].attrs, selector.second) == -1) {
            return QL_NO_SUCH_KEY;
        }
    }
    if (rawSelectors.size() == 0) {
        for (auto& tb: tables) {
            for (auto attr: tb.second.attrs)
                rawSelectors.push_back(make_pair(tb.first, attr.attrName));
        }
    }

    //gather values
    map<string, vector<TableLine>> valuess;
    for (auto& tbName: tbNames) {
        vector<TableLine> values;
        vector<RID> rids;
        rc = GetAllItems(tbName, values, rids);
        auto selected = select(values, rids, singleConds[tbName]);
        valuess[tbName] = selected.first;
    }

    vector<AttrInfo> joinedInfo;
    vector<TableLine> joinedValue;
    Join(joinedInfo, joinedValue, valuess, attrss, dualConds, rawSelectors);
    if (gOp == NO_GOP) {
        PrintTable(joinedInfo, joinedValue);
    } else {
        assert(joinedInfo.size() == 1);
        AttrInfo info = joinedInfo[0];
        vector<int> iValues;
        vector<float> fValues;
        if (info.type == INT) {
            for (auto& x: joinedValue) {
                assert(x.size() == 1);
                if (!x[0].isNull)
                    iValues.push_back(*reinterpret_cast<const int*>(x[0].value.c_str()));
            }
            if (iValues.size() == 0) {
                return QL_NOTHING_IS_FOUND;
            }
        } else if (info.type == FLOAT) {
            for (auto& x: joinedValue) {
                assert(x.size() == 1);
                if (!x[0].isNull)
                    fValues.push_back(*reinterpret_cast<const float*>(x[0].value.c_str()));
            }
            if (fValues.size() == 0) {
                return QL_NOTHING_IS_FOUND;
            }
        } else {
            return QL_TYPE_NOT_MATCH;
        }
        switch (gOp)
        {
            case AVG_GOP: {
                if (info.type == FLOAT) {
                    float sum = 0;
                    for (auto& f: fValues) {
                        sum += f;
                    }
                    printf("Average: %f\n", sum / fValues.size());
                } else {
                    long long sum = 0;
                    for (auto& x: iValues) {
                        sum += x;
                    }
                    printf("Average: %f\n", sum * 1.0 / iValues.size());
                }
                break;
            }
            case SUM_GOP: {
                if (info.type == FLOAT) {
                    float sum = 0;
                    for (auto& f: fValues) {
                        sum += f;
                    }
                    printf("Sum: %f\n", sum);
                } else {
                    long long sum = 0;
                    for (auto& x: iValues) {
                        sum += x;
                    }
                    printf("Sum: %lld\n", sum);
                }
                break;
            }
            case MIN_GOP: {
                if (info.type == FLOAT) {
                    float mn = fValues[0];
                    for (auto& f: fValues) {
                        mn = min(mn, f);
                    }
                    printf("Min: %f\n", mn);
                } else {
                    int mn = iValues[0];
                    for (auto& x: iValues) {
                        mn = min(mn, x);
                    }
                    printf("Min: %d\n", mn);
                }
                break;
            }
            case MAX_GOP: {
                if (info.type == FLOAT) {
                    float mx = fValues[0];
                    for (auto& f: fValues) {
                        mx = max(mx, f);
                    }
                    printf("Max: %f\n", mx);
                } else {
                    int mx = iValues[0];
                    for (auto& x: iValues) {
                        mx = max(mx, x);
                    }
                    printf("Max: %d\n", mx);
                }
                break;
            }
            default: {
                assert(false);
            }
        }
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
    PrintTable(table.attrs, values);
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

void QL_Manager::PrintTable(const std::vector<AttrInfo>& attrs, const std::vector<TableLine>& values) {
    cout << endl;
    int n = attrs.size();
    std::string line; while (line.length() < PRINT_WIDTH + 1) line.append("-");
    cout << "+"; for (int i = 0; i < n; i++) cout << line << "+"; cout << endl;
    cout << "|";
    for (auto& attr: attrs) {
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

bool QL_Manager::CanUpdate(const std::string &tbName, const TableInfo& table, const std::string& idxName, const std::vector<TableLine>& toUpdate, const std::vector<RID>& rids) {
    int pos = IndexInfo::getPos(table.indexes, idxName);
    assert(pos != -1);
    set<RID> sRid;
    for (auto& rid: rids) {
        sRid.insert(rid);
    }

    vector<vector<string>> formated;
    for (auto& value: toUpdate) {
        formated.push_back(formatIndex(table.attrs, table.indexes[pos].attrs, value));
    }
    if (isDumplicated(formated))
        return 0;
    for (auto& toSearch: formated) {
        auto result = SearchIndex(tbName, idxName, toSearch, EQ_OP);
        for (auto& findRid: result) {
            if (sRid.find(findRid) == sRid.end())
                return 0;
        }
    }
    return 1;
}

void QL_Manager::Join(std::vector<AttrInfo>& joinedInfo,
          std::vector<TableLine>& joinedValue, 
          const std::map<std::string, std::vector<TableLine>>& values,
          const std::map<std::string, std::vector<AttrInfo>>& attrss,
          const std::vector<DualWhere>& dualConds_i,
          const std::vector<RawTbAttr>& toShow) {
    
    vector<pair<int, string>> sizeToName;
    for (auto& x: values) {
        sizeToName.push_back(make_pair(x.second.size(), x.first));
    }
    sort(sizeToName.begin(), sizeToName.end());
    map<string, int> nameToRank;
    for (int i = 0; i < sizeToName.size(); i++) {
        nameToRank[sizeToName[i].second] = i;
    }

    map<string, vector<DualWhere>> conds;
    for (auto cond: dualConds_i) {
        if (nameToRank[cond.tbName1] > nameToRank[cond.tbName2]) {
            swap(cond.tbName1, cond.tbName2);
            swap(cond.idx1, cond.idx2);
        }
        conds[cond.tbName2].push_back(cond);
    }
    
    vector<vector<AttrInfo>> joiningInfos;
    vector<vector<TableLine>> joiningValues;
    for (int i = 0; i < sizeToName.size(); i++) {
        string tbName = sizeToName[i].second;
        joiningInfos.push_back(attrss.find(tbName)->second);
        if (i == 0) {
            joiningValues.push_back(values.find(tbName)->second);
            continue;
        }
        
        auto oldValues = joiningValues;
        auto toInsert = values.find(tbName)->second;
        joiningValues.clear();
        joiningValues.resize(i + 1);
        int n = oldValues[0].size();

        if (conds[tbName].empty()) {
            for (int j = 0; j < n; j++) {
                for (int k = 0; k < toInsert.size(); k++) {
                    for (int d = 0; d < i; d++) {
                        joiningValues[d].push_back(oldValues[d][j]);
                    }
                    joiningValues[i].push_back(toInsert[k]);
                }
            }
            continue;
        }

        map<string, vector<int>> mp;
        int tableID = nameToRank[conds[tbName][0].tbName1], colID = conds[tbName][0].idx1;
        for (int j = 0; j < n; j++)
            if (!oldValues[tableID][j][colID].isNull)
                mp[oldValues[tableID][j][colID].value].push_back(j);
        int colID2 = conds[tbName][0].idx2;
        for (auto& tableLine: toInsert) {
            if (tableLine[colID2].isNull)
                continue;
            if (mp.find(tableLine[colID2].value) != mp.end()) {
                for (auto& matched: mp[tableLine[colID2].value]) {
                    for (int d = 0; d < i; d++) {
                        joiningValues[d].push_back(oldValues[d][matched]);
                    }
                    joiningValues[i].push_back(tableLine);
                }
            }
        }
        for (int cc = 1; cc < conds[tbName].size(); cc++) {
            tableID = nameToRank[conds[tbName][cc].tbName1], colID = conds[tbName][cc].idx1;
            colID2 = conds[tbName][cc].idx2;
            oldValues = joiningValues;
            joiningValues.clear();
            joiningValues.resize(i + 1);
            
            n = oldValues[0].size();
            for (int j = 0; j < n; j++) {
                auto& value1 = oldValues[tableID][j][colID], value2 = oldValues[tableID][j][i];
                if (value1.isNull || value2.isNull) continue;
                if (value1.value == value2.value) {
                    for (int d = 0; d <= i; d++)
                        joiningValues[d].push_back(oldValues[d][j]);
                }
            }
        }
    }

    joinedInfo.clear(); joinedValue.clear();
    int total = joiningValues[0].size();
    joinedValue.resize(total);
    for (auto& show: toShow) {
        int tableID = nameToRank[show.first], colID = AttrInfo::getPos(attrss.find(show.first)->second, show.second);
        assert(colID != -1);
        joinedInfo.push_back(joiningInfos[tableID][colID]);
        for (int i = 0; i < total; i++) {
            joinedValue[i].push_back(joiningValues[tableID][i][colID]);
        }
    }
}

bool QL_Manager::TableIsEmpty(const std::string& tbName) {
    printf("tablename %s\n", tbName.c_str());
    RM_FileHandle handle;
    RC rc = rmm.OpenFile(tbName.c_str(), handle); MUST_SUCC;
    RM_FileScan scan;
    rc = scan.OpenScan(handle); MUST_SUCC;
    RM_Record rec;
    rc = scan.GetNextRec(rec);
    bool isEmpty = (rc == RM_EOF);
    if (rc != RM_EOF) {
        MUST_SUCC;
    }
    rc = scan.CloseScan(); MUST_SUCC;
    rc = rmm.CloseFile(handle); MUST_SUCC;
    return isEmpty;
}