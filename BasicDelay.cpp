#include "BasicDelay.h"


BasicDelay::BasicDelay(unsigned long bufferLength, int sampleRate)
{
	this->sampleRate = sampleRate;
	this->bufferLength = bufferLength;
	this->delaySamples = 0;
	feed = 0;
	buffer = std::make_unique<double[]>(bufferLength);//(double*)malloc(sizeof(double) * bufferLength);
	/*for (unsigned long i = 0; i < bufferLength; i++)
		buffer[i] = 0;*/
	index = 0;
#ifdef interpolated_delay
	findex = 0;
	indexinc = 0;
#endif
}


BasicDelay::~BasicDelay()
{
}

double BasicDelay::process(double input)
{
	
	buffer[index] = buffer[index] * feed + input;
#ifdef interpolated_delay
	for (int i = 0; i < (int)(indexinc); i++)
	{
		if (++index >= bufferLength) index = 0;
		buffer[index] = buffer[index] * feed + input;
	}
#else
	if (++index >= delaySamples) index = 0;
	if (index >= bufferLength) index = 0;
#endif
	return buffer[index];
}
void BasicDelay::setTime(double milliseconds)
{
#ifndef interpolated_delay
	unsigned long oldDelay = delaySamples;
#endif
	delaySamples = (unsigned long)(milliseconds * 0.001 * sampleRate);
	if (delaySamples > bufferLength)
		delaySamples = bufferLength;
#ifdef interpolated_delay
	indexinc = (double)bufferLength / (double)delaySamples;
#else
	if (delaySamples < oldDelay)
		for (unsigned long ul = delaySamples; ul < oldDelay; ul++)
			buffer[ul] = 0;
#endif
}
void BasicDelay::setFeedback(double feedback)
{
	feed = feedback;
	if (feed < 0) feed = 0;
	else if (feed > 0.99) feed = 0.99;
}
