#include "CChangeEvents.h"

CChangeMapping::CChangeMapping(VstInt32 paramNumber, int controllerNumber)
{
	this->controllerNumber = controllerNumber;
	this->paramNumber = paramNumber;
	this->value = 0;
}


CChangeMapping::~CChangeMapping()
{

}

int CChangeMapping::getControllerNumber()
{
	return controllerNumber;
}


void CChangeMapping::setValueAndNotify(float value, AudioEffectX *plugin)
{
	this->value = value;
	plugin->setParameter(paramNumber, value);
}

CChangeEvents::CChangeEvents(AudioEffectX *plugin)
{
	mappingCount = 0;
	this->plugin = plugin;
}


CChangeEvents::~CChangeEvents()
{
}


void CChangeEvents::addCCMapping(VstInt32 paramNumber, int controllerNumber)
{
	mappingCount++;
	const int mappingIndex = mappingCount - 1;
	CChangeMapping mapping(paramNumber, controllerNumber);
	mappings.push_back(mapping);
}

void CChangeEvents::onControllerChange(int controllerNumber, int value0to127)
{
	float value = 0.00787401574803149606299212598425f * value0to127;
	value = value > 1 ? 1 : (value < -1 ? -1 : value);
	for (int i = 0; i < mappingCount; i++)
	{
		if (mappings[i].getControllerNumber() == controllerNumber)
		{
			mappings[i].setValueAndNotify(value, plugin);
			//break;
		}
	}
}