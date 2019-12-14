#pragma once
#include <string>
#include <vector>
#include "../redbase.h"
#include "../sm/sm.h"

namespace Parser {

class TreeNode {
};

class Type {
public:
    Type(AttrType _ty): ty(_ty), withi(false), vi(0) {}
    Type(AttrType _ty, int _vi): ty(_ty), withi(true), vi(_vi) {}
    // do not need dtor

    AttrType ty;
    bool withi;
    int vi;
};

class Value {
public:
    Value(int vi, bool isDate = false) {
        if (isDate) {
            dt.vd = vi;
            ty = DATE;
        } else {
            dt.vi = vi;
            ty = INT;
        }
    }

    Value(float vf) {
        dt.vf = vf;
        ty = FLOAT;
    }

    Value(std::string* vs) {
        dt.vs = vs;
        ty = STRING;
    }

    Value() {
        ty = NO_TYPE;
    }

    ~Value() {
        if (ty == STRING)
            delete dt.vs;
    }

    std::string dump() {
        switch (ty) {
            case INT: {
                return std::string(reinterpret_cast<char*>(&dt.vi), sizeof(int));
            }
            case DATE: {
                return std::string(reinterpret_cast<char*>(&dt.vi), sizeof(int));
            }
            case FLOAT: {
                return std::string(reinterpret_cast<char*>(&dt.vf), sizeof(float));
            }
            case STRING: {
                return std::string(*(dt.vs));
            }
            case NO_TYPE: {
                return std::string();
            }
        }
    }

    AttrType ty;
    union DT
    {
        int vi;
        int vd;
        float vf;
        std::string* vs;
    } dt;
};

class Table {
public:
    Table(std::string* _tbName): tbName(_tbName) {}
    ~Table() {
        delete tbName;
    }

    std::string* tbName;
};

class Column {
public:
    Column(std::string* _colName): colName(_colName) {}
    ~Column() {
        delete colName;
    }
    
    std::string* colName;
};

class Col {
public:
    Col(std::string* _colName): hasTb(false), colName(_colName) {}
    Col(std::string* _tbName, std::string* _colName): hasTb(true), tbName(_tbName), colName(_colName) {}
    ~Col() {
        if (hasTb) {
            delete tbName;
        }
        delete colName;
    }

    bool hasTb;
    std::string* tbName;
    std::string* colName;
};

class Expr {
public:
    Expr(Value* v) {
        ty = EXPR_VALUE;
        dt.vv = v;
    }
    Expr(Col* c) {
        ty = EXPR_COL;
        dt.vc = c;
    }
    ~Expr() {
        switch (ty) {
            case EXPR_VALUE: {
                delete dt.vv;
                break;
            }
            case EXPR_COL: {
                delete dt.vc;
                break;
            }
        }
    }


    enum Ty {
        EXPR_VALUE, EXPR_COL
    };
    union DT
    {
        Value* vv;
        Col* vc;
    };
    Ty ty;
    DT dt;
};

class Field: public TreeNode {
public:
    Field(std::string* _colName, Type* _type,
        bool _hasDefault, Value* _dVal, bool _notNull):
        colName(_colName), type(_type), 
        hasDefault(_hasDefault), dVal(_dVal), notNull(_notNull),
        ty(Simple), attr(_type->ty,  _type->vi, *_colName, _notNull, _hasDefault, _hasDefault ? _dVal->dump() : std::string()), hasError(0) {
            if (hasDefault) {
                if (dVal->ty != _type->ty) {
                    hasError = 1;
                    return;
                }
                if (dVal->ty == STRING) {
                    if (_type->withi && dVal->dt.vs->length() > _type->vi) {
                        hasError = 1;
                        return;
                    } else if (!_type->withi && dVal->dt.vs->length() > MAXSTRINGLEN) {
                        hasError = 1;
                        return;
                    }
                }
            }
        }

    Field(std::vector<Column*>* _columns): columns(_columns), ty(Primary), hasError(0) {}

    Field(std::string* _colName, std::string* _tbName, std::string* _othercol): colName(_colName), tbName(_tbName), othercol(_othercol), ty(Foreign), hasError(0) {}

    ~Field() {
        switch (ty) {
            case Simple: {
                delete colName;
                delete type;
                delete dVal;
                break;
            }
            case Primary: {
                for (auto c: *columns) {
                    delete c;
                }
                delete columns;
                break;
            }
            case Foreign: {
                delete colName;
                delete tbName;
                delete othercol;
                break;
            }
        }
    }

    enum FieldType {
        Simple, Primary, Foreign
    };
    
    std::string* colName;
    Type* type;
    Value* dVal;
    std::vector<Column*>* columns;
    std::string* tbName;
    std::string* othercol;
    bool hasDefault;
    bool notNull;
    FieldType ty;
    AttrInfo attr;
    bool hasError;
};

class WhereClause {
public:
    WhereClause(Col* _col, CompOp _op, Expr* _expr): ty(OP), col(_col), op(_op), expr(_expr) {}
    WhereClause(Col* _col, bool is_null): col(_col), ty(is_null ? IS_NULL : NOT_NULL) {}
    ~WhereClause() {
        switch (ty) {
            case OP: {
                delete col;
                delete expr;
                break;
            }
            case IS_NULL: {
                delete col;
                break;
            }
            case NOT_NULL: {
                delete col;
                break;
            }
        }
    }

    enum Ty { OP, IS_NULL, NOT_NULL };
    Ty ty;
    Col* col;
    CompOp op;
    Expr* expr;
};

class SetClause {
public:
    SetClause(std::string* _colName, Value* _value): colName(_colName), value(_value) {}
    ~SetClause() {
        delete colName;
        delete value;
    }

    std::string* colName;
    Value* value;
};

class Selector {
public:
    Selector(): exist(false) {}
    Selector(std::vector<Col*>* _colList): exist(true), colList(_colList) {}
    ~Selector() {
        if (exist) {
             for (auto c: *colList) {
                delete c;
            }
            delete colList;
        }
    }

    bool exist;
    std::vector<Col*>* colList;
};

class Stmt: public TreeNode {
public:
    void run() {
        visit();
    }
    virtual void visit() = 0;
};

class ShowDatabases: public Stmt {
public:
    ShowDatabases() {}
    ~ShowDatabases() {}

    void visit() override;
};

class CreateDatabase: public Stmt {
public:
    CreateDatabase(std::string* _dbName): dbName(_dbName) {}
    ~CreateDatabase() {
        delete dbName;
    }

    void visit() override;
    
    std::string* dbName;
};

class DropDatabase: public Stmt {
public:
    DropDatabase(std::string* _dbName): dbName(_dbName) {}
    ~DropDatabase() {
        delete dbName;
    }

    void visit() override;
    
    std::string* dbName;
};

class UseDatabase: public Stmt {
public:
    UseDatabase(std::string* _dbName): dbName(_dbName) {}
    ~UseDatabase() {
        delete[] dbName;
    }

    void visit() override;
    
    std::string *dbName;
};

class ShowTables: public Stmt {
public:
    ShowTables() = default;

    void visit() override;
};

class CreateTable: public Stmt {
public:
    CreateTable(std::string* _tbName, std::vector<Field*>* _fields): tbName(_tbName), fields(_fields) {}
    ~CreateTable() {
        delete tbName;
        for (auto x: *fields) {
            delete x;
        }
        delete fields;
    }

    void visit() override;

    std::string* tbName;
    std::vector<Field*>* fields;
};

class DropTable: public Stmt {
public:
    DropTable(std::string* _tbName): tbName(_tbName) {}
    ~DropTable() {
        delete tbName;
    }

    void visit() override;
    
    std::string* tbName;
};

class Desc: public Stmt {
public:
    Desc(std::string* _tbName): tbName(_tbName) {}
    ~Desc() {
        delete tbName;
    }

    void visit() override;
    
    std::string* tbName;
};

class DescTable: public Stmt {
public:
    DescTable(std::string* _tbName): tbName(_tbName) {}
    ~DescTable() {
        delete tbName;
    }

    void visit() override;

    std::string* tbName;
};

class InsertValue: public Stmt {
public:
    InsertValue(std::string* _tbName, std::vector<std::vector<Value*>*>* _values): tbName(_tbName), values(_values) {}
    ~InsertValue() {
        delete tbName;
        for (auto vv: *values) {
            for (auto v: *vv) {
                delete v;
            }
            delete vv;
        }
        delete values;
    }

    void visit() override;
    
    std::string* tbName;
    std::vector<std::vector<Value*>*>* values;
};

class DeleteValue: public Stmt {
public:
    DeleteValue(std::string* _tbName, std::vector<WhereClause*>* _wheres): tbName(_tbName), wheres(_wheres) {}
    ~DeleteValue() {
        delete tbName;
        for (auto w: *wheres) {
            delete w;
        }
        delete wheres;
    }

    void visit() override;
    
    std::string* tbName;
    std::vector<WhereClause*>* wheres;
};

class UpdateValue: public Stmt {
public:
    UpdateValue(std::string* _tbName,
        std::vector<SetClause*>* _sets,
        std::vector<WhereClause*>* _wheres):
            tbName(_tbName), sets(_sets), wheres(_wheres) {}
    ~UpdateValue() {
        delete tbName;
        for (auto s: *sets) {
            delete s;
        }
        for (auto w: *wheres) {
            delete w;
        }
        delete wheres;
    }

    void visit() override;

    std::string* tbName;
    std::vector<SetClause*>* sets;
    std::vector<WhereClause*>* wheres;
};

class SelectValue: public Stmt {
public:
    SelectValue(Selector* _selector, std::vector<Table*>* _tables,
    std::vector<WhereClause*>* _wheres): selector(_selector), tables(_tables), wheres(_wheres) {}
    ~SelectValue() {
        delete selector;
        for (auto t: *tables) {
            delete t;
        }
        delete tables;
        for (auto w: *wheres) {
            delete w;
        }
        delete wheres;
    }

    void visit() override;

    Selector* selector;
    std::vector<Table*>* tables;
    std::vector<WhereClause*>* wheres;
};

class AddIndex: public Stmt {
public:
    AddIndex(std::string* _tbName, std::string* _idxName, std::vector<Column*>* _columns): tbName(_tbName), idxName(_idxName), columns(_columns) {}
    ~AddIndex() {
        delete tbName;
        delete idxName;
        for (auto c: *columns) {
            delete c;
        }
        delete columns;
    }

    void visit() override;

    std::string* tbName;
    std::string* idxName;
    std::vector<Column*>* columns;
};

class DropIndex: public Stmt {
public:
    DropIndex(std::string* _tbName, std::string* _idxName): tbName(_tbName), idxName(_idxName) {}
    ~DropIndex() {
        delete tbName;
        delete idxName;
    }

    void visit() override;
    std::string* tbName;
    std::string* idxName;
};

class AddField: public Stmt {
public:
    AddField(std::string* _tbName, Field* _field): tbName(_tbName), field(_field) {}
    ~AddField() {
        delete tbName;
        delete field;
    }

    void visit() override;

    std::string* tbName;
    Field* field;
};

class DropCol: public Stmt {
public:
    DropCol(std::string* _tbName, std::string* _colName): tbName(_tbName), colName(_colName) {}
    ~DropCol() {
        delete tbName;
        delete colName;
    }

    void visit() override;

    std::string* tbName;
    std::string* colName;
};

class ChangeCol: public Stmt {
public:
    ChangeCol(std::string* _tbName, std::string* _colName, Field* _field): tbName(_tbName), colName(_colName), field(_field) {}
    ~ChangeCol() {
        delete tbName;
        delete colName;
        delete field;
    }

    void visit() override;

    std::string* tbName;
    std::string* colName;
    Field* field;
};

class DropPrimaryKey: public Stmt {
public:
    DropPrimaryKey(std::string* _tbName): tbName(tbName) {}
    ~DropPrimaryKey() {
        delete tbName;
    }

    void visit() override;
    
    std::string* tbName;
};

class AddForeignKey: public Stmt {
public:
    AddForeignKey(std::string* _tbName, std::string* _fkName, std::vector<Column*>* _srcCols, std::string* _refTable, std::vector<Column*>* _refCols) : tbName(_tbName), fkName(_fkName), refTable(_refTable), srcCols(_srcCols), refCols(_refCols) {}
    ~AddForeignKey() {
        delete tbName;
        delete fkName;
        delete refTable;
        for (auto c: *srcCols) {
            delete c;
        }
        delete srcCols;
        for (auto c: *refCols) {
            delete c;
        }
        delete refCols;        
    }

    void visit() override;

    std::string *tbName, *fkName, *refTable;
    std::vector<Column*> *srcCols, *refCols;
};

class DropForeignKey: public Stmt {
public:
    DropForeignKey(std::string* _tbName, std::string* _fkName) : tbName(_tbName), fkName(_fkName) {}
    ~DropForeignKey() {
        delete tbName;
        delete fkName; 
    }

    void visit() override;

    std::string *tbName, *fkName;
};

}