#pragma once
#include "BasicOscillator.h"
#include <memory>

#define FLANGER_BUF_LEN 600

class Flanger
{
private:
	BasicOscillator *osc;
	double delay, depth, freq;
	std::unique_ptr<double[]> buffer;
	int index;
	int bufferLength, bufferLenDiv3;
public:
	Flanger(int sampleRate, double phase = 0);
	~Flanger();
	double process(double input);
	void SetParams(double rate_Hz, double delay, double depth);
};

