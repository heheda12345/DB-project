#pragma once
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string>

bool DirExist(const char* dir);
void InitDir(const char* dir);
int TryMkDir(const char* dir);
void RmDir(const char* dir);

void setBit(unsigned char& x, int pos, int val);
int getBit(unsigned char x, int pos);
void dumpString(char* pData, const std::string& st); // add '\0'