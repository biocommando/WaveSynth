#pragma once
#include <vector>

class WaveSynth;

class CChangeMapping
{
private:
	int controllerNumber;
	float value;
	int paramNumber;
public:
	void setValueAndNotify(float value, WaveSynth *plugin);
	int getControllerNumber();
	CChangeMapping(int paramNumber, int controllerNumber);
	~CChangeMapping();
};

class CChangeEvents
{
private:
	WaveSynth *plugin;
	std::vector<CChangeMapping> mappings;
	int mappingCount;
public:
	void addCCMapping(int paramNumber, int controllerNumber);
	void onControllerChange(int controllerNumber, int value0to127);
	CChangeEvents(WaveSynth *plugin);
	~CChangeEvents();
};

