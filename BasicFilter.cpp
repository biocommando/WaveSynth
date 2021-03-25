#include "BasicFilter.h"


BasicFilter::BasicFilter(bool isHipass, double initialCutoff)
{
	i2 = 0;
	o2 = 0;
	type = isHipass ? HIGH_PASS : LOW_PASS;
	setCut0to1(initialCutoff);
}

BasicFilter::~BasicFilter()
{
}

void BasicFilter::setCut0to1(double cut0to1)
{
	if (cut0to1 < 0.0)
	{
		cut0to1 = 0.0;
	}
	if (type == HIGH_PASS)
	{
		cutCoeff = 0.3183099 / (0.3183099 + cut0to1);
	}
	else
	{
		cutCoeff = cut0to1 / (0.3183099 + cut0to1);
	}
}

double BasicFilter::process(double input)
{
	if (type == HIGH_PASS)
	{
		return processHPFilter(input);
	}
	else
	{
		return processLPFilter(input);
	}
}

double BasicFilter::processLPFilter(double input)
{
	o2 = o2 + cutCoeff * (input - o2);
	return o2;
}
double BasicFilter::processHPFilter(double input)
{
	o2 = cutCoeff * (input + o2 - i2);
	i2 = input;
	return o2;
}

void BasicFilter::reset()
{
	o2 = 0;
	i2 = 0;
}