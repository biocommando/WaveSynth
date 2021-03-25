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


typedef struct{
	int intValue;
	float floatValue;
	char id[32];
	void create(char *name, float value)
	{
		if (value < 0.0) value = 0;
		else if (value > 1.0) value = 1;
		floatValue = value;
		intValue = -1;
		strcpy(id, name);
	}
	/* The block (one dto) is saved into memory in the following way:
	type			count	description
	unsigned short	1		size of block, for forward compatibility
	char			1		type indication, currently f=float, i=int	
	char			1		N, length of id excluding terminating zero
	char			N		id excluding terminating zero
	float			1		floatValue
	*/
	char* fromMem(char *mem)
	{
		char *ptr = mem;
		unsigned short blockSize = 0;
		memcpy(&blockSize, ptr, sizeof(unsigned short));
		ptr += sizeof(unsigned short);
		char type = *ptr;
		ptr++;
		char idLen = *ptr;
		ptr++;
		memcpy(id, ptr, idLen);
		id[idLen] = 0;
		ptr += idLen;
		memcpy(&floatValue, ptr, sizeof(float));
		if (type == 'i')
			intValue = (int)floatValue;
		else
			intValue = -1;
		return mem + blockSize;
	}
	unsigned int toMem(char **mem, unsigned int size)
	{
		int len = strlen(id);
		unsigned short blockSize = len + 2 + sizeof(double) + sizeof(unsigned short);
		unsigned int newSize = size + blockSize;
		*mem = (char*)realloc(*mem, newSize);
		char *buf = *mem + size;
		memcpy(buf, &blockSize, sizeof(unsigned short));
		buf += sizeof(unsigned short);
		if (intValue != -1)
		{
			buf[0] = 'i';
			floatValue = (float)intValue;
		}
		else
			buf[0] = 'f';
		buf[1] = (char)len;
		memcpy(&buf[2], id, len);
		memcpy(&buf[2 + len], &floatValue, sizeof(float));
		return newSize;
	}
	void fromString(const char *str)
	{
		char type = 0;
		intValue = -1;
		sscanf(str, "%c %s %f", &type, id, &floatValue);
		if (type == 'i') intValue = (int)floatValue;
	}
	void toString(char *str)
	{
		if (intValue >= 0)
			sprintf(str, "i %s %d", id, intValue);
		else
			sprintf(str, "f %s %.3f", id, floatValue);
	}
} ParamDTO;


class WaveSynth : public AudioEffectX
{
private:
	CChangeEvents *CChanges;
	MinimalParameter *params[NUM_PARAMS];
	WavePlayer *osc[NUM_PARAM_SETS];
	BasicDelay *delay[2];
	Flanger *flanger[2];
	BasicOscillator *LFO;
	PredictableRandom *predictableRandom;
	struct{
		char name[8], patchNames[8][8];
		char longName[40], longPatchNames[8][40];
		int numPatches;
	}	banks[8];
	WaveSynthSettings *settings;
	int numBanks, sequence, currentPack;
	int transactionCount; // to avoid never ending loops
	char *chunk, workDir[1024];
	int randomSeed;
	void ReadSettings();
	void ReadCCMappings();
	void updateParams(int callerIndex = -1);
	void GetBanksAndPatches();
	void ChangePatch(WavePlayer *osc, MinimalParameter *pBank, MinimalParameter *pPatch);
	void ReplaceStr(char *str, char replaceWhat, char replacement);
	void setWaveFileOffset(FILE *waves, int bank, int patch);
	void setSelectedPack(int packIndex);
public:
	WaveSynth(audioMasterCallback audioMaster);
	~WaveSynth();
	void processReplacing(float **inputs, float **outputs, VstInt32 sampleFrames);
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
	VstInt32 processEvents(VstEvents* events);
	void startTransaction();
	void endTransaction();
	bool isTransactionOver();
	FILE *getMacroDefinitionFile();

	VstInt32 getChunk(void** data, bool isPreset);
	VstInt32 setChunk(void* data, VstInt32 byteSize, bool isPreset);

	void open();
	void close();
};
