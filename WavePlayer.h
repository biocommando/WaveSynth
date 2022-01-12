#pragma once
#include "BasicFilter.h"
#include "MS20Filter.h"
#include "BasicOscillator.h"
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <mutex>
#define NUM_VOICES 64
#define SEMITONE 1.0594630943592952645618252949463
#define LOWEST_C_HZ 8.1757989156

#define ENVELOPE_SCALE 4

// Voice modes
#define VCMODE_ONE_SHOT 0
#define VCMODE_LOOP 1
#define VCMODE_LOOP_WITH_RELEASE 2

typedef struct {
	bool isPlaying, isLooping, noteReleased;
	double index, playSpeed, sampleRate, velocity;
	char key;
	char keyTemp; // this is the to-be-played buffer that will be transferred to key when processing
	int priority;
	MS20Filter *filter;
	double envA, envD, envS, envR, envPos, releasePos, envToCutoff, tailMultiplier;
} Voice;

class WavePlayer
{
private:
	Voice voices[NUM_VOICES];
	int voiceMode;
	int lastActiveVoice;
	unsigned int waveLen, waveLoop1, waveLoop2, modLoop, modLoop2;
	std::unique_ptr<double[]> waveMemory;
	double origFreqHz, playSpeedModifier, octave, tailMultiplier;
	double waveShaping;
	double pitchMultiplier; // for pitchbend
	int sampleRate;
	// UI runs in a different thread so loading a new wave may occur at the same time as processing is ongoing
	// so to protect the memory allocation and access we must have mutual exclusive lock for all memory operations.
	std::mutex waveMemoryLock;

	BasicFilter *DCCut;
	BasicOscillator *LFO;
	int LFOType;
	double LFOFreq, LFOToFilter, LFOToPitch, LFOToLevel, LFOToCue;
	void CreateBuffer(short *waveMemoryTemp, unsigned int waveLenTemp);
	void StartPlay(double freqHz, int voice);
	void StopPlay(int voice);
	void StopLoop(int voice);
	double GetEnvelope(int voice);
	double WaveShaping(int voice, double input);
public:
	WavePlayer(int sampleRate);
	~WavePlayer();
	bool isOneShot;
	void SetVoiceMode(int mode);
	void ReadFile(FILE *file);
	double process(double AdditionalModulation = 0);
	void NoteOn(char key, char velocity, double detune = 0, double phase = 0);
	void NoteOff(char key, bool forceStop = false);
	void MuteAll();
	void SetEnvelope(double a, double d, double s, double r);
	void SetPlaySpeedModifier(double octave, double semis, double linear = 1);
	void SetFilterValues(double cut, double res, double envToCutoff);
	void SetWaveShaping(double clipLevel);
	void SetLoopingModifier(double value);
	void SetLFOParams(int LFOType, double LFOFreq, double LFOToFilter, double LFOToPitch, double LFOToLevel, double LFOToCue);
	void SetPitchMultiplier(double multiplier);
	void SetDCCut(double freq0to1);
	void setMemoryLock(bool state);
};

