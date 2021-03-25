#pragma once
#include<math.h>

class BasicCompressor
{
	double th, rat, att, rel, e;
	double msToSamples;
public:
	void SetParameters(double threshold, double ratio, double attack, double release);
	void SetParametersDBMs(double threshold, double ratio, double attack, double release);
	void OneKnobMagic(double value);
	double process(double input);
	BasicCompressor(int sampleRate = 44100);
	~BasicCompressor();
};

