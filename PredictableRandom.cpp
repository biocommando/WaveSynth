#include "PredictableRandom.h"

PredictableRandom::PredictableRandom() : state(123)
{
}

int PredictableRandom::next(int mod)
{
	uint32_t x = state;
	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;
	state = x;
	return state % mod;
}

void PredictableRandom::seed(uint32_t num)
{
	state = num;
}

PredictableRandom::~PredictableRandom()
{
}
