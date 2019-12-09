#include "rm.h"

void RM_PrintError(RC rc) {
    switch (rc) {
        case RM_RECORD_GETDATA: { printf("rm error RM_RECORD_GETDATA\n"); break;}
        case RM_RECORD_GETRID: { printf("rm error RM_RECORD_GETRID\n"); break;}
        case RM_FILEHANDLE_GETREC: { printf("rm error RM_FILEHANDLE_GETREC\n"); break;}
        case RM_FILEHANDLE_INSERTREC: { printf("rm error RM_FILEHANDLE_INSERTREC\n"); break;}
        case RM_FILEHANDLE_DELETEREC: { printf("rm error RM_FILEHANDLE_DELETEREC\n"); break;}
        case RM_FILEHANDLE_UPDATEREC: { printf("rm error RM_FILEHANDLE_UPDATEREC\n"); break;}
        case RM_FILEHANDLE_FORCEPAGES: { printf("rm error RM_FILEHANDLE_FORCEPAGES\n"); break;}
        case RM_FILEHANDLE_OPEN: { printf("rm error RM_FILEHANDLE_OPEN\n"); break;}
        case RM_FILEHANDLE_CLOSE: { printf("rm error RM_FILEHANDLE_CLOSE\n"); break;}
        case RM_FILEHANDLE_CHECKRID: { printf("rm error RM_FILEHANDLE_CHECKRID\n"); break;}
        case RM_FILEHANDLE_GETFIRSTREC: { printf("rm error RM_FILEHANDLE_GETFIRSTREC\n"); break;}
        case RM_FILEHANDLE_GETNEXTREC: { printf("rm error RM_FILEHANDLE_GETNEXTREC\n"); break;}
        case RM_FILEHANDLE_SETMETA: { printf("rm error RM_FILEHANDLE_SETMETA\n"); break;}
        case RM_FILEHANDLE_GETMETA: { printf("rm error RM_FILEHANDLE_GETMETA\n"); break;}
        case RM_MANAGER_CREATEFILE: { printf("rm error RM_MANAGER_CREATEFILE\n"); break;}
        case RM_MANAGER_DESTROYFILE: { printf("rm error RM_MANAGER_DESTROYFILE\n"); break;}
        case RM_MANAGER_OPENFILE: { printf("rm error RM_MANAGER_OPENFILE\n"); break;}
        case RM_MANAGER_CLOSEFILE: { printf("rm error RM_MANAGER_CLOSEFILE\n"); break;}
        case RM_FILESCAN_OPENSCAN: { printf("rm error RM_FILESCAN_OPENSCAN\n"); break;}
        case RM_FILESCAN_GETNEXTREC: { printf("rm error RM_FILESCAN_GETNEXTREC\n"); break;}
        case RM_FILESCAN_CLOSESCAN: { printf("rm error RM_FILESCAN_CLOSESCAN\n"); break;}
        case RM_NEW_WARN_START: { printf("rm error RM_NEW_WARN_START\n"); break;}
        case RM_NEW_ERROR_START: { printf("rm error RM_NEW_ERROR_START\n"); break;}
        case RM_WARN_EMPTY_RECORD: { printf("rm error RM_WARN_EMPTY_RECORD\n"); break;}
        case RM_EOF: { printf("rm error RM_EOF\n"); break;}
        case RM_FILE_IS_OPEN: { printf("rm error RM_FILE_IS_OPEN\n"); break;}
        case RM_FILE_NOT_OPEN: { printf("rm error RM_FILE_NOT_OPEN\n"); break;}
        case RM_SLOT_OUTOFRANGE: { printf("rm error RM_SLOT_OUTOFRANGE\n"); break;}
        case RM_NO_SUCH_REC: { printf("rm error RM_NO_SUCH_REC\n"); break;}
        case RM_NOT_EMPTY_REC: { printf("rm error RM_NOT_EMPTY_REC\n"); break;}
        case RM_SCAN_NOT_OPEN: { printf("rm error RM_SCAN_NOT_OPEN\n"); break;}
        case RM_SCAN_NOT_CLOSE: { printf("rm error RM_SCAN_NOT_CLOSE\n"); break;}
        default: { printf("rm error %d\n", rc); }
    }
}