#pragma once
#include "BasicFilter.h"
#define OSC_SINE 0
#define OSC_TRIANGLE 1
#define OSC_SAW 2
#define OSC_SQUARE 3
#define OSC_SINE3 4
#define OSC_SMOOTH_SAW 5
#define OSC_SMOOTH_SQR 6
#define OSC_NOISE 7

class BasicOscillator
{
private:
	double phase, frequency, Hz_to_F;
	BasicFilter *lowpassFilter;
	// sine, triangle, saw, square
	int oscType;
	double sin1(double phase);
	double tri1(double phase);
	double saw1(double phase);
	double sqr1(double phase);
public:
	double value;
	BasicOscillator(int sampleRate, bool randomizePhase = true);
	~BasicOscillator();
	double calculate();
	double calculate(double ph);
	double ValueWithPhaseShift(double offset);
	void setFrequency(double f_Hz);
	void setType(int oscType);
};

