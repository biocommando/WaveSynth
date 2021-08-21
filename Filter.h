#pragma once

#include <vector>

class Filter
{
	float factor = 0;
	float sampleRate;
	float state[2] = { 0, 0 };

public:
	Filter(float sampleRate) : sampleRate(sampleRate)
	{
	}

	void setSampleRate(float sampleRate)
	{
		this->sampleRate = sampleRate;
	}

	void updateLowpass(float cutFreqHz)
	{
		factor = 1.0f / (1.0f / (2.0f * 3.14159265f * 1.0f / sampleRate * cutFreqHz) + 1.0f);
	}

	void updateHighpass(float cutFreqHz)
	{
		factor = 1.0f / (2.0f * 3.14159265f * 1.0f / sampleRate * cutFreqHz + 1.0f);
	}

	float processHighpass(float input)
	{
		state[0] = factor * (input + state[0] - state[1]);
		state[1] = input;

		return state[0];
	}

	float processLowpass(float input)
	{
		state[0] = factor * input + (1.0f - factor) * state[1];
		state[1] = state[0];

		return state[0];
	}
};

class LowpassFilter : Filter
{
public:
	LowpassFilter(float sampleRate) : Filter(sampleRate) {}

	inline float process(float input) { return processLowpass(input); }

	inline void update(float hz) { updateLowpass(hz); }
};

class MultistageLowpassFilter
{
	std::vector<Filter> filters;
public:
	MultistageLowpassFilter(float samplerate, int stages)
	{
		for (int i = 0; i < stages; i++)
		{
			filters.push_back(Filter(samplerate));
		}
	}

	MultistageLowpassFilter() {}

	void update(float hz)
	{
		for (auto i = filters.begin(); i != filters.end(); i++)
		{
			i->updateLowpass(hz);
		}
	}

	float process(float input)
	{
		float output = input;
		for (auto i = filters.begin(); i != filters.end(); i++)
		{
			output = i->processLowpass(output);
		}
		return output;
	}
};

class DcFilter : Filter
{
public:
	DcFilter(float sampleRate) : Filter(sampleRate)
	{
		updateHighpass(10);
	}

	inline float process(float input) { return processHighpass(input); }
};