// WaveSynth.h
#pragma once
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
#include <cstdint>
#include <mutex>
#include <vector>
#include "PredictableRandom.h"
#include "ParamDto.h"
#include "Filter.h"

class EditorGui;

class WaveSynth
{
private:
	CChangeEvents *CChanges = NULL;
	MinimalParameter *params[NUM_PARAMS];
	double paramDefaults[NUM_PARAMS];
	WavePlayer *osc[NUM_PARAM_SETS] = {};
	// bank & patch currently loaded to each oscillator, -1 = nothing loaded
	int loadedWave[NUM_PARAM_SETS][2];
	BasicDelay *delay[2] = {};
	Flanger *flanger[2] = {};
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
	char workDir[1024];
	int randomSeed = 0;
	void ReadCCMappings();
	void updateParams(int callerIndex = -1);
	void GetBanksAndPatches();
	void ChangePatch(int oscid);
	void ReplaceStr(char *str, char replaceWhat, char replacement);
	void setWaveFileOffset(FILE *waves, int bank, int patch);
	void destroyDsp();
	int oversampling = 1;
	int sampleRate = 44100;
	bool activated = false;
	FourstageLowpassFilter downsamplingFilter1;
	FourstageLowpassFilter downsamplingFilter2;

	bool transportWasPlaying = false;
	double tempo = 120;

	// Parameter changes done from the GUI that need to be reported to the host
	std::mutex pendingHostNotificationsLock;
	std::vector<int> pendingHostNotifications;
public:
	EditorGui *editor = nullptr;

	WaveSynth();
	~WaveSynth();
	// Creates parameters & reads configuration. Doesn't depend on sample rate.
	void init();
	// Creates the DSP objects for the given sample rate. Can be called multiple times.
	void activate(int sampleRate);
	void processReplacing(float **outputs, int sampleFrames);
	void processMidi(const uint8_t *data);
	void noteOn(int key, int velocity);
	void noteOff(int key);
	// Call once per process block with transport state
	void setTransportState(bool isPlaying, bool hasTempo, double tempo);
	double getTempo() { return tempo; }
	float getParameter(int index);
	double getParameterDefault(int index) { return paramDefaults[index]; }
	void setParameter(int index, float value);
	// Set parameter from GUI and inform host about the change
	void setParameterAutomated(int index, float value);
	// Returns the param indexes changed from GUI since last call
	std::vector<int> takePendingHostNotifications();
	bool hasPendingHostNotifications();
	void getParameterName(int index, char *label);
	void getParameterDisplay(int index, char *text);
	void getParameterDisplay(int index, double value, char *text, int size);
	void getParameterLabel(int index, char *label);
	bool getEffectName(char* name);
	bool getProductString(char* text);
	bool getVendorString(char* text);
	void getBankAndPatchName(char *buff, double bank, double patch);
	int setParameter(ParamDTO *src); // returns param index
	void getParameter(ParamDTO *dst, int index);
	float getParameter(char *name);
	void startTransaction();
	void endTransaction();
	bool isTransactionOver();
	FILE *getMacroDefinitionFile();
	void setSelectedPack(int packIndex);
	int getSelectedPack() { return settings.getSelectedPackIndex(); }
	bool getUseGUI() { return settings.getUseGUI(); }

	std::string getChunk();
	bool setChunk(const void* data, int byteSize);
};
