#include "sm.h"

SM_Manager::SM_Manager(IX_Manager &ixm, RM_Manager &rmm) : rmm(rmm), ixm(ixm) {

}

SM_Manager::~SM_Manager() {

}

RC SM_Manager::OpenDb(const char *dbName) {
    return OK_RC;
}

RC SM_Manager::CloseDb() {
    return OK_RC;
}

RC CreateTable(const char *relName, int attrCount, AttrInfo   *attributes) {
    return OK_RC;
}

RC CreateIndex(const char *relName,  const char *attrName) {
    return OK_RC;
}

RC DropTable(const char *relName) {
    return OK_RC;
}

RC DropIndex (const char *relName, const char *attrName) {
    return OK_RC;
}

RC Load(const char *relName, const char *fileName) {
    return OK_RC;
}

RC Help() {
    return OK_RC;
}

RC Help(const char *relName) {
    return OK_RC;
}

RC Print(const char *relName) {
    return OK_RC;
}

RC Set(const char *paramName, const char *value) {
    return OK_RC;
}