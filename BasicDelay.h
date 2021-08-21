#pragma once
#include <stdlib.h>
#include <memory>

class BasicDelay
{
private:
	unsigned long delaySamples, bufferLength, index;
	int sampleRate;
	std::unique_ptr<double[]> buffer;
	double feed;

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

