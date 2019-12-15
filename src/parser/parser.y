%{
#include <stdio.h>
#include <vector>
#include "tree.h"

extern "C"
{
    int yylex();
    void yyerror(const char *);
}
using namespace Parser;
using namespace std;
%}


%code requires {
#include <string.h>
#include "../redbase.h"
#include "tree.h"
}

%union {
    int intval;
    float floatval;
    std::string* strval;
    CompOp op;
    Parser::TreeNode* treeNode;
    Parser::Stmt* stmt;
    Parser::Field* field;
    std::vector<Parser::Field*>* fieldList;
    Parser::Type* type;
    Parser::Value* value;
    std::vector<Parser::Value*>* valueList;
    std::vector<std::vector<Parser::Value*>*>* valueLists;
    Parser::WhereClause* whereClause;
    std::vector<Parser::WhereClause*>* whereClauseList;
    Parser::Col* col;
    std::vector<Parser::Col*>* colList;
    Parser::Expr* expr;
    Parser::SetClause* setClause;
    std::vector<Parser::SetClause*>* setClauseList;
    Parser::Selector* selector;
    Parser::Table* table;
    std::vector<Parser::Table*>* tableList;
    Parser::Column* column;
    std::vector<Parser::Column*>* columnList;
}

%token DATABASE DATABASES TABLE TABLES SHOW CREATE DROP USE PRIMARY KEY NOT VALUE_NULL INSERT INTO VALUES DELETE FROM WHERE UPDATE SET SELECT IS TYPE_INT TYPE_CHAR TYPE_VARCHAR DEFAULT CONSTRAINT CHANGE ALTER ADD RENAME DESC REFERENCES INDEX UNIQUE ON AND TYPE_DATE TYPE_FLOAT FOREIGN EQ NE LT GT LE GE TO EXIT

%token <intval> VALUE_INT VALUE_DATE
%token <floatval> VALUE_FLOAT
%token <strval> VALUE_STRING IDENTIFIER
%type <treeNode> program
%type <stmt> stmt sysStmt dbStmt tbStmt idxStmt alterStmt
%type <fieldList> fieldList
%type <field> field
%type <type> type
%type <valueLists> valueLists
%type <valueList> valueList
%type <value> value
%type <whereClause> whereClause
%type <whereClauseList> whereClauseList
%type <op> op
%type <col> col
%type <expr> expr
%type <setClauseList> setClauseList
%type <selector> selector
%type <colList> colList
%type <tableList> tableList
%type <columnList> columnList
%type <strval> dbName tbName colName idxName

%%

program: program stmt
    {
        $2->run();
        delete $2;
    }
    | /* empty */
    {

    }

stmt: sysStmt ';'
    {
        $$ = $1;
    }
    | dbStmt ';'
    {
        $$ = $1;
    }
    | tbStmt ';'
    {
        $$ = $1;
    }
    | idxStmt ';'
    {
        $$ = $1;
    }
    | alterStmt ';'
    {
        $$ = $1;
    }

sysStmt: SHOW DATABASES
    {
        $$ = new ShowDatabases();
    }
    | EXIT
    {
        printf("DB end!\n");
        YYACCEPT;
    }

dbStmt: CREATE DATABASE dbName
    {
        $$ = new CreateDatabase($3);
    }
    | DROP DATABASE dbName
    {
        $$ = new DropDatabase($3);
    }
    | USE DATABASE dbName
    {
        $$ = new UseDatabase($3);
    }
    | SHOW TABLES
    {
        $$ = new ShowTables();
    }

tbStmt: CREATE TABLE tbName '(' fieldList ')'
    {
        $$ = new CreateTable($3, $5);
    }
    | DROP TABLE tbName
    {
        $$ = new DropTable($3);
    }
    | DESC tbName
    {
        $$ = new Desc($2);
    }
    | INSERT INTO tbName VALUES valueLists
    {
        $$ = new InsertValue($3, $5);
    }
    | DELETE FROM tbName WHERE whereClauseList
    {
        $$ = new DeleteValue($3, $5);
    }
    | UPDATE tbName SET setClauseList WHERE whereClauseList
    {
        $$ = new UpdateValue($2, $4, $6);
    }
    | SELECT selector FROM tableList WHERE whereClauseList
    {
        $$ = new SelectValue($2, $4, $6);
    }

idxStmt: ALTER TABLE tbName ADD INDEX idxName '(' columnList ')'
    {
        $$ = new AddIndex($3, $6, $8);
    }
    | ALTER TABLE tbName DROP INDEX idxName
    {
        $$ = new DropIndex($3, $6);
    }

alterStmt: ALTER TABLE tbName ADD field
    {
        $$ = new AddField($3, $5);
    }
	| ALTER TABLE tbName DROP colName
    {
        $$ = new DropCol($3, $5);
    } 
	| ALTER TABLE tbName CHANGE colName field
    {
        $$ = new ChangeCol($3, $5, $6);
    }
	| ALTER TABLE tbName DROP PRIMARY KEY
    {
        $$ = new DropPrimaryKey($3);
    }
    | ALTER TABLE tbName ADD CONSTRAINT idxName FOREIGN KEY '(' columnList ')' REFERENCES tbName '(' columnList ')'
    {
        $$ = new AddForeignKey($3, $6, $10, $13, $15);
    }
    | ALTER TABLE tbName DROP FOREIGN KEY idxName
    {
        $$ = new DropForeignKey($3, $7);
    }
    | ALTER TABLE tbName ADD CONSTRAINT idxName UNIQUE KEY '(' columnList ')'
    {
        $$ = new AddUniqueKey($3, $6, $10);
    }
    | ALTER TABLE tbName DROP UNIQUE KEY idxName
    {
        $$ = new DropUniqueKey($3, $7);
    }

fieldList: field
    {
        $$ = new vector<Field*>();
        $$->push_back($1);
    }
    | fieldList ',' field
    {
        $$ = $1;
        $$->push_back($3);
    }

field: colName type
    {
        $$ = new Field($1, $2, false, nullptr, false);
    }
    | colName type NOT VALUE_NULL
    {
        $$ = new Field($1, $2, false, nullptr, true);
    }
    | colName type DEFAULT value
    {
        $$ = new Field($1, $2, true, $4, false);
    }
    | colName type NOT VALUE_NULL DEFAULT value
    {
        $$ = new Field($1, $2, true, $6, true);
    }
    | PRIMARY KEY '(' columnList ')'
    {
        $$ = new Field($4);
    }
    | FOREIGN KEY '(' colName ')' REFERENCES tbName '(' colName ')'
    {
        $$ = new Field($4, $7, $9);
    }

type: TYPE_INT
    {
        $$ = new Type(INT);
    }
    | TYPE_VARCHAR
    {
        $$ = new Type(STRING);
        $$->setCanChange();
    }
    | TYPE_VARCHAR '(' VALUE_INT ')'
    {
        $$ = new Type(STRING, $3);
        $$->setCanChange();
    }
    | TYPE_CHAR
    {
        $$ = new Type(STRING);
    }
    | TYPE_CHAR '(' VALUE_INT ')'
    {
        $$ = new Type(STRING, $3);
    }
    | TYPE_DATE
    {
        $$ = new Type(DATE);
    }
    | TYPE_FLOAT
    {
        $$ = new Type(FLOAT);
    }

valueLists: '(' valueList ')'
    {
        $$ = new vector<vector<Value*>*>();
        $$->push_back($2);
    }
    | valueLists ',' '(' valueList ')'
    {
        $$ = $1;
        $$->push_back($4);
    }

valueList: value
    {
        $$ = new vector<Value*>();
        $$->push_back($1);
    }
    | valueList ',' value
    {
        $$ = $1;
        $$->push_back($3);
    }

value: VALUE_INT
    {
        $$ = new Value($1);
    }
    | VALUE_FLOAT
    {
        $$ = new Value($1);
    }
    | VALUE_DATE
    {
        $$ = new Value($1, true);
    }
    | VALUE_STRING
    {
        $$ = new Value($1);
    }
    | VALUE_NULL
    {
        $$ = new Value();
    }

whereClause: col op expr
    {
        $$ = new WhereClause($1, $2, $3);
    }
    | col IS NOT VALUE_NULL
    {
        $$ = new WhereClause($1, false);
    }
    | col IS VALUE_NULL
    {
        $$ = new WhereClause($1, true);
    }

whereClauseList: whereClauseList AND whereClause
    {
        $$ = $1;
        $$->push_back($3);
    }
    | whereClause
    {
        $$ = new vector<WhereClause*>();
        $$->push_back($1);
    }

op:   EQ { $$ = EQ_OP; }
    | NE { $$ = NE_OP; } 
    | GT { $$ = GT_OP; }
    | LT { $$ = LT_OP; }
    | GE { $$ = GE_OP; }
    | LE { $$ = LE_OP; }

col: tbName '.' colName
    {
        $$ = new Col($1, $3);
    }
    | colName
    {
        $$ = new Col($1);
    }

expr: value
    {
        $$ = new Expr($1);
    }
    | col
    {
        $$ = new Expr($1);
    }
    

setClauseList: colName EQ value
    {
        $$ = new vector<SetClause*>;
        $$->push_back(new SetClause($1, $3));
    }
    | setClauseList ',' colName EQ value
    {
        $$ = $1;
        $$->push_back(new SetClause($3, $5));
    }

selector: '*'
    {
        $$ = new Selector();
    }
    | colList
    {
        $$ = new Selector($1);
    }

colList: col
    {
        $$ = new vector<Col*>();
        $$->push_back($1);
    }
    | colList ',' col
    {
        $$ = $1;
        $$->push_back($3);
    }

tableList: tbName
    {
        $$ = new vector<Table*>();
        $$->push_back(new Table($1));
    }
    | tableList ',' tbName
    {
        $$ = $1;
        $$->push_back(new Table($3));
    }

columnList: colName
    {
        $$ = new vector<Column*>();
        $$->push_back(new Column($1));
    }
    | columnList ',' colName
    {
        $$ = $1;
        $$->push_back(new Column($3));
    }

dbName: IDENTIFIER { $$ = $1; }

tbName: IDENTIFIER { $$ = $1; }

colName: IDENTIFIER { $$ = $1; }

idxName: IDENTIFIER { $$ = $1; }
%%

void yyerror(const char* s) {
    printf("yyerror %s\n", s);
}
