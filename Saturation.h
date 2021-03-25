#pragma once
#include <stdio.h>
#include <stdlib.h>

class Saturation
{
	double *dataSet;
	int dataSetSize;
public:
	double process(double input, double gain);
	Saturation(char *fileName);
	~Saturation();
};

