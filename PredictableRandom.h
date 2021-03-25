#pragma once
#include<cstdint>

class PredictableRandom
{
	uint32_t state;
public:
	int next(int mod);
	void seed(uint32_t num);
	PredictableRandom();
	~PredictableRandom();
};

