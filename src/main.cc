#include <cstdio>
#include "utils/utils.h"
#include "parser/parser.tab.h"

int main() {
    // InitDir("../data");
    chdir("../data");
    printf("DB initialized!\n");
    printf(">>>");
    yyparse();
}