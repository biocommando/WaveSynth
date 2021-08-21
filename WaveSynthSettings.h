#pragma once
#include <cstdio>

class WaveSynthSettings
{
private:
	int packIndex;
	int packCount;
	char packs[8][256];
	char packIds[8][32];
	char banksFile[256];
	char macrosFile[256];
	char midiFile[256];
	bool useGUI;
	int oversampling;
public:
	
	int getSelectedPackIndex();
	int getPackCount();
	void getPack(int index, char *result);
	void getPackId(int index, char *result);
	int setSelectedPack(int index);
	bool getUseGUI();
	FILE *getBankDefinitionFile();
	FILE *getMacroDefinitionFile();
	FILE *getMidiMapDefinitionFile();
	int getOversampling();
	
	WaveSynthSettings();
	~WaveSynthSettings();
};

