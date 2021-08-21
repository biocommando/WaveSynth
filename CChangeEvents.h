#pragma once
#include "audioeffectx.h"
#include "aeffguieditor.h"
#include <vector>

class CChangeMapping
{
private:
	int controllerNumber;
	float value;
	VstInt32 paramNumber;
public:
	void setValueAndNotify(float value, AudioEffectX *plugin);
	int getControllerNumber();
	CChangeMapping(VstInt32 paramNumber, int controllerNumber);
	~CChangeMapping();
};

class CChangeEvents
{
private:
	AudioEffectX *plugin;
	std::vector<CChangeMapping> mappings;
	int mappingCount;
public:
	void addCCMapping(VstInt32 paramNumber, int controllerNumber);
	void onControllerChange(int controllerNumber, int value0to127);
	CChangeEvents(AudioEffectX *plugin);
	~CChangeEvents();
};

