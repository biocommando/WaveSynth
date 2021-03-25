#include "BasicCompressor.h"


BasicCompressor::BasicCompressor(int sampleRate)
{
	th = rat = att = rel = e = 0;
	msToSamples = 1.0/(double)sampleRate;
}


BasicCompressor::~BasicCompressor()
{
}

void BasicCompressor::SetParameters(double threshold, double ratio, double attack, double release)
{
	th = threshold;
	rat = ratio;
	att = attack;
	rel = release;
}
double DbToLin(double v)
{
	v *= 0.025; // /40
	return pow(10, v);
}

void BasicCompressor::SetParametersDBMs(double threshold, double ratio, double attack, double release)
{
	static double th_prev = -1, ratio_prev = -1;
	if (th_prev != threshold)
	{
		th = DbToLin(threshold);
		th_prev = threshold;
	}
	if (ratio_prev != ratio)
	{
		rat = DbToLin(ratio);
		ratio_prev = ratio;
	}
	att = attack * msToSamples;
	rel = release * msToSamples;
}
double BasicCompressor::process(double input)
{
	double g = 1, ai;
	if (input > 0) ai = input;
	else if (input <= 0) ai = -input;
	if (e<1 && ai>th)
		e = e + att;
	if (e>0 && ai<th)
		e = e - rel;
	g = 1 - e*rat;
	return input*g;
}

void BasicCompressor::OneKnobMagic(double value)
{
	th = 0.5 * (1.0 - value);
	rat = 0.4 + 0.5 * value;
	att = 0.0022675 * (1.0 - value);
	rel = att;
}