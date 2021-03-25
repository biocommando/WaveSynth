#pragma once
class MS20Filter
{
private:
	double k2, k1, T;
	double C1, C2, R1, R2;
	double xm1, xm2, ym1, ym2;
	double Ax, Bx, Cx, Ay, By, Cy;
	double iCy, Cx0, tempTerm1;
	double cutoff, resonance, cutmod, pC1, pR1;
	int sampleIdx, SCurve;
	void update();
	void updateConstants();
public:
	MS20Filter(int samplfreq);
	~MS20Filter();
	double calculate(double input);
	void setCutoff(double v);
	void setResonance(double v);
	void setModulation(double v);
	void setSCurve(int v);
	void setComponentValues(double c, double r);
	void reset();
};

