#include "Saturation.h"

double Saturation::process(double input, double gain)
{
	if (gain < 1e-6)
		return input;
	double dist;
	if (input < -1)
		dist = dataSet[0];
	else if (input > 1)
		dist = dataSet[dataSetSize - 1];
	else
	{
		/*double dIndex = ((0.5 + input * 0.5) * (dataSetSize - 1));
		int index = (int)dIndex;
		if (index == dataSetSize - 1) index--;
		double diff = dIndex - index;
		return dataSet[index] + diff * (dataSet[index] - dataSet[index - 1]);*/
		double factor = 1.0 / (double)dataSetSize;
		for (int i = 0; i < dataSetSize - 1; i++)
		{
			if (input > 2 * i * factor - 1 && input < 2 * (i + 1) * factor - 1)
			{
				double diff = input - (2.0 * (double)i * factor - 1.0);
				dist = (dataSet[i] + (dataSet[i + 1] - dataSet[i]) * diff * dataSetSize * 0.5);
				break;
			}
		}
	}
	return dist  * gain + input * (1 - gain);
}

Saturation::Saturation(char *fileName)
{
	FILE *f = fopen(fileName, "r");
	char data[100];
	dataSet = NULL;
	int dataIdx = -1;
	dataSetSize = 0;
	while (!feof(f) && dataIdx < dataSetSize)
	{
		fgets(data, 100, f);
		if (data[0] == '/') continue;
		if (dataSet == NULL)
		{
			sscanf(data, "%d", &dataSetSize);
			if (dataSetSize > 0)
				dataSet = (double *)malloc((dataSetSize + 1) * sizeof(double));
			dataIdx = 0;
		}
		else
		{
			if (sscanf(data, "%lf", &dataSet[dataIdx]))
				dataIdx++;
		}
	}
	fclose(f);
	// Remove DC offset
	double dc = 0;
	for (int i = 0; i < dataSetSize; i++)
		dc += dataSet[i];
	dc /= (double)dataSetSize;
	for (int i = 0; i < dataSetSize; i++)
		dataSet[i] -= dc;
	dataSet[dataSetSize] = dataSet[dataSetSize - 1];
}


Saturation::~Saturation()
{
	free(dataSet);
}
