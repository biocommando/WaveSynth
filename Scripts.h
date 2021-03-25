#pragma once
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

#define TYPE_TEMPORARY 1
#define TYPE_VST_FLOAT_PARAM 2
#define TYPE_VST_INTEGER_PARAM 3

typedef struct{
	char type, name[32];
	double value;
} ScriptVariable;

unsigned int ExecuteScript(char *filename, char **dtoStream, char *initial);
