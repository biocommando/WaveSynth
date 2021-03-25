#include "Flanger.h"
#include <cstdlib>

Flanger::Flanger(int sampleRate, double phase)
{
	double bufferFactor = (double)sampleRate / 44100.0;
	bufferLength = (int) (FLANGER_BUF_LEN * bufferFactor);
	bufferLenDiv3 = bufferLength / 3;
	buffer = (double*)malloc(bufferLength * sizeof(double));
	for (int i = 0; i < bufferLength; i++)
		buffer[i] = 0;
	osc = new BasicOscillator(sampleRate);
	osc->setType(OSC_TRIANGLE);
	osc->setFrequency(1);
	delay = 0;
	depth = 0;
	freq = 1;
	index = (int)(phase * bufferLength);
	index %= bufferLength;
}


Flanger::~Flanger()
{
	free(buffer);
	delete osc;
}

void Flanger::SetParams(double rate_Hz, double delay, double depth)
{
	freq = rate_Hz;
	osc->setFrequency(freq);
	if (delay > 1) delay = 1;
	else if (delay < 0) delay = 0;
	this->delay = delay * bufferLenDiv3;
	if (depth > 1) depth = 1;
	else if (depth < 0) depth = 0;
	this->depth = depth * bufferLenDiv3;
}


double Flanger::process(double input)
{
	double f, frac;
	double temp1, temp2;
	double dly = delay + depth * (osc->calculate() + 1);
	
	f = (int)(dly - 0.5);
	frac = dly - f;

	index++;
	if (index == bufferLength)
	{
		index = 0;
	}

	buffer[index] = input;

	temp1 = index - f;
	if (temp1 < 0)
		temp1 = temp1 + bufferLength;
	temp1 = buffer[(int)temp1];

	temp2 = index - f - 1;
	if (temp2 < 0)
		temp2 = temp2 + bufferLength;
	temp2 = buffer[(int)temp2];

	return -1 * ((1 - frac) * temp1 + frac * temp2);

}