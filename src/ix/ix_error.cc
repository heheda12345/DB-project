#include "ix_error.h"

#include <cstdio>

void IX_PrintError(RC rc) {
    return;
    switch (rc) {
        case IX_KEYNOTFOUND: { printf("ix error IX_KEYNOTFOUND\n"); break;}
        case IX_INVALIDSIZE: { printf("ix error IX_INVALIDSIZE\n"); break;}
        case IX_ENTRYEXISTS: { printf("ix error IX_ENTRYEXISTS\n"); break;}
        case IX_NOSUCHENTRY: { printf("ix error IX_NOSUCHENTRY\n"); break;}
        case IX_SIZETOOBIG: { printf("ix error IX_SIZETOOBIG\n"); break;}
        case IX_PF: { printf("ix error IX_PF\n"); break;}
        case IX_BADIXPAGE: { printf("ix error IX_BADIXPAGE\n"); break;}
        case IX_FCREATEFAIL: { printf("ix error IX_FCREATEFAIL\n"); break;}
        case IX_HANDLEOPEN: { printf("ix error IX_HANDLEOPEN\n"); break;}
        case IX_BADOPEN: { printf("ix error IX_BADOPEN\n"); break;}
        case IX_FNOTOPEN: { printf("ix error IX_FNOTOPEN\n"); break;}
        case IX_BADRID: { printf("ix error IX_BADRID\n"); break;}
        case IX_BADKEY: { printf("ix error IX_BADKEY\n"); break;}
        case IX_EOF: { printf("ix error IX_EOF\n"); break;}
        case IX_RM: { printf("ix error IX_RM\n"); break;}
        case IX_BTREE: { printf("ix error IX_BTREE\n"); break;}
        case IX_SCAN_OPENED: { printf("ix error IX_SCAN_OPENED\n"); break;}
        case IX_SCAN_CLOSED: { printf("ix error IX_SCAN_CLOSED\n"); break;}
        case IX_INVALIDCOMP: { printf("ix error IX_INVALIDCOMP\n"); break;}
        default: { printf("ix error %d\n", rc); }
    }
}