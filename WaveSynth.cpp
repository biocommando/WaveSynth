// This is the main DLL file.

#include "stdafx.h"
#include "WaveSynth.h"
#include "EditorGui.h"
#include "IniFileReader.h"
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include "util.h"

FstAudioEffect* createFstInstance(audioMasterCallback audioMaster)
{
	return new WaveSynth(audioMaster);
}

bool loggingOn = true;
// same as above but persists the file until it is unloaded... this can be used when a faster logwrite is needed
void WriteLogPersist(const char *msg, double val = 0, bool unLoad = false)
{
	if (!loggingOn) return;
	static int event_id = 0;
	static FILE *log = NULL;
	char mode[2] = "a";
	if (event_id == 0) mode[0] = 'w';
	if (log == NULL)
	{
		log = fopen("c:\\users\\public\\wavesynth.txt", mode);
	}
	fprintf(log, "%d %s - %f\n", event_id++, msg, val);
	if (unLoad)
	{
		fclose(log);
		log = NULL;
	}
}
void WriteLog(const char *msg, double val = 0)
{
	WriteLogPersist(msg, val, true);
}

void getWorkDir(char *workDir)
{
	auto dir = Util::getWorkDir(true);
	if (dir.back() != '\\')
		dir = dir + '\\';
	strcpy(workDir, dir.c_str());
}

WaveSynth::WaveSynth(audioMasterCallback audioMaster) :
	AudioEffectX(audioMaster, 0, NUM_PARAMS) {
	setNumInputs(2); // stereo in
	setNumOutputs(2); // stereo out
	setUniqueID('WaSy');	// identify
	getWorkDir(workDir);
	CChanges = new CChangeEvents(this);
	chunk = NULL;
	transactionCount = 0;
	isSynth(true);
	programsAreChunks();
	if (settings.getUseGUI())
	{
		hasEditor();
	}
}

void WaveSynth::open()
{
	startTransaction();

	randomSeed = (int)time(NULL);
	predictableRandom = new PredictableRandom();
	predictableRandom->seed(randomSeed);
	
	GetBanksAndPatches();

	oversampling = settings.getOversampling();

	int sampleRate = (int)getSampleRate();

	downsamplingFilter1 = MultistageLowpassFilter(sampleRate * oversampling, 4);
	downsamplingFilter1.update(0.5 * sampleRate);
	downsamplingFilter2 = MultistageLowpassFilter(sampleRate * oversampling, 4);
	downsamplingFilter2.update(0.5 * sampleRate);

	for (int ch = 0; ch < 2; ch++)
	{
		delay[ch] = new BasicDelay(sampleRate * oversampling, sampleRate * oversampling);
		flanger[ch] = new Flanger(sampleRate * oversampling, ch * 0.5);
	}
	LFO = new BasicOscillator(sampleRate * oversampling);

	params[P_TOTAL_LEVEL] = new MinimalParameter("Level", 0.5, 1, 0, "");
	params[P_UNISON_COUNT] = new MinimalParameter("Unison", 0, NUM_UNISON_VOICES * 0.99, 1, "");
	params[P_UNISON_COUNT]->showAsInt = 1;
	params[P_UNISON_DETUNE] = new MinimalParameter("UDetune", 0, 100, 0, "");
	params[P_UNISON_DETUNE]->showAsInt = 1;
	params[P_DELAY_TIME] = new MinimalParameter("DelayTm", 0, 1000, 0, "ms");
	params[P_DELAY_TIME]->showAsInt = 1;
	params[P_DELAY_FEED] = new MinimalParameter("DlyFeed", 0.0, 1, 0, "");
	params[P_DELAY_WET] = new MinimalParameter("DlyWet", 0.0, 1, 0, "");
	params[P_SEQUENCE] = new MinimalParameter("OscSeq", 0.0, 1, 0, "");
	params[P_SEQUENCE]->AddDisplayMapping("Layered");
	params[P_SEQUENCE]->AddDisplayMapping("1>2>3");
	params[P_SEQUENCE]->AddDisplayMapping("1>2>3>2");
	params[P_SEQUENCE]->AddDisplayMapping("Random");
	params[P_SOFT_CLIP] = new MinimalParameter("SoftClp", 0.0, 1, 0, "");
	params[P_FLANGER_FREQ] = new MinimalParameter("FlngFrq", 0.0, 5, 0.01, "Hz");
	params[P_FLANGER_DELAY] = new MinimalParameter("FlngDly", 0.0, 1, 0, "");
	params[P_FLANGER_DEPTH] = new MinimalParameter("FlngDpt", 0.0, 1, 0, "");
	params[P_FLANGER_WET] = new MinimalParameter("FlngWet", 0.0, 100, 0, "%");
	params[P_FLANGER_WET]->showAsInt = 1;
	sequence = 0;

	params[P_GLOBAL_LFO_FREQ] = new MinimalParameter("LfoFreq", 0.0, 5, 0.01, "Hz");
	params[P_GLOBAL_LFO_TYPE] = new MinimalParameter("LfoType", 0.0, 1, 0, "");
	params[P_GLOBAL_LFO_TYPE]->AddDisplayMapping("SINE");
	params[P_GLOBAL_LFO_TYPE]->AddDisplayMapping("TRI");
	params[P_GLOBAL_LFO_TYPE]->AddDisplayMapping("SAW");
	params[P_GLOBAL_LFO_TYPE]->AddDisplayMapping("SQR");
	params[P_GLOBAL_LFO_TYPE]->AddDisplayMapping("SIN3");
	params[P_GLOBAL_LFO_TYPE]->AddDisplayMapping("S.SAW");
	params[P_GLOBAL_LFO_TYPE]->AddDisplayMapping("S.SQR");
	params[P_GLOBAL_LFO_TYPE]->AddDisplayMapping("Noise");
	params[P_GLOBAL_LFO_TO_FLANGER] = new MinimalParameter("L2Flang", 0.0, 1, 0, "");
	params[P_GLOBAL_LFO_TO_DELAY] = new MinimalParameter("L2Delay", 0.0, 1, 0, "");
	params[P_GLOBAL_LFO_TO_CLIP] = new MinimalParameter("L2Clip", 0.0, 1, 0, "");
	params[P_GLOBAL_LFO_TO_LEVEL] = new MinimalParameter("L2Level", 0.0, 1, 0, "");
	params[P_GLOBAL_LFO_TO_PAN] = new MinimalParameter("L2Pan", 0.0, 1, 0, "");
	params[P_GLOBAL_LOW_CUT] = new MinimalParameter("LowCut", 0.0, 1, 0, "");
	params[P_GLOBAL_LFO_STEREO_OFFSET] = new MinimalParameter("LStereo", 0.0, 1, 0, "");
	params[P_GLOBAL_LFO_TO_OSC_LFO] = new MinimalParameter("L2LFOs", 0.0, 1, 0, "");

	for (int oscid = 0; oscid < NUM_PARAM_SETS; oscid++)
	{
		char temp[10];
		osc[oscid] = new WavePlayer(sampleRate * oversampling);
		sprintf(temp, "Bank_%d", oscid + 1);
		params[P_SET(oscid, P_BANK)] = new MinimalParameter(temp, 0, 1, 0, "");
		params[P_SET(oscid, P_BANK)]->tag = -1;
		for (int i = 0; i < numBanks; i++)
		{
			params[P_SET(oscid, P_BANK)]->AddDisplayMapping(banks[i].name);
		}
		sprintf(temp, "Patch_%d", oscid + 1);
		params[P_SET(oscid, P_PATCH)] = new MinimalParameter(temp, 0, 1, 0, "");
		params[P_SET(oscid, P_PATCH)]->tag = -1;
		ChangePatch(osc[oscid], params[P_SET(oscid, P_BANK)], params[P_SET(oscid, P_PATCH)]);

		sprintf(temp, "Attack%d", oscid + 1);
		params[P_SET(oscid, P_ATTACK)] = new MinimalParameter(temp, 0, 4, 0, "sec");
		sprintf(temp, "Decay%d", oscid + 1);
		params[P_SET(oscid, P_DECAY)] = new MinimalParameter(temp, 0, 4, 0, "sec");
		sprintf(temp, "Sust.%d", oscid + 1);
		params[P_SET(oscid, P_SUSTAIN)] = new MinimalParameter(temp, 1, 1, 0, "");
		sprintf(temp, "Tail.%d", oscid + 1);
		params[P_SET(oscid, P_TAIL)] = new MinimalParameter(temp, 1, 4, 0, "sec");

		sprintf(temp, "Octave%d", oscid + 1);
		params[P_SET(oscid, P_DETUNE_OCT)] = new MinimalParameter(temp, 0.5, 1, 0, "");
		for (int dmap = -3; dmap <= 3; dmap++)
		{
			char plus[2] = { 0, 0 };
			if (dmap > 0) plus[0] = '+';
			sprintf(temp, "%s%d", plus, dmap);
			params[P_SET(oscid, P_DETUNE_OCT)]->AddDisplayMapping(temp);
		}
		sprintf(temp, "Detune%d", oscid + 1);
		params[P_SET(oscid, P_DETUNE_CENT)] = new MinimalParameter(temp, 0.5, 100, -50, "");
		params[P_SET(oscid, P_DETUNE_CENT)]->showAsInt = 1;
		sprintf(temp, "Level_%d", oscid + 1);
		params[P_SET(oscid, P_LEVEL)] = new MinimalParameter(temp, 1, 1, 0, "");
		sprintf(temp, "FCut_%d", oscid + 1);
		params[P_SET(oscid, P_CUTOFF)] = new MinimalParameter(temp, 1, 1, 0, "");
		sprintf(temp, "FRes_%d", oscid + 1);
		params[P_SET(oscid, P_RESONANCE)] = new MinimalParameter(temp, 0, 1, 0, "");
		sprintf(temp, "E2Cut_%d", oscid + 1);
		params[P_SET(oscid, P_ENV_TO_CUTOFF)] = new MinimalParameter(temp, 0, 1, 0, "");
		sprintf(temp, "Wavshp%d", oscid + 1);
		params[P_SET(oscid, P_DISTORTION)] = new MinimalParameter(temp, 0, 1, 0, "");
		sprintf(temp, "Loop%d", oscid + 1);
		params[P_SET(oscid, P_LOOPING)] = new MinimalParameter(temp, 0.5, 1, 0, "");
		params[P_SET(oscid, P_LOOPING)]->AddDisplayMapping("No");
		params[P_SET(oscid, P_LOOPING)]->AddDisplayMapping("Yes");
		params[P_SET(oscid, P_LOOPING)]->AddDisplayMapping("Release");
		sprintf(temp, "CuePos%d", oscid + 1);
		params[P_SET(oscid, P_LOOP_POSITION)] = new MinimalParameter(temp, 0, 1, 0, "");
		sprintf(temp, "LFOFrq%d", oscid + 1);
		params[P_SET(oscid, P_LFO_FREQ)] = new MinimalParameter(temp, 0, 5, 0.01, "Hz");
		sprintf(temp, "LFOShp%d", oscid + 1);
		params[P_SET(oscid, P_LFO_TYPE)] = new MinimalParameter(temp, 0, 1, 0, "");
		params[P_SET(oscid, P_LFO_TYPE)]->AddDisplayMapping("SINE");
		params[P_SET(oscid, P_LFO_TYPE)]->AddDisplayMapping("TRI");
		params[P_SET(oscid, P_LFO_TYPE)]->AddDisplayMapping("SAW");
		params[P_SET(oscid, P_LFO_TYPE)]->AddDisplayMapping("SQR");
		params[P_SET(oscid, P_LFO_TYPE)]->AddDisplayMapping("SIN3");
		params[P_SET(oscid, P_LFO_TYPE)]->AddDisplayMapping("S.SAW");
		params[P_SET(oscid, P_LFO_TYPE)]->AddDisplayMapping("S.SQR");
		params[P_SET(oscid, P_LFO_TYPE)]->AddDisplayMapping("Noise");
		sprintf(temp, "LFO2Fl%d", oscid + 1);
		params[P_SET(oscid, P_LFO_TO_FILTER)] = new MinimalParameter(temp, 0, 1, 0, "");
		sprintf(temp, "LFO2Pt%d", oscid + 1);
		params[P_SET(oscid, P_LFO_TO_PITCH)] = new MinimalParameter(temp, 0, 1, 0, "");
		sprintf(temp, "LFO2Lv%d", oscid + 1);
		params[P_SET(oscid, P_LFO_TO_LEVEL)] = new MinimalParameter(temp, 0, 1, 0, "");
		sprintf(temp, "LFO2Cu%d", oscid + 1);
		params[P_SET(oscid, P_LFO_TO_CUE)] = new MinimalParameter(temp, 0, 1, 0, "");
		sprintf(temp, "Pan_%d", oscid + 1);
		params[P_SET(oscid, P_PAN)] = new MinimalParameter(temp, 0.5, 200, -100, "%");
		params[P_SET(oscid, P_PAN)]->showAsInt = 1;
	}

	if (settings.getUseGUI())
	{
		extern AEffGUIEditor* createEditor(AudioEffectX*);
		setEditor(createEditor(this));
	}

	ReadCCMappings();

	endTransaction();
}

void WaveSynth::startTransaction()
{
	transactionCount++;
}
void WaveSynth::endTransaction()
{
	transactionCount--;
}
bool WaveSynth::isTransactionOver()
{
	return transactionCount == 0;
}

void WaveSynth::ReadCCMappings()
{
	IniFileReader *reader = new IniFileReader();
	reader->openFile(settings.getMidiMapDefinitionFile());
	int mapIdx = 0;
	while (1)
	{
		char segment[100] = "";
		reader->readCharValueWithIndex("midimap", "map", mapIdx, segment);
		if (!segment[0])
		{
			break;
		}

		int mappingIdx = 0;
		while (1)
		{
			char mapping[32] = "";
			reader->readCharValueWithIndex(segment, "src", mappingIdx, mapping);
			int cc;
			if (mapping[0] && sscanf(mapping, "%d", &cc))
			{
				reader->readCharValueWithIndex(segment, "dst", mappingIdx, mapping);
				for (int i = 0; i < NUM_PARAMS; i++)
				{
					if (!strcmp(params[i]->name, mapping))
					{
						CChanges->addCCMapping(i, cc);
						break;
					}
				}
			}
			else
			{
				break;
			}
			mappingIdx++;
		}
		mapIdx++;
	}
	reader->closeFile();
	delete reader;
}

void WaveSynth::close()
{
	if (chunk != NULL)
		free(chunk);

	for (int i = 0; i < NUM_PARAMS; i++)
		delete params[i];

	for (int oscid = 0; oscid < NUM_PARAM_SETS; oscid++)
		delete osc[oscid];

	for (int ch = 0; ch < 2; ch++)
	{
		delete delay[ch];
		delete flanger[ch];
	}

	delete CChanges;
	delete LFO;
	delete predictableRandom;
}

WaveSynth::~WaveSynth() {

}
// Chunk consists of:
// version id
// -- 2 bytes: 'v','i' (version info)
// -- version as unsigned short
// In version 2 onwards:
// -- data offset as unsigned int
// In version 3 onwards:
// -- bank index as int
// In version 4 onwards:
// -- used random seed as int
// all else: parameter data
VstInt32 WaveSynth::getChunk(void** data, bool isPreset)
{
	if (chunk)
		free(chunk);

	const unsigned int headerSize = 2 + sizeof(unsigned short) + sizeof(unsigned int) + sizeof(int) + sizeof(int);
	char header[headerSize];
	header[0] = 'v';
	header[1] = 'i';
	unsigned short version = CURRENT_VERSION;
	int offset = 2;
	memcpy(&header[offset], &version, sizeof(unsigned short));
	offset += sizeof(unsigned short);
	memcpy(&header[offset], &headerSize, sizeof(unsigned int));
	offset += sizeof(unsigned int);
	int selectedPack = settings.getSelectedPackIndex();
	memcpy(&header[offset], &selectedPack, sizeof(int));
	offset += sizeof(unsigned int);
	memcpy(&header[offset], &randomSeed, sizeof(int));
	std::string s(header, headerSize);
	for (int i = 0; i < NUM_PARAMS; i++)
	{
		ParamDTO dto;
		getParameter(&dto, i);
		s.append(dto.toMem());
	}

	*data = malloc(s.size());
	memcpy(*data, s.c_str(), s.size());
	return s.size();
}

void WaveSynth::setSelectedPack(int packIndex)
{
	if (currentPack != packIndex)
	{
		settings.setSelectedPack(packIndex);
		GetBanksAndPatches();
		for (int i = 0; i < NUM_PARAM_SETS; i++)
		{
			params[P_SET(i, P_BANK)]->ClearDisplayMappings();
			for (int iBankInPack = 0; iBankInPack < numBanks; iBankInPack++)
			{
				params[P_SET(i, P_BANK)]->AddDisplayMapping(banks[iBankInPack].name);
			}
			params[P_SET(i, P_PATCH)]->tag = -1;
			params[P_SET(i, P_BANK)]->tag = -1;
		}
	}
}

VstInt32 WaveSynth::setChunk(void* data, VstInt32 byteSize, bool isPreset)
{
	auto chunk = (char*)data;

	unsigned short version;
	unsigned int dataOffset = 0;
	char *ptr = chunk;
	int offset = 0;
	if (byteSize < 2 + sizeof(unsigned short) || !(chunk[0] == 'v' && chunk[1] == 'i'))
	{
		version = 1;
	}
	else
	{
		offset = 2;
		memcpy(&version, &chunk[offset], sizeof(unsigned short));
		offset += sizeof(unsigned short);
		memcpy(&dataOffset, &chunk[offset], sizeof(unsigned int));
		offset += sizeof(unsigned int);
	}
	if (version < 3)
	{
		setSelectedPack(0);
	}
	else
	{
		int selectedPack = 0;
		memcpy(&selectedPack, &chunk[offset], sizeof(int));
		offset += sizeof(int);
		setSelectedPack(selectedPack);
	}
	if (version == 4)
	{
		memcpy(&randomSeed, &chunk[offset], sizeof(int));
		offset += sizeof(int);
	}
	predictableRandom->seed(randomSeed);
	if (version == 1)
	{
		setParameter(P_GLOBAL_LFO_STEREO_OFFSET, 0);
		setParameter(P_GLOBAL_LOW_CUT, 0);
		setParameter(P_GLOBAL_LFO_TO_OSC_LFO, 0);
	}
	ptr = &chunk[dataOffset];
	while (ptr - chunk < byteSize)
	{
		ParamDTO dto;
		ptr = dto.fromMem(ptr);
		setParameter(&dto);
	}
	if (version == 1)
	{
		setParameter(P_GLOBAL_LFO_FREQ, (float)(getParameter(P_GLOBAL_LFO_FREQ) / 5.0));
	}
	return 0;
}


int WaveSynth::setParameter(ParamDTO *src)
{
	startTransaction();
	if (!strcmp(src->id, "randomSeed") && src->floatValue >= 0)
	{
		randomSeed = (int)(10000 * src->floatValue);
		predictableRandom->seed(randomSeed);
		endTransaction();
		return -1;
	}
	for (int i = 0; i < NUM_PARAMS; i++)
	{
		if (!strcmp(params[i]->name, src->id))
		{
			if (src->intValue >= 0)
				params[i]->SetSelection(src->intValue);
			else
				params[i]->value = src->floatValue;
			updateParams(i);
			endTransaction();
			return i;
		}
	}
	endTransaction();
	return -1;
}

float WaveSynth::getParameter(char *name)
{
	for (int i = 0; i < NUM_PARAMS; i++)
	{
		if (!strcmp(params[i]->name, name))
		{
			return (float)params[i]->value;
		}
	}
	return 0;
}


void WaveSynth::getParameter(ParamDTO *dst, int index)
{
	MinimalParameter *p = params[index];
	if (p->IsSelection())
		dst->intValue = p->GetSelection();
	else
	{
		dst->intValue = -1;
		dst->floatValue = (float)p->value;
	}
	strcpy(dst->id, p->name);
}

void WaveSynth::setWaveFileOffset(FILE *waves, int bank, int patch)
{
	unsigned long offset = (bank * 8 + patch) * sizeof(unsigned long);
	fseek(waves, offset, SEEK_SET);
	fread(&offset, sizeof(unsigned long), 1, waves);
	fseek(waves, offset, SEEK_SET);
}

void WaveSynth::ChangePatch(WavePlayer *osc, MinimalParameter *pBank, MinimalParameter *pPatch)
{
	bool readNewWave = false;
	startTransaction();
	if (pBank->tag != pBank->GetSelection())
	{
		int sel = pBank->GetSelection();
		char temp[10];
		strcpy(temp, pPatch->name);
		pPatch->ClearDisplayMappings();
		for (int i = 0; i < banks[sel].numPatches; i++)
		{
			pPatch->AddDisplayMapping(banks[sel].patchNames[i]);
		}
		pPatch->value = 0;
		readNewWave = true;
	}
	if (pPatch->tag != pPatch->GetSelection())
	{
		readNewWave = true;
	}
	if (readNewWave)
	{
		pPatch->tag = pPatch->GetSelection();
		pBank->tag = pBank->GetSelection();
		char temp[256], pack[256];
		settings.getPack(settings.getSelectedPackIndex(), pack);
		sprintf(temp, "%s%s", workDir, pack);
		FILE *allWaves = fopen(temp, "rb");
		setWaveFileOffset(allWaves, pBank->tag, pPatch->tag);
		osc->ReadFile(allWaves);
		fclose(allWaves);
	}
	endTransaction();
}

void WaveSynth::ReplaceStr(char *str, char replaceWhat, char replacement)
{
	for (char *c = str; *c != 0; c++)
		if (*c == replaceWhat) *c = replacement;
}

FILE * WaveSynth::getMacroDefinitionFile()
{
	return settings.getMacroDefinitionFile();
}


void WaveSynth::GetBanksAndPatches() {
	char bankId[256];
	currentPack = settings.getSelectedPackIndex();
	settings.getPackId(currentPack, bankId);
	IniFileReader *reader = new IniFileReader();
	reader->openFile(settings.getBankDefinitionFile());
	char temp[256];
	sprintf(temp, "%s:banks", bankId);
	numBanks = reader->readIntValue("general", temp);
	for (int iBank = 0; iBank < numBanks; iBank++) {
		char bankSegment[256];
		sprintf(bankSegment, "%s:bank_%d", bankId, iBank);
		banks[iBank].numPatches = reader->readIntValue(bankSegment, "patches");
		reader->readCharValue(bankSegment, "shortName", banks[iBank].name);
		reader->readCharValue(bankSegment, "name", banks[iBank].longName);
		for (int iPatch = 0; iPatch < banks[iBank].numPatches; iPatch++) {
			reader->readCharValueWithIndex(bankSegment, "shortName_", iPatch, banks[iBank].patchNames[iPatch]);
			reader->readCharValueWithIndex(bankSegment, "name_", iPatch, banks[iBank].longPatchNames[iPatch]);
		}
	}
	reader->closeFile();
	delete reader;
}

void WaveSynth::updateParams(int callerIndex)
{
	startTransaction();
	for (int oscid = 0; oscid < NUM_PARAM_SETS; oscid++)
	{
		if (callerIndex == P_SET(oscid, P_BANK) || callerIndex == P_SET(oscid, P_PATCH))
			ChangePatch(osc[oscid], params[P_SET(oscid, P_BANK)], params[P_SET(oscid, P_PATCH)]);
		else if (callerIndex == P_SET(oscid, P_ATTACK) || callerIndex == P_SET(oscid, P_DECAY) || callerIndex == P_SET(oscid, P_SUSTAIN) || callerIndex == P_SET(oscid, P_TAIL))
			osc[oscid]->SetEnvelope(params[P_SET(oscid, P_ATTACK)]->value, params[P_SET(oscid, P_DECAY)]->value, params[P_SET(oscid, P_SUSTAIN)]->value, params[P_SET(oscid, P_TAIL)]->value);
		else if (callerIndex == P_SET(oscid, P_DETUNE_OCT) || callerIndex == P_SET(oscid, P_DETUNE_CENT))
			osc[oscid]->SetPlaySpeedModifier(params[P_SET(oscid, P_DETUNE_OCT)]->GetSelection() - 3, params[P_SET(oscid, P_DETUNE_CENT)]->value - 0.5);
		else if (callerIndex == P_SET(oscid, P_CUTOFF) || callerIndex == P_SET(oscid, P_RESONANCE) ||
			callerIndex == P_SET(oscid, P_ENV_TO_CUTOFF))
			osc[oscid]->SetFilterValues(params[P_SET(oscid, P_CUTOFF)]->value, params[P_SET(oscid, P_RESONANCE)]->value,
				params[P_SET(oscid, P_ENV_TO_CUTOFF)]->value);
		else if (callerIndex == P_SET(oscid, P_DISTORTION))
			osc[oscid]->SetWaveShaping(params[P_SET(oscid, P_DISTORTION)]->value);
		else if (callerIndex == P_SET(oscid, P_LOOPING))
		{
			osc[oscid]->SetVoiceMode(params[P_SET(oscid, P_LOOPING)]->GetSelection());
		}
		else if (callerIndex == P_SET(oscid, P_LOOP_POSITION))
		{
			osc[oscid]->SetLoopingModifier(params[P_SET(oscid, P_LOOP_POSITION)]->value);
		}
		else if (callerIndex == P_SET(oscid, P_LFO_FREQ) || callerIndex == P_SET(oscid, P_LFO_TYPE) ||
			callerIndex == P_SET(oscid, P_LFO_TO_FILTER) || callerIndex == P_SET(oscid, P_LFO_TO_PITCH) ||
			callerIndex == P_SET(oscid, P_LFO_TO_LEVEL) || callerIndex == P_SET(oscid, P_LFO_TO_CUE))
			osc[oscid]->SetLFOParams(params[P_SET(oscid, P_LFO_TYPE)]->GetSelection(), params[P_SET(oscid, P_LFO_FREQ)]->value * 5 + 0.01,
				params[P_SET(oscid, P_LFO_TO_FILTER)]->value, params[P_SET(oscid, P_LFO_TO_PITCH)]->value, params[P_SET(oscid, P_LFO_TO_LEVEL)]->value,
				params[P_SET(oscid, P_LFO_TO_CUE)]->value);
		else if (callerIndex == P_SET(oscid, P_LEVEL) && params[P_SET(oscid, P_LEVEL)]->value < 0.001)
			osc[oscid]->MuteAll();
	}
	if (callerIndex == P_DELAY_TIME)
	{
		for (int ch = 0; ch < 2; ch++)
			delay[ch]->setTime(params[P_DELAY_TIME]->value * 1000);
	}
	else if (callerIndex == P_DELAY_FEED)
	{
		for (int ch = 0; ch < 2; ch++)
			delay[ch]->setFeedback(params[P_DELAY_FEED]->value);
	}
	else if (callerIndex == P_FLANGER_FREQ || callerIndex == P_FLANGER_DELAY || callerIndex == P_FLANGER_DEPTH)
	{
		for (int ch = 0; ch < 2; ch++)
			flanger[ch]->SetParams(params[P_FLANGER_FREQ]->value * 5 + 0.01, params[P_FLANGER_DELAY]->value, params[P_FLANGER_DEPTH]->value);
	}
	else if (callerIndex == P_GLOBAL_LFO_FREQ)
		LFO->setFrequency(params[P_GLOBAL_LFO_FREQ]->value * 5);
	else if (callerIndex == P_GLOBAL_LFO_TYPE)
		LFO->setType(params[P_GLOBAL_LFO_TYPE]->GetSelection());
	else if (callerIndex == P_GLOBAL_LOW_CUT)
	{
		for (int oscN = 0; oscN < NUM_PARAM_SETS; oscN++)
			osc[oscN]->SetDCCut(params[P_GLOBAL_LOW_CUT]->value);
	}

	endTransaction();
}


float WaveSynth::getParameter(VstInt32 index)
{
	if (index >= NUM_PARAMS)
		return 0;
	return (float)params[index]->value;
}
void WaveSynth::setParameter(VstInt32 index, float value)
{
	if (index >= NUM_PARAMS)
		return;
	params[index]->value = value;
	updateParams(index);
	if (isTransactionOver())
		((EditorGui*)(this->editor))->setParameter(index, value);
}
void WaveSynth::getParameterName(VstInt32 index, char *label)
{
	if (index >= NUM_PARAMS)
		return;
	strcpy_s(label, 8, params[index]->name);
}
void WaveSynth::getParameterDisplay(VstInt32 index, char *text)
{
	params[index]->GetDisplayText(text, 8);
}
void WaveSynth::getParameterLabel(VstInt32 index, char *label)
{
	strcpy_s(label, 8, params[index]->label);
}
bool WaveSynth::getEffectName(char* name)
{
	strcpy_s(name, 32, "Little, ugly gnome");
	return true;
}
bool WaveSynth::getProductString(char* text)
{
	getEffectName(text);
	return true;
}
bool WaveSynth::getVendorString(char* text)
{
	strcpy_s(text, 64, "(c) 2016-2022 Joonas Salonpaa");
	return true;
}
void WaveSynth::getBankAndPatchName(char *buff, double bank, double patch)
{
	int ibank = (int)(numBanks * 0.99 * bank);
	int ipatch = (int)(banks[ibank].numPatches * 0.99 * patch);
	sprintf(buff, "%s: %s", banks[ibank].longName, banks[ibank].longPatchNames[ipatch]);
}

VstInt32 WaveSynth::processEvents(VstEvents* events)
{
	for (int i = 0; i < events->numEvents; i++)
	{
		if (!(events->events[i]->type&kVstMidiType))
			continue;
		VstMidiEvent *midievent = (VstMidiEvent*)(events->events[i]);
		if ((char)midievent->midiData[0] >> 4 == MIDI_NOTE_OFF)
		{
			for (int oscid = 0; oscid < NUM_PARAM_SETS; oscid++)
				osc[oscid]->NoteOff(midievent->midiData[1]);
		}
		else if ((char)midievent->midiData[0] >> 4 == MIDI_NOTE_ON)
		{
			int unisonCount = (int)(params[P_UNISON_COUNT]->value * 0.99 * NUM_UNISON_VOICES + 1);
			int seqMode = params[P_SEQUENCE]->GetSelection();
			for (int oscid = 0; oscid < NUM_PARAM_SETS; oscid++)
			{
				if (params[P_SET(oscid, P_LEVEL)]->value < 0.001) continue;
				if (seqMode != SEQ_MODE_LAYER && oscid != OscSequences[sequence]) continue;

				if (unisonCount <= 1)
				{
					osc[oscid]->NoteOn(midievent->midiData[1], midievent->midiData[2]);
				}
				else
				{
					double detuneStep = params[P_UNISON_DETUNE]->value / unisonCount;
					double detune = -0.5 * params[P_UNISON_DETUNE]->value;
					for (int unisonIdx = 0; unisonIdx < unisonCount; unisonIdx++)
					{
						osc[oscid]->NoteOn(midievent->midiData[1], midievent->midiData[2], detune, (double)(rand() % 30000) / 30000.0);
						detune += detuneStep;
					}
				}
			}
			// Advance sequence
			if (seqMode != SEQ_MODE_LAYER)
			{
				sequence++;
				if (seqMode == SEQ_MODE_UP && sequence >= 3) sequence = 0;
				else if (seqMode == SEQ_MODE_UP_AND_DOWN && sequence >= 4) sequence = 0;
				else if (seqMode == SEQ_MODE_RANDOM) sequence = predictableRandom->next(3);
			}
		}
		else if ((char)midievent->midiData[0] >> 4 == MIDI_PITCH_BEND)
		{
			double pitchBendValue = midievent->midiData[2] * 128 + midievent->midiData[1];
			// http://dsp.stackexchange.com/questions/1645/converting-a-pitch-bend-midi-value-to-a-normal-pitch-value
			double pitchMultiplier = pow(2, (pitchBendValue - MIDI_PITCH_BEND_CENTER) / MIDI_PITCH_BEND_CENTER);
			for (int oscid = 0; oscid < NUM_PARAM_SETS; oscid++)
			{
				osc[oscid]->SetPitchMultiplier(pitchMultiplier);
			}
		}
		else if ((char)midievent->midiData[0] >> 4 == MIDI_CC)
		{
			CChanges->onControllerChange(midievent->midiData[1], midievent->midiData[2]);
		}
		else if ((char)midievent->midiData[0] >> 4 == MIDI_AFTERTOUCH)
		{
			CChanges->onControllerChange(AFTERTOUCH_CC_NUMBER, midievent->midiData[1]);
		}
	}
	return 0;
}

void WaveSynth::processReplacing(float **inputs, float **outputs,
	VstInt32 sampleFrames) {
	VstTimeInfo* timeInfo = getTimeInfo(kVstTransportChanged);
	if (timeInfo != NULL)
	{
		if ((timeInfo->flags & kVstTransportChanged) && !(timeInfo->flags & kVstTransportPlaying))
		{
			sequence = 0;
			predictableRandom->seed(randomSeed);
		}
	}
	const double lfoStreoOffset = params[P_GLOBAL_LFO_STEREO_OFFSET]->value * 0.5,
		totalLevel = params[P_TOTAL_LEVEL]->value,
		lfoToLevel = params[P_GLOBAL_LFO_TO_LEVEL]->value,
		lfoToFlanger = params[P_GLOBAL_LFO_TO_FLANGER]->value,
		flangerWet = params[P_FLANGER_WET]->value,
		delayWet = params[P_DELAY_WET]->value,
		lfoToDelay = params[P_GLOBAL_LFO_TO_DELAY]->value,
		lfoToClip = params[P_GLOBAL_LFO_TO_CLIP]->value,
		softClip = params[P_SOFT_CLIP]->value,
		lfoToPan = params[P_GLOBAL_LFO_TO_PAN]->value,
		lfoToOscLfo = params[P_GLOBAL_LFO_TO_OSC_LFO]->value;

	const auto chBufSz = sampleFrames * oversampling;
	const auto outputBufSz = chBufSz * 2;
	auto outputBuf = std::make_unique<float[]>(outputBufSz);

	// Real processing goes here
	for (int oscid = 0; oscid < NUM_PARAM_SETS; oscid++)
	{
		const double voiceVolumeLevel = params[P_SET(oscid, P_LEVEL)]->value,
			voicePan = params[P_SET(oscid, P_PAN)]->value;

		if (voiceVolumeLevel < 0.001) 
		{
			continue;
		}

		osc[oscid]->setMemoryLock(true);
		// Processing each oscillator for the whole buffer at a time vastly improves performance
		// maybe it's a CPU caching thing?
		for (int i = 0; i < chBufSz; i++)
		{
			if (oscid == 0)
				LFO->calculate();
			const double monoOut = osc[oscid]->process(lfoToOscLfo * LFO->value) * voiceVolumeLevel;
			const double panCh1 = voicePan * (1 - LFO->value * lfoToPan);
			const double panCh0 = 1 - panCh1;
			outputBuf[i] += (float)(monoOut * panCh0);
			outputBuf[i + chBufSz] += (float)(monoOut * panCh1);
		}
		osc[oscid]->setMemoryLock(false);
	}
	for (int i = 0; i < chBufSz; i++)
	{
		for (int ch = 0; ch < 2; ch++)
		{
			double lfoValue;
			if (ch == 0)
			{
				lfoValue = LFO->value;
			}
			else
			{
				lfoValue = LFO->ValueWithPhaseShift(lfoStreoOffset);
			}
			double tempOut = (outputBuf[ch * chBufSz + i] * totalLevel * (1 - lfoValue * lfoToLevel));
			const double flangerAmount = flangerWet * (1 - lfoValue * lfoToFlanger);
			tempOut = -flanger[ch]->process(tempOut) * flangerAmount + tempOut * (1 - flangerAmount);
			tempOut += (delay[ch]->process(tempOut) * delayWet * (1 - lfoValue * lfoToDelay));
			double clip = softClip * (1 - lfoValue * lfoToClip);
			if (clip > 1e-4)
			{
				tempOut = tempOut * (1 - clip) + (tempOut / (fabs(tempOut) + 1)) * clip;
			}
			if (ch == 0)
				tempOut = downsamplingFilter1.process(tempOut);
			else
				tempOut = downsamplingFilter2.process(tempOut);
			outputBuf[ch * chBufSz + i] = (float)tempOut;

		}
	}
	int outputBufIdx = 0;
	for (int i = 0; i < sampleFrames; i++)
	{
		outputs[0][i] = outputBuf[i * oversampling];
		outputs[1][i] = outputBuf[i * oversampling + chBufSz];
	}
}
