// TestVST.h
#pragma once
#include "audioeffectx.h"
#include "aeffguieditor.h"
#include "Flanger.h"
#include "BasicFilter.h"
#include "BasicOscillator.h"
#include "WavePlayer.h"
#include "MinimalParameter.h"
#include "BasicDelay.h"
#include "WaveSynthConsts.h"
#include "CChangeEvents.h"
#include "WaveSynthSettings.h"
#include <ctime>
#include "PredictableRandom.h"
#include "ParamDto.h"
#include "Filter.h"


class WaveSynth : public AudioEffectX
{
private:
	CChangeEvents *CChanges = NULL;
	MinimalParameter *params[NUM_PARAMS];
	WavePlayer *osc[NUM_PARAM_SETS];
	BasicDelay *delay[2];
	Flanger *flanger[2];
	BasicOscillator *LFO = NULL;
	PredictableRandom *predictableRandom = NULL;
	struct{
		char name[8], patchNames[8][8];
		char longName[40], longPatchNames[8][40];
		int numPatches;
	}	banks[8];
	WaveSynthSettings settings;
	int numBanks = 0, sequence = 0, currentPack = 0;
	int transactionCount = 0; // to avoid never ending loops
	char *chunk = NULL, workDir[1024];
	int randomSeed = 0;
	void ReadCCMappings();
	void updateParams(int callerIndex = -1);
	void GetBanksAndPatches();
	void ChangePatch(WavePlayer *osc, MinimalParameter *pBank, MinimalParameter *pPatch);
	void ReplaceStr(char *str, char replaceWhat, char replacement);
	void setWaveFileOffset(FILE *waves, int bank, int patch);
	int oversampling;
	MultistageLowpassFilter downsamplingFilter1;
	MultistageLowpassFilter downsamplingFilter2;
public:
	WaveSynth(audioMasterCallback audioMaster);
	~WaveSynth();
	void processReplacing(float **inputs, float **outputs, VstInt32 sampleFrames) override;
	float getParameter(VstInt32 index);
	void setParameter(VstInt32 index, float value);
	void getParameterName(VstInt32 index, char *label);
	void getParameterDisplay(VstInt32 index, char *text);
	void getParameterLabel(VstInt32 index, char *label);
	bool getEffectName(char* name);
	bool getProductString(char* text);
	bool getVendorString(char* text);
	void getBankAndPatchName(char *buff, double bank, double patch);
	int setParameter(ParamDTO *src); // returns param index
	void getParameter(ParamDTO *dst, int index);
	float getParameter(char *name);
	VstInt32 processEvents(VstEvents* events) override;
	void startTransaction();
	void endTransaction();
	bool isTransactionOver();
	FILE *getMacroDefinitionFile();
	void setSelectedPack(int packIndex);
	int getSelectedPack() { return settings.getSelectedPackIndex(); }

	VstInt32 getChunk(void** data, bool isPreset);
	VstInt32 setChunk(void* data, VstInt32 byteSize, bool isPreset);

	void open();
	void close();
};
