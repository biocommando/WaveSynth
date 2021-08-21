#include "WavePlayer.h"


WavePlayer::WavePlayer(int sampleRate)
{
	this->sampleRate = sampleRate;
	waveLen = 0;
	playSpeedModifier = 1;
	waveShaping = 1;
	//isOneShot = false;
	voiceMode = VCMODE_LOOP;
	modLoop2 = modLoop = 0;
	tailMultiplier = 1;
	pitchMultiplier = 1;
	LFO = new BasicOscillator(sampleRate, false);
	LFO->setFrequency(0.01);
	LFOToFilter = LFOToLevel = LFOToPitch = LFOToCue = 0;
	DCCut = new BasicFilter(true, 0.0005);
	for (int voice = 0; voice < NUM_VOICES; voice++)
	{
		voices[voice].isPlaying = false;
		voices[voice].isLooping = false;
		voices[voice].noteReleased = false;
		voices[voice].releasePos = 0;
		voices[voice].priority = 0;
		voices[voice].filter = new MS20Filter(sampleRate);
		voices[voice].filter->setSCurve(2);
		voices[voice].envToCutoff = 0;
		voices[voice].key = -1;
	}
	lastActiveVoice = -1;
	SetEnvelope(0, 0, 1, 1);
}


WavePlayer::~WavePlayer()
{
	delete LFO;
	delete DCCut;
	for (int voice = 0; voice < NUM_VOICES; voice++)
	{
		delete voices[voice].filter;
	}
}

void WavePlayer::SetDCCut(double freq0to1)
{
	DCCut->setCut0to1(freq0to1 * freq0to1 * 0.1 + 0.0005);
}

void WavePlayer::setMemoryLock(bool state)
{
	if (state)
	{
		waveMemoryLock.lock();
	}
	else
	{
		waveMemoryLock.unlock();
	}
}

/*
	Inits the wave from file with the following format:
	(Size in bytes, type, description)
	4, uint, Magic number = 715517
	1, uint, Version
	4, uint, Data offset
	4, uint, Data length
	4, uint, Loop start
	4, uint, Loop end
	4, float, Base frequency
*/
void WavePlayer::ReadFile(FILE *fIn)
{
	MuteAll();
	unsigned char version;
	unsigned int dataOffset, magicNumber = 0, waveLenTemp;
	float origFreqFloat;
	unsigned long originalPosition = ftell(fIn);
	fread(&magicNumber, sizeof(unsigned int), 1, fIn);
	if (magicNumber != 715517)
	{
		return;
	}
	fread(&version, sizeof(unsigned char), 1, fIn);
	fread(&dataOffset, sizeof(unsigned int), 1, fIn);
	fread(&waveLenTemp, sizeof(unsigned int), 1, fIn);
	fread(&waveLoop1, sizeof(unsigned int), 1, fIn);
	fread(&waveLoop2, sizeof(unsigned int), 1, fIn);
	fread(&origFreqFloat, sizeof(float), 1, fIn);
	origFreqHz = origFreqFloat;
	double sampleRateConversion = (double)sampleRate / 44100.0;
	origFreqHz *= sampleRateConversion;
	fseek(fIn, dataOffset + originalPosition, SEEK_SET);
	auto waveMemoryTemp = std::make_unique<short[]>(waveLenTemp);
	fread(waveMemoryTemp.get(), sizeof(short), waveLenTemp, fIn);
	CreateBuffer(waveMemoryTemp.get(), waveLenTemp);
}

void WavePlayer::SetPitchMultiplier(double multiplier)
{
	pitchMultiplier = multiplier;
}

void WavePlayer::StopPlay(int voice)
{
	voices[voice].isLooping = false;
	voices[voice].isPlaying = false;
	voices[voice].noteReleased = false;
	voices[voice].releasePos = 0;
	voices[voice].key = voices[voice].keyTemp = -1;
	while (lastActiveVoice >= 0 && !voices[lastActiveVoice].isPlaying)
		lastActiveVoice--;
}
void WavePlayer::StartPlay(double freqHz, int voice)
{
	voices[voice].isPlaying = true;
	voices[voice].isLooping = voiceMode != VCMODE_ONE_SHOT;
	voices[voice].noteReleased = false;
	voices[voice].releasePos = 0;
	voices[voice].playSpeed = freqHz / origFreqHz;
	voices[voice].index = 0;
	voices[voice].envPos = 0;
	voices[voice].tailMultiplier = tailMultiplier;
	voices[voice].filter->reset();
	if (voice > lastActiveVoice) lastActiveVoice = voice;
}
void WavePlayer::CreateBuffer(short *waveMemoryTemp, unsigned int waveLenTemp)
{
	setMemoryLock(true);
	waveLen = waveLenTemp;
	waveMemory = std::make_unique<double[]>(waveLen);
	for (unsigned int ui = 0; ui < waveLen; ui++)
	{
		double temp = (double)waveMemoryTemp[ui] * 0.000030517578125; // 1/32768.0=0.000030517578125
		waveMemory[ui] = temp;
	}
	setMemoryLock(false);
}

void WavePlayer::StopLoop(int voice)
{
	voices[voice].isLooping = false;
}

void WavePlayer::ResetWaveMemory()
{
	/*if (waveLen)
	{
		setMemoryLock(true);
		free(waveMemory);
		waveMemory = NULL;
		setMemoryLock(false);
		waveLen = 0;
	}*/
}

void WavePlayer::SetVoiceMode(int mode)
{
	if (mode != VCMODE_LOOP && mode != VCMODE_LOOP_WITH_RELEASE && mode != VCMODE_ONE_SHOT)
		return;
	// Releases must be cut or else it won't work...
	if (voiceMode == VCMODE_LOOP_WITH_RELEASE && mode != voiceMode)
	{
		for (int voice = 0; voice < NUM_VOICES; voice++)
		{
			if (voices[voice].noteReleased)
				StopPlay(voice);
		}
	}
	voiceMode = mode;
}

void WavePlayer::MuteAll()
{
	for (int voice = 0; voice < NUM_VOICES; voice++)
		StopPlay(voice);
}

void WavePlayer::NoteOn(char key, char velocity, double detune, double phase)
{
	int lowestPriorityVoice = 0;
	// Finds the first unused voice or if it doesn't exist, use the lowest priority voice
	for (int voice = 0; voice < NUM_VOICES; voice++)
	{
		if (!voices[voice].isPlaying || (voices[voice].key == key && voiceMode != VCMODE_LOOP_WITH_RELEASE))
		{
			lowestPriorityVoice = voice;
			if (voices[voice].isPlaying)
				NoteOff(key, true);
			break;
		}
		if (voices[lowestPriorityVoice].priority > voices[voice].priority)
			lowestPriorityVoice = voice;
	}
	// update priorities for other voices
	for (int voice = 0; voice < NUM_VOICES; voice++)
	{
		if (lowestPriorityVoice != voice && voices[voice].isPlaying)
			voices[voice].priority--;
	}
	double freq = pow(SEMITONE, (double)key + detune) * LOWEST_C_HZ;
	StartPlay(freq, lowestPriorityVoice);
	voices[lowestPriorityVoice].priority = NUM_VOICES;
	voices[lowestPriorityVoice].velocity = (double)velocity * 0.0078741; //=~1/127
	voices[lowestPriorityVoice].keyTemp = key;


	if (phase > 1) phase = 1;
	else if (phase < 0) phase = 0;
	voices[lowestPriorityVoice].index = phase * waveLoop2;
}
void WavePlayer::NoteOff(char key, bool forceStop)
{
	for (int voice = 0; voice < NUM_VOICES; voice++)
	{
		if (voices[voice].isPlaying && voices[voice].key == key)
		{
			if (forceStop)
				StopPlay(voice);
			else if (voiceMode != VCMODE_LOOP_WITH_RELEASE)
				StopLoop(voice);
			voices[voice].noteReleased = true;
		}
	}
}


void WavePlayer::SetPlaySpeedModifier(double octave, double semis, double linear)
{
	playSpeedModifier = pow(2, octave) * pow(SEMITONE, semis) * linear;
}

void WavePlayer::SetEnvelope(double a, double d, double s, double r)
{
	a *= ENVELOPE_SCALE * sampleRate;
	d *= ENVELOPE_SCALE * sampleRate;
	r *= ENVELOPE_SCALE * sampleRate;
	if (a < 10)
		a = 10; // this will eliminate the clicking sound in unison mode
	if (d < 1)
		d = 1;
	if (r < 1)
		r = 1;
	if (s < 0)
		s = 0;
	else if (s > 1)
		s = 1;
	double tail = 0.001 * r + 0.999;
	if (tail < 0) tail = 0;
	else if (tail > 1) tail = 1;
	tailMultiplier = tail;
	for (int voice = 0; voice < NUM_VOICES; voice++)
	{
		voices[voice].envA = a;
		voices[voice].envD = d;
		voices[voice].envS = s;
		voices[voice].envR = r;
	}
}

double WavePlayer::GetEnvelope(int voice)
{
	double envLength = voices[voice].envA + voices[voice].envD;
	double level;

	if (voices[voice].envPos >= envLength)
	{
		level = voices[voice].envS;
	}
	else
	{
		if (!voices[voice].noteReleased || voiceMode != VCMODE_LOOP_WITH_RELEASE)
		{
			voices[voice].envPos += 1;
		}
		if (voices[voice].envPos < voices[voice].envA)
		{
			level = voices[voice].envPos / voices[voice].envA;
		}
		else
		{
			double posFromDecayBegin = voices[voice].envPos - voices[voice].envA;
			level = (1 - posFromDecayBegin / voices[voice].envD) * (1 - voices[voice].envS) + voices[voice].envS;
		}
	}
	if (voices[voice].noteReleased && voiceMode == VCMODE_LOOP_WITH_RELEASE)
	{
		voices[voice].releasePos += 1;
		level *= (voices[voice].envR - voices[voice].releasePos) / voices[voice].envR;
		if (voices[voice].releasePos >= voices[voice].envR)
			StopPlay(voice);
	}
	else if (voices[voice].index > waveLoop2 || !voices[voice].isLooping)
	{
		level *= voices[voice].tailMultiplier;
		voices[voice].tailMultiplier *= tailMultiplier;
	}

	return level;
}

void WavePlayer::SetFilterValues(double cut, double res, double envToCutoff)
{
	for (int voice = 0; voice < NUM_VOICES; voice++)
	{
		voices[voice].filter->setCutoff(cut);
		voices[voice].filter->setResonance(res);
		voices[voice].envToCutoff = envToCutoff;
	}
}

double WavePlayer::WaveShaping(int voice, double input)
{
	double voiceWaveShaping = waveShaping;

	if (voices[voice].index > waveLoop2)
		voiceWaveShaping = (1 - (voices[voice].index - waveLoop2) / ((double)waveLen - waveLoop2)) * waveShaping;

	double output = voiceWaveShaping * input;
	if (output > 1) output = 1;
	else if (output < -1) output = -1;
	return output;
}

void WavePlayer::SetWaveShaping(double clipLevel)
{
	waveShaping = 33 * clipLevel + 1;
}

void WavePlayer::SetLoopingModifier(double value)
{
	if (value < 0) value = 0;
	else if (value > 1) value = 1;
	unsigned int loopLen = waveLoop2 - waveLoop1;
	modLoop = (unsigned int)(waveLen * value);
	modLoop = (modLoop / loopLen) * loopLen;
}

void WavePlayer::SetLFOParams(int LFOType, double LFOFreq, double LFOToFilter, double LFOToPitch, double LFOToLevel, double LFOToCue)
{
	this->LFOType = LFOType;
	this->LFOFreq = LFOFreq;
	LFO->setFrequency(LFOFreq);
	LFO->setType(LFOType);
	this->LFOToFilter = LFOToFilter;
	this->LFOToPitch = LFOToPitch * LFOToPitch;
	this->LFOToLevel = LFOToLevel;
	this->LFOToCue = LFOToCue;
}

// Note! The user of this method must call setMemoryLock accordingly!
double WavePlayer::process(double AdditionalModulation)
{
	double totalOutput = 0;
	LFO->calculate();
	double LFOvalue = LFO->value + AdditionalModulation;
	if (LFOvalue > 1.0) LFOvalue = 1.0;
	else if (LFOvalue < -1.0) LFOvalue = -1.0;
	modLoop2 = (unsigned int)(waveLen * fabs(LFOToCue * LFOvalue));
	modLoop2 = (modLoop2 / (waveLoop2 - waveLoop1)) * (waveLoop2 - waveLoop1);
	for (int voice = 0; voice < NUM_VOICES; voice++)
	{
		if (voice > lastActiveVoice) break;
		if (!voices[voice].isPlaying)
		{
			continue;
		}
		voices[voice].key = voices[voice].keyTemp;
		voices[voice].index += voices[voice].playSpeed * playSpeedModifier * (1 - LFOToPitch * LFOvalue) * pitchMultiplier;
		if (voices[voice].isLooping && voices[voice].index > waveLoop2)
		{
			voices[voice].index -= waveLoop2 - waveLoop1;
			if (voices[voice].index < 0) voices[voice].index = 0;
		}
		if (voices[voice].index >= waveLen)
		{
			StopPlay(voice);
			continue;
		}
		double oneVoiceOutput;
		unsigned int modifiedIndex = (unsigned int)voices[voice].index;
		if (modLoop > 0) modifiedIndex = (modifiedIndex + modLoop) % waveLen;
		if (modLoop2 > 0) modifiedIndex = (modifiedIndex + modLoop2) % waveLen;
		oneVoiceOutput = waveMemory[modifiedIndex] * voices[voice].velocity * (1 - LFOToLevel * LFOvalue);
		double envelope = GetEnvelope(voice);
		oneVoiceOutput *= envelope;
		voices[voice].filter->setModulation(envelope * voices[voice].envToCutoff + LFOToFilter * LFOvalue);
		oneVoiceOutput = voices[voice].filter->calculate(WaveShaping(voice, oneVoiceOutput));
		totalOutput += oneVoiceOutput;
	}
	totalOutput = DCCut->process(totalOutput);
	return totalOutput;
}

