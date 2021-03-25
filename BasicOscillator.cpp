#include "BasicOscillator.h"
#include<cstdlib>
#include<cmath>


BasicOscillator::BasicOscillator(int sampleRate, bool randomizePhase)
{
	if (randomizePhase)
		phase = (double)(rand() % 1000)*0.001;
	else
		phase = 0;
	Hz_to_F = 1.0 / (double)sampleRate;
	frequency = 0.0;
	oscType = 0;
	value = 0.0;
	lowpassFilter = new BasicFilter(false, 0.0001);
}


BasicOscillator::~BasicOscillator()
{
	delete lowpassFilter;
}

double BasicOscillator::sin1(double phase)
{
	double phase_rad = phase * 6.283185307179586476925286766559;
	return sin(phase_rad);
}
double BasicOscillator::tri1(double phase)
{
	if (phase < 0.5)
		return 4 * phase - 1;
	else
		return -4 * phase + 3;
}
double BasicOscillator::saw1(double phase)
{
		return 2 * phase - 1;
}
double BasicOscillator::sqr1(double phase)
{
	return phase < 0.5 ? 1 : -1;
}


double BasicOscillator::calculate(double ph)
{
	double val = 0;
	if (oscType == OSC_SINE)
	{
		val = sin1(ph);
	}
	else if (oscType == OSC_TRIANGLE)
	{
		val = tri1(ph);
	}
	else if (oscType == OSC_SAW)
	{
		val = saw1(ph);
	}
	else if (oscType == OSC_SQUARE)
	{
		val = sqr1(ph);
	}
	else if (oscType == OSC_SINE3)
	{
		val = sin1(ph);
		val = val * val * val;
	}
	else if (oscType == OSC_SMOOTH_SAW)
	{
		val = lowpassFilter->process(saw1(ph));
	}
	else if (oscType == OSC_SMOOTH_SQR)
	{
		val = lowpassFilter->process(sqr1(ph));
	}
	else if (oscType == OSC_NOISE)
	{
		val = 1 - (((double)(rand() % 20001)) / 10000.0);
		val = (lowpassFilter->process(val) - 0.14) * 30;
		if (val > 1) val = 1;
		else if (val < -1) val = -1;
	}
	return val;
}
double BasicOscillator::calculate()
{
	phase = phase + frequency;
	if (phase >= 1.0) phase = phase - 1.0;
	value = calculate(phase);
	return value;
}

double BasicOscillator::ValueWithPhaseShift(double offset)
{
	if (oscType == OSC_NOISE)
		return value;
	double phTotal = phase + offset;
	if (phTotal >= 1.0) phTotal = phTotal - 1.0;
	return calculate(phTotal);
}

void BasicOscillator::setFrequency(double f_Hz)
{
	frequency = f_Hz * Hz_to_F;
}
void BasicOscillator::setType(int oscType)
{
	if (oscType == OSC_SINE || oscType == OSC_TRIANGLE ||
		oscType == OSC_SAW || oscType == OSC_SQUARE ||
		oscType == OSC_SINE3 || oscType == OSC_SMOOTH_SAW ||
		oscType == OSC_SMOOTH_SQR || oscType == OSC_NOISE)
	{
		this->oscType = oscType;
	}
	if (oscType == OSC_SMOOTH_SAW || oscType == OSC_SMOOTH_SQR)
	{
		lowpassFilter->setCut0to1(2.2676e-4);
	}
	if (oscType == OSC_NOISE)
	{
		// This is tuned so that with cut off 0.0001 this will give values between -1 and 1
		lowpassFilter->setCut0to1(0.0001);
	}
}