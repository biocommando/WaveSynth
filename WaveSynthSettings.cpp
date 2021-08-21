#include "WaveSynthSettings.h"
#include "IniFileReader.h"
#include <cstring>

extern void getWorkDir(char *workDir);

WaveSynthSettings::WaveSynthSettings()
{
	char workDir[256] = "";
	getWorkDir(workDir);
	IniFileReader* reader = new IniFileReader();
	char fileName[256];
	sprintf(fileName, "%sconfig\\settings.ini", workDir);
	reader->openFile(fileName);

	packCount = reader->readIntValue("wavepacks", "count");
	packIndex = reader->readIntValue("wavepacks", "default");
	for (int i = 0; i < 8 && i < packCount; i++) {
		reader->readCharValueWithIndex("wavepacks", "pack", i, packs[i]);
		reader->readCharValueWithIndex("wavepacks", "pack_id", i, packIds[i]);
	}

	useGUI = reader->readBoolValue("general", "useGui");

	reader->readCharValue("files", "banks", fileName);
	sprintf(banksFile, "%s%s", workDir, fileName);
	reader->readCharValue("files", "macros", fileName);
	sprintf(macrosFile, "%s%s", workDir, fileName);
	reader->readCharValue("files", "midiMap", fileName);
	sprintf(midiFile, "%s%s", workDir, fileName);

	oversampling = reader->readIntValue("general", "oversampling");
	if (oversampling <= 0)
		oversampling = 1;
	else if (oversampling > 32)
		oversampling = 32;

	reader->closeFile();
	delete reader;
}


WaveSynthSettings::~WaveSynthSettings()
{
}


FILE *WaveSynthSettings::getBankDefinitionFile()
{
	return fopen(banksFile, "r");
}

FILE *WaveSynthSettings::getMacroDefinitionFile()
{
	return fopen(macrosFile, "r");
}

FILE *WaveSynthSettings::getMidiMapDefinitionFile()
{
	return fopen(midiFile, "r");
}

int WaveSynthSettings::getOversampling()
{
	return oversampling;
}

int WaveSynthSettings::getSelectedPackIndex()
{
	return packIndex;
}
int WaveSynthSettings::getPackCount()
{
	return packCount;
}
void WaveSynthSettings::getPack(int index, char *result)
{
	if (index >= packCount || index < 0) return;
	strcpy(result, packs[index]);
}
void WaveSynthSettings::getPackId(int index, char *result)
{
	if (index >= packCount || index < 0) return;
	strcpy(result, packIds[index]);
}
int WaveSynthSettings::setSelectedPack(int index)
{
	if (index >= packCount || index < 0) return packIndex;
	packIndex = index;
	return packIndex;
}

bool WaveSynthSettings::getUseGUI()
{
	return useGUI;
}