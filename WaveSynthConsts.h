#pragma once
#define NUM_PARAMS 88
#define P_TOTAL_LEVEL 66
#define P_UNISON_COUNT 67
#define P_UNISON_DETUNE 68
#define P_DELAY_TIME 69
#define P_DELAY_FEED 70
#define P_DELAY_WET 71
#define P_SEQUENCE 72
#define P_SOFT_CLIP 73
#define P_FLANGER_FREQ 74
#define P_FLANGER_DELAY 75
#define P_FLANGER_DEPTH 76
#define P_FLANGER_WET 77
#define P_GLOBAL_LFO_FREQ 78
#define P_GLOBAL_LFO_TYPE 79
#define P_GLOBAL_LFO_TO_PAN 80
#define P_GLOBAL_LFO_TO_LEVEL 81
#define P_GLOBAL_LFO_TO_DELAY 82
#define P_GLOBAL_LFO_TO_FLANGER 83
#define P_GLOBAL_LOW_CUT 84
#define P_GLOBAL_LFO_TO_OSC_LFO 85
#define P_GLOBAL_LFO_TO_CLIP 86
#define P_GLOBAL_LFO_STEREO_OFFSET 87
#define SET_PARAMS_START 0
#define SET_PARAMS_COUNT 22
#define NUM_PARAM_SETS 3
#define TOTAL_SET_PARAMS_COUNT (SET_PARAMS_COUNT*NUM_PARAM_SETS)
#define P_BANK 0
#define P_PATCH 1
#define P_LOOPING 2
#define P_LOOP_POSITION 3
#define P_DETUNE_OCT 4
#define P_DETUNE_CENT 5
#define P_CUTOFF 6
#define P_RESONANCE 7
#define P_DISTORTION 8
#define P_ATTACK 9
#define P_DECAY 10
#define P_SUSTAIN 11
#define P_TAIL 12
#define P_ENV_TO_CUTOFF 13
#define P_LFO_TYPE 14
#define P_LFO_FREQ 15
#define P_LFO_TO_PITCH 16
#define P_LFO_TO_CUE 17
#define P_LFO_TO_FILTER 18
#define P_LFO_TO_LEVEL 19
#define P_PAN 20
#define P_LEVEL 21

const char longParamNames[SET_PARAMS_COUNT][20] = {
	"Bank",
	"Patch",
	"Looping",
	"Cue",
	"Octave",
	"Detune",
	"Cutoff",
	"Peaking",
	"Distort",
	"Attack",
	"Decay",
	"Sustain",
	"Tail",
	"E->Filter",
	"Type",
	"Rate",
	"L->Pitch",
	"L->Cue",
	"L->Filter",
	"L->Level",
	"Pan",
	"Level"
};

#define P_SET(paramNumber, paramId) (SET_PARAMS_START + SET_PARAMS_COUNT * (paramNumber) + (paramId))

#define MIDI_NOTE_OFF -8
#define MIDI_NOTE_ON -7
#define MIDI_PITCH_BEND -2
#define MIDI_PITCH_BEND_CENTER 8192
#define MIDI_PITCH_BEND_OCTAVE 2048
#define MIDI_CC -5
#define MIDI_AFTERTOUCH -3

#define AFTERTOUCH_CC_NUMBER 1000

//#define WORK_DIR ".\\wavesynth_resources\\"
const int OscSequences[8] = { 0, 1, 2, 1, 2, 3, 1, 2 };

#define NUM_UNISON_VOICES 32

#define SEQ_MODE_LAYER 0
#define SEQ_MODE_UP 1
#define SEQ_MODE_UP_AND_DOWN 2
#define SEQ_MODE_RANDOM 3

#define CURRENT_VERSION 4