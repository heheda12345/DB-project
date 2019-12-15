#pragma once
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string>
#include <vector>
#include "../redbase.h"

bool DirExist(const char* dir);
void InitDir(const char* dir);
int TryMkDir(const char* dir);
void RmDir(const char* dir);

void setBit(unsigned char& x, int pos, int val);
int getBit(unsigned char x, int pos);
void dumpString(char* pData, const std::string& st);
std::vector<std::string> getFiles(const char* path);
std::vector<std::string> getAllTable(const char* path);

std::string getName(AttrType ty, bool canChange);
int getDefaultLen(AttrType ty);

bool typeIsMatch(std::vector<AttrType> ty1, std::vector<AttrType> ty2);
bool lengthIsMatch(std::vector<std::string> attr1, std::vector<std::string> attr2);
int getSum(const std::vector<int> &vec);
int findName(const std::vector<std::string>& vec, const std::string& target);
bool isDumplicated(const std::vector<std::string>& vec);

bool isLegalDate(int date);
std::string cutForPrint(std::string st);