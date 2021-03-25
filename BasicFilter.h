#pragma once
enum FilterType { HIGH_PASS, LOW_PASS };

class BasicFilter
{
private:
	double o2, i2;
	FilterType type;
	double cutCoeff;
	double processLPFilter(double input);
	double processHPFilter(double input);
public:
	BasicFilter(bool isHipass = false, double initialCutoff = 0.0);
	~BasicFilter();
	void setCut0to1(double cut0to1);
	double process(double input);
	void reset();
};

