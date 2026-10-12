#include "MinimalParameter.h"
#include <cstring>
#include <cstdio>

extern void WriteLog(const char*, double);
MinimalParameter::MinimalParameter(const char *name, double defaultVal, double scaling, double addition, const char *label)
{
	snprintf(this->name, sizeof(this->name), "%s", name);
	this->value = defaultVal;
	this->scaling = scaling;
	this->addition = addition;
	snprintf(this->label, sizeof(this->label), "%s", label);
	displayCount = 0;
	showAsInt = 0;
}

MinimalParameter::~MinimalParameter()
{
}

void MinimalParameter::AddDisplayMapping(const char *mapping)
{
	if (displayCount == DISPLAY_MAPPINGS_MAX)
		return;
	if (strlen(mapping) + 1 > DISPLAY_MAPPINGS_LEN)
		displayMap[displayCount++][0] = 0;
	else
		strcpy(displayMap[displayCount++], mapping);
}

void MinimalParameter::ClearDisplayMappings()
{
	displayCount = 0;
}

void MinimalParameter::GetDisplayText(char *out, int bsize)
{
	GetDisplayText(out, bsize, value);
}

void MinimalParameter::GetDisplayText(char *out, int bsize, double value)
{
	if (displayCount == 0)
	{
		if (showAsInt)
			snprintf(out, bsize, "%d", (int)(value * scaling + addition));
		else
			snprintf(out, bsize, "%.2f", value * scaling + addition);
	}
	else
	{
		snprintf(out, bsize, "%s", displayMap[GetSelection(value)]);
	}
}

int MinimalParameter::GetSelection()
{
	return GetSelection(value);
}

int MinimalParameter::GetSelection(double value)
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