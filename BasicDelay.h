#pragma once
#include <stdlib.h>

class BasicDelay
{
private:
	unsigned long delaySamples, bufferLength, index;
	int sampleRate;
	double *buffer, feed;

#ifdef interpolated_delay
	double findex, indexinc;
#endif

public:
	BasicDelay(unsigned long bufferLength, int sampleRate = 44100);
	~BasicDelay();
	double process(double input);
	void setTime(double milliseconds);
	void setFeedback(double feedback);
};

