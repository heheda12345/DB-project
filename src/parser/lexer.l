%option caseless

%{
#include "parser.tab.h"
extern "C"
{
	int yywrap(void);
	int yylex(void);
}
%}

IDENTIFIER [A-Za-z][_0-9A-Za-z]*
FORMAT_INT [0-9]+
FORMAT_DATE [0-9]{4}-[0-9]{1,2}-[0-9]{1,2}
FORMAT_FLOAT [0-9]+.[0-9]+
FORMAT_STRING '[^']*'

%%

DATABASE { return DATABASE; }
DATABASES { return DATABASES; }
TABLE { return TABLE; }
TABLES { return TABLES; }
SHOW { return SHOW; }
CREATE { return CREATE; }
DROP { return DROP; }
USE { return USE; }
PRIMARY { return PRIMARY; }
KEY { return KEY; }
NOT { return NOT; }
NULL { return VALUE_NULL; }
INSERT { return INSERT; }
INTO { return INTO; }
VALUES { return VALUES; }
DELETE { return DELETE; }
FROM { return FROM; }
WHERE { return WHERE; }
UPDATE { return UPDATE; }
SET { return SET; }
SELECT { return SELECT; }
IS { return IS; }
INT { return TYPE_INT; }
VARCHAR { return TYPE_VARCHAR; }
CHAR { return TYPE_CHAR; }
DEFAULT { return DEFAULT; }
CONSTRAINT { return CONSTRAINT; }
CHANGE { return CHANGE; }
ALTER { return ALTER; }
ADD { return ADD; }
RENAME { return RENAME; }
DESC { return DESC; }
REFERENCES { return REFERENCES; }
INDEX { return INDEX; }
UNIQUE { return UNIQUE; }
ON { return ON; }
AND { return AND; }
DATE { return TYPE_DATE; }
FLOAT { return TYPE_FLOAT; }
FOREIGN { return FOREIGN; }
TO { return TO; }
EXIT { return EXIT; }
"," { return yytext[0]; }
"(" { return yytext[0]; }
")" { return yytext[0]; }
"." { return yytext[0]; }
";" { return yytext[0]; }
"+" { return yytext[0]; }
"-" { return yytext[0]; }
"*" { return yytext[0]; }
"/" { return yytext[0]; }
"=" { return EQ; }
"<>" { return NE; }
"<=" { return LE; }
">=" { return GE; }
"<" { return LT; }
">" { return GT; }
{IDENTIFIER} { yylval.strval = new std::string(yytext, yyleng); return IDENTIFIER; }
{FORMAT_INT} { sscanf(yytext, "%d", &yylval.intval); return VALUE_INT; }
{FORMAT_FLOAT} { sscanf(yytext, "%f", &yylval.floatval); return VALUE_FLOAT; }
{FORMAT_STRING} { yylval.strval = new std::string(yytext + 1, yyleng-2); return VALUE_STRING; }
{FORMAT_DATE} {
    int year, month, date;
    sscanf(yytext, "%d-%d-%d", &year, &month, &date);
    yylval.intval = year * 10000 + month * 100 + date;
    return VALUE_DATE;
}
%%

int yywrap() {
    return 1;
}