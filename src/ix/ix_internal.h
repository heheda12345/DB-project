#pragma once

struct IX_Node {

};

struct IX_Util {
    static int recSize(int attrLen) {
        return attrLen + sizeof(IX_Node);
    } 
};