#pragma once
#include <string>
#include <vector>
#include "../redbase.h"
#include "../sm/sm.h"
#include "../ql/ql.h"
#include "../utils/utils.h"

namespace Parser {

class TreeNode {
};

class Type {
public:
    Type(AttrType _ty): ty(_ty), withi(false), vi(0), canChange(0) {}
    Type(AttrType _ty, int _vi): ty(_ty), withi(true), vi(_vi), canChange(0) {}
    void setCanChange() {
        canChange = 1;
    }
    // do not need dtor

    AttrType ty;
    bool withi;
    int vi;
    bool canChange;
};

class Value {
public:
    Value(int vi, bool isDate = false): hasError(0) {
        if (isDate) {
            dt.vd = vi;
            ty = DATE;
            if (!isLegalDate(vi))
                hasError = 1;
        } else {
            dt.vi = vi;
            ty = INT;
        }
    }

    Value(float vf): hasError(0) {
        dt.vf = vf;
        ty = FLOAT;
    }

    Value(std::string* vs): hasError(0) {
        dt.vs = vs;
        ty = STRING;
    }

    Value(): hasError(0) {
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
                assert(isLegalDate(dt.vi));
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

    Item toItem() {
        Item item;
        item.isNull = ty == NO_TYPE;
        item.type = ty;
        item.value = dump();
        return item;
    }

    AttrType ty;
    union DT
    {
        int vi;
        int vd;
        float vf;
        std::string* vs;
    } dt;
    bool hasError;
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
    std::string getTbName() {
        return hasTb? *tbName : std::string();
    }
    std::string getColName() {
        return *colName;
    }

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
        ty(Simple), attr(_type->ty,  _type->vi, *_colName, _notNull, _hasDefault, _type->canChange, _hasDefault ? _dVal->dump() : std::string()), hasError(0) {
            if (hasDefault) {
                if (dVal->ty != _type->ty) {
                    hasError = 1;
                    return;
                }
                if (dVal->ty == STRING) {
                    if (_type->withi) {
                        if ( _type->vi > MAXSTRINGLEN) {
                            hasError = 1;
                            return;
                        }
                        if (dVal->dt.vs->length() > _type->vi) {
                            hasError = 1;
                            return;
                        }
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
    WhereClause(Col* _col, CompOp _op, Expr* _expr): ty(OP), col(_col), op(_op), expr(_expr) {
        singleWhere.hasError = dualWhere.hasError = 0;
        if (expr->ty == Expr::EXPR_VALUE) {
            inSingle = 1;
            singleWhere.whereType = SingleWhere::TY_OP_VALUE;
            Value* v = _expr->dt.vv;
            if (v->hasError)
                singleWhere.hasError = 1;
            if (v->ty == NO_TYPE)
                singleWhere.hasError = 1;
            singleWhere.ty = v->ty;
            singleWhere.idx1 = col->getColName();
            singleWhere.value = v->dump();
            singleWhere.tbName = col->getTbName();
            singleWhere.op = _op;
        } else {
            Col* col2 = _expr->dt.vc;
            if (col->getTbName() == col2->getTbName()) {
                inSingle = 1;
                singleWhere.whereType = SingleWhere::TY_OP_COL;
                singleWhere.idx1 = col->getColName();
                singleWhere.idx2 = col2->getColName();
                singleWhere.tbName = col->getTbName();
                singleWhere.op = _op;
            } else {
                inSingle = 0;
                if (col->getTbName() == "" || col2->getTbName() == "")
                    dualWhere.hasError = 1;
                if (_op != EQ_OP)
                    dualWhere.hasError = 1;
                dualWhere.tbName1 = col->getTbName();
                dualWhere.idx1 = col->getColName();
                dualWhere.tbName2 = col2->getTbName();
                dualWhere.idx2 = col2->getColName();
            }
        }
    }
    WhereClause(Col* _col, bool is_null): col(_col), ty(is_null ? IS_NULL : NOT_NULL) {
        singleWhere.hasError = 0;
        singleWhere.whereType = is_null ? SingleWhere::TY_IS_NULL : SingleWhere::TY_NOT_NULL;
        singleWhere.idx1 = col->getColName();
        inSingle = 1;
    }
    WhereClause(Col* _col, std::string* _pattern): col(_col), pattern(_pattern) {
        singleWhere.hasError = 0;
        singleWhere.whereType = SingleWhere::TY_LIKE;
        singleWhere.idx1 = col->getColName();
        singleWhere.value = *pattern;
        inSingle = 1;
    }
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

    enum Ty { OP, IS_NULL, NOT_NULL, LIKE };
    Ty ty;
    Col* col;
    CompOp op;
    Expr* expr;
    std::string* pattern;
    bool inSingle;
    RawSingleWhere singleWhere;
    RawDualWhere dualWhere;
};

class SetNode {
public:
    SetNode(Value* _value) {
        ty = ExprNode::VALUE;
        value = _value;
        hasError = _value->hasError;

        node.nodeType = ty;
        node.str = _value->dump();
        node.hasError = hasError;
        node.attrType = _value->ty;
    }
    SetNode(std::string* _col) {
        ty = ExprNode::COL;
        hasError = 0;
        colName = _col;

        node.nodeType = ty;
        node.str = *colName;
        node.hasError = hasError;
        node.attrType = NO_TYPE;
    }
    SetNode(SetNode* l, ExprNode::ExprNodeType _ty, SetNode* r) {
        ty = _ty;
        assert(ty == ExprNode::ADD || ty == ExprNode::SUB ||
               ty == ExprNode::MUL || ty == ExprNode::DIV);
        sonL = l, sonR = r;
        hasError = l->hasError | r->hasError;

        node.nodeType = ty;
        node.hasError = hasError;
        node.attrType = NO_TYPE;
        node.sons.clear();
        node.sons.push_back(l->node);
        node.sons.push_back(r->node);
    }
    ~SetNode() {
        switch (ty) {
            case ExprNode::VALUE: {
                delete value;
                break;
            }
            case ExprNode::COL: {
                delete colName;
                break;
            }
            case ExprNode::ADD: // no break
            case ExprNode::SUB: // no break
            case ExprNode::MUL: // no break;
            case ExprNode::DIV: {
                delete sonL;
                delete sonR;
            }
        }
    }

    ExprNode::ExprNodeType ty;
    bool hasError;

    std::string* colName;
    Value* value;
    SetNode *sonL;
    SetNode *sonR;
    RawExprNode node;
};

class SetClause {
public:
    SetClause(std::string* _colName, SetNode* _node): colName(_colName), node(_node) {}
    ~SetClause() {
        delete colName;
        delete node;
    }

    std::string* colName;
    SetNode* node;
};

class Selector {
public:
    enum Ty {
        ALL, PART, GATHER
    };
    
    Selector(): ty(ALL), gOp(NO_GOP) {}
    Selector(std::vector<Col*>* _colList): ty(PART), colList(_colList), gOp(NO_GOP) {
        for (auto &col: *colList) {
            attrNames.push_back(make_pair(col->getTbName(), col->getColName()));
        }
    }
    Selector(Col* _col, GatherOp _op): ty(GATHER), col(_col), gOp(_op) {
        attrNames.push_back(make_pair(col->getTbName(), col->getColName()));
    } 

    ~Selector() {
        if (ty == PART) {
             for (auto c: *colList) {
                delete c;
            }
            delete colList;
        } else {
            if (ty == GATHER) {
                delete col;
            }
        }
    }
    
    Ty ty;
    Col* col;
    std::vector<Col*>* colList;
    std::vector<RawTbAttr> attrNames;
    GatherOp gOp;
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
    InsertValue(std::string* _tbName, std::vector<Value*>* _values): tbName(_tbName), values(_values) {}
    ~InsertValue() {
        delete tbName;
        for (auto v: *values) {
            delete v;
        }
        delete values;
    }

    void visit() override;
    
    std::string* tbName;
    std::vector<Value*>* values;
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

class CopyFrom: public Stmt {
public:
    CopyFrom(std::string* _tbName, std::string* _path): tbName(_tbName), path(_path) {}
    ~CopyFrom() {
        delete tbName;
        delete path;
    }

    void visit() override;

    std::string* tbName;
    std::string* path;
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
    DropPrimaryKey(std::string* _tbName): tbName(_tbName) {}
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

class AddUniqueKey: public Stmt {
public:
    AddUniqueKey(std::string* _tbName, std::string* _fkName, std::vector<Column*>* _cols) : tbName(_tbName), fkName(_fkName), cols(_cols) {}
    ~AddUniqueKey() {
        delete tbName;
        delete fkName;
        for (auto c: *cols) {
            delete c;
        }        
    }

    void visit() override;

    std::string *tbName, *fkName;
    std::vector<Column*> *cols;
};

class DropUniqueKey: public Stmt {
public:
    DropUniqueKey(std::string* _tbName, std::string* _fkName) : tbName(_tbName), fkName(_fkName) {}
    ~DropUniqueKey() {
        delete tbName;
        delete fkName; 
    }

    void visit() override;

    std::string *tbName, *fkName;
};

class RenameTable: public Stmt {
public:
    RenameTable(std::string* _oldName, std::string* _newName): oldName(_oldName), newName(_newName) {}
    ~RenameTable() {
        delete oldName;
        delete newName;
    }

    void visit() override;

    std::string *oldName, *newName;
};

}