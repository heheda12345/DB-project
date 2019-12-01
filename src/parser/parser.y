%{
#include <stdio.h>
extern "C"
{
    int yylex();
    void yyerror(const char *);
}
%}


%code requires {
#include "../redbase.h"
#include "../utils/simpleString.h"
}

%union {
    int intval;
    float floatval;
    SimpleString strval;
    CompOp op;
}

%token DATABASE DATABASES TABLE TABLES SHOW CREATE DROP USE PRIMARY KEY NOT VALUE_NULL INSERT INTO VALUES DELETE FROM WHERE UPDATE SET SELECT IS TYPE_INT TYPE_STRING DEFAULT CONSTRAINT CHANGE ALTER ADD RENAME DESC REFERENCES INDEX ON AND TYPE_DATE TYPE_FLOAT FOREIGN EQ NE LT GT LE GE IDENTIFIER TO VALUE_INT VALUE_DATE VALUE_FLOAT VALUE_STRING

%%

program: program stmt
    {

    }
    | /* empty */
    {

    }

stmt: sysStmt ';'
    {

    }
    | dbStmt ';'
    {

    }
    | tbStmt ';'
    {

    }
    | idxStmt ';'
    {

    }
    | alterStmt ';'
    {

    }

sysStmt: SHOW DATABASES
    {

    }

dbStmt: CREATE DATABASE dbName
    {

    }
    | DROP DATABASE dbName
    {

    }
    | USE dbName
    {

    }
    | SHOW TABLES
    {

    }
tbStmt: CREATE TABLE tbName '(' fieldList ')'
    {

    }
    | DROP TABLE tbName
    {

    }
    | DESC tbName
    {

    }
    | INSERT INTO tbName VALUES valueLists
    {

    }
    | DELETE FROM tbName WHERE whereClauseList
    {

    }
    | UPDATE tbName SET setClause WHERE whereClauseList
    {

    }
    | SELECT selector FROM tableList WHERE whereClauseList
    {

    }

idxStmt: CREATE INDEX idxName ON tbName '(' columnList ')'
    {

    }
    | DROP INDEX idxName
    {

    }
    | ALTER TABLE tbName ADD INDEX idxName '(' columnList ')'
    {

    }
    | ALTER TABLE tbName DROP INDEX idxName
    {

    }

alterStmt: ALTER TABLE tbName ADD field
    {

    }
	| ALTER TABLE tbName DROP colName
    {

    } 
	| ALTER TABLE tbName CHANGE colName field
    {

    }
	| ALTER TABLE tbName RENAME TO tbName
    {

    }
	| ALTER TABLE tbName DROP PRIMARY KEY
    {

    }

fieldList: field
    {

    }
    | fieldList ',' field
    {

    }

field: colName type
    {

    }
    | colName type NOT VALUE_NULL
    {

    }
    | colName type DEFAULT value
    {

    }
    | colName type NOT VALUE_NULL DEFAULT value
    {

    }
    | PRIMARY KEY '(' columnList ')'
    {

    }
    | FOREIGN KEY '(' colName ')' REFERENCES tbName '(' colName ')'
    {

    }

type: TYPE_INT
    {

    }
    | TYPE_INT VALUE_INT
    {

    }
    | TYPE_STRING
    {

    }
    | TYPE_STRING VALUE_INT
    {

    }
    | TYPE_DATE
    {

    }
    | TYPE_FLOAT
    {

    }

valueLists: '(' valueList ')'
    {

    }
    | valueLists ',' '(' valueList ')'
    {

    }

valueList: value
    {

    }
    | valueList ',' value
    {

    }

value: VALUE_INT
    {

    }
    | VALUE_FLOAT
    {

    }
    | VALUE_DATE
    {

    }
    | VALUE_STRING
    {

    }
    | VALUE_NULL
    {

    }

whereClause: col op expr
    {

    }
    | col IS NOT VALUE_NULL
    {

    }
    | col IS VALUE_NULL
    {

    }

whereClauseList: whereClauseList AND whereClause
    {

    }
    | whereClause
    {

    }

op:   EQ { }
    | NE { } 
    | GT { }
    | LT { }
    | GE { }
    | LE { }

col: tbName '.' colName
    {

    }
    | colName
    {

    }

expr: value
    {

    }
    | col
    {

    }
    

setClause: colName EQ value
    {

    }
    | setClause ',' colName EQ value
    {

    }

selector: '*'
    {

    }
    | colList
    {

    }

colList: col
    {

    }
    | colList ',' col
    {

    }

tableList: tbName
    {

    }
    | tableList ',' tbName
    {

    }

columnList: colName
    {

    }
    | columnList ',' colName
    {

    }

dbName: IDENTIFIER {

}

tbName: IDENTIFIER {
    
}

colName: IDENTIFIER {

}

idxName: IDENTIFIER {

}
%%

void yyerror(const char* s) {
    printf("yyerror %s\n", s);
}
