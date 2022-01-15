#pragma once
#include "ParamDto.h"
#include <string>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

#define TYPE_TEMPORARY 1

struct PluginVariable
{
	char type; // Currently not used for anything
	std::string name;
	double value;
};

std::vector<ParamDTO> ExecutePlugin(const std::string& filename, std::vector<PluginVariable>& initial);
