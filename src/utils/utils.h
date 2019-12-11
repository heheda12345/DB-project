#pragma once
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

bool DirExist(const char* dir);
void InitDir(const char* dir);
int TryMkDir(const char* dir);
void RmDir(const char* dir);