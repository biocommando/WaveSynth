#pragma once
#include "ParamDto.h"
#include <string>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

#define TYPE_TEMPORARY 1
#define TYPE_VST_FLOAT_PARAM 2
#define TYPE_VST_INTEGER_PARAM 3

typedef struct{
	char type;
	std::string name;
	double value;
} ScriptVariable;

std::vector<ParamDTO> ExecuteScript(const std::string &filename, std::vector<ScriptVariable> &initial);
