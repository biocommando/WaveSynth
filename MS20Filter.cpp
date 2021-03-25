#include "MS20Filter.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

MS20Filter::MS20Filter(int samplfreq)
{
	T = 1.0 / (double)samplfreq;
	pC1 = 0.0000000033;
	pR1 = 20000;
	xm2 = 0;
	xm1 = 0;
	ym1 = 0;
	ym2 = 0;
	sampleIdx = 0;
	setCutoff(1);
	setResonance(0);
	setModulation(0);
	SCurve = 0;
	updateConstants();
	update();
}


MS20Filter::~MS20Filter()
{
}

double MS20Filter::calculate(double input)
{
	if (++sampleIdx == 15)
	{
		sampleIdx = 0;
		update();
	}
	double x = input;
	double y = (Ax*xm2 + Bx*xm1 + Cx*x - Ay * ym2 - By * ym1) * iCy;
	/*if (SCurve == 0)
		x = x / sqrt(1 + x*x);
	else if (SCurve == 1)
		x = 1.25 * x / (fabs(x) + 1);
	else if (SCurve == 2)
	{
		if (x < -1) x = -1;
		else if (x > 1) x = 1;
	}
	else
	{
		x = atan(3 * x) * 0.5;
	}*/
	if (x < -1) x = -1;
	else if (x > 1) x = 1;
	xm2 = xm1;
	xm1 = x;
	ym2 = ym1;
	ym1 = y;
	return y;
}

void MS20Filter::update()
{
	k2 = 2.2 * resonance;
	double cmtemp = cutmod + 1;
	cmtemp = 10 - (cutoff * cmtemp *1.4);
	R1 = pow(2, cmtemp*0.7382) * pR1;
	R2 = 3 * R1;
	Cx = Cx0 / R1;
	//Cx = 46.70634596 * exp(.7163537482 * cutoff);
	//Bx = (2 * T*T) / (C1*C2*R1);
	//Ax = (T*T) / (C1*C2*R1);
	Bx = 2 * Cx;
	Ax = Cx;
	double tempTerm2 = (2 * k2*T) / C2; // dependent on resonance
	double tempTerm3 = (2 * R2*T) / (C1*R1); // dependent on cutoff
	Cy = 4 * R2 + tempTerm1 - tempTerm2 + tempTerm3 + Cx;
	By = ((-8)*R2 + Bx);
	Ay = 4 * R2 - tempTerm1 + tempTerm2 - tempTerm3 + Ax;
	iCy = 1 / Cy;
}

void MS20Filter::updateConstants()
{
	C1 = pC1;
	C2 = C1*0.30303;
	Cx0 = (T*T) / (C1*C2);
	tempTerm1 = (2 * T) / C1 + (2 * T) / C2;
}

void  MS20Filter::setCutoff(double v)
{
	cutoff = 10 * v;
}
void  MS20Filter::setResonance(double v)
{
	resonance = v;
}
void  MS20Filter::setModulation(double v)
{
	cutmod = v;
}

void MS20Filter::setSCurve(int v)
{
	SCurve = v;
}

void MS20Filter::setComponentValues(double c, double r)
{
	pC1 = c;
	pR1 = r;
	updateConstants();
}

void MS20Filter::reset()
{
	xm2 = 0;
	xm1 = 0;
	ym1 = 0;
	ym2 = 0;
	sampleIdx = 0;
	updateConstants();
	update();
}
