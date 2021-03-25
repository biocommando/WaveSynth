#include "MinimalParameter.h"
#include <cstring>
#include <cstdio>

extern void WriteLog(char*, double);
MinimalParameter::MinimalParameter(char *name, double defaultVal, double scaling, double addition, char *label)
{
	strcpy_s(this->name, 32, name);
	this->value = defaultVal;
	this->scaling = scaling;
	this->addition = addition;
	strcpy_s(this->label, 8, label);
	displayCount = 0;
	showAsInt = 0;
}

MinimalParameter::~MinimalParameter()
{
}

void MinimalParameter::AddDisplayMapping(char *mapping)
{
	if (displayCount == DISPLAY_MAPPINGS_MAX)
		return;
	if (strlen(mapping) + 1 > DISPLAY_MAPPINGS_LEN)
		displayMap[displayCount++][0] = 0;
	else
		strcpy_s(displayMap[displayCount++], DISPLAY_MAPPINGS_LEN, mapping);
}

void MinimalParameter::ClearDisplayMappings()
{
	displayCount = 0;
}

void MinimalParameter::GetDisplayText(char *out, int bsize)
{
	if (displayCount == 0)
	{
		if (showAsInt)
			sprintf_s(out, bsize, "%d", (int)(value * scaling + addition));
		else
			sprintf_s(out, bsize, "%.2f", value * scaling + addition);
	}
	else
	{
		strcpy_s(out, bsize, displayMap[GetSelection()]);
	}
}

int MinimalParameter::GetSelection()
{
	return (int)(value * 0.99 * displayCount);
}

double MinimalParameter::SetSelection(int index)
{
	if (displayCount == 0) return value;
	value = (double) (index + 0.5) / displayCount;
	if (value > 1) value = 1;
	else if (value < 0) value = 0;
	return value;
}

bool MinimalParameter::IsSelection()
{
	return displayCount > 0;
}