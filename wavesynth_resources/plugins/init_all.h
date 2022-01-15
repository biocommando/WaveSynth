#ifndef INIT_ALL_H
#define INIT_ALL_H
#include "gnome.h"
/*
// parameters (optional): skip_synth_1 skip_synth_2 skip_synth_3 skip_all_synths skip_global
// 1 = true, 0 = false

include $/legacy.js

inline sub init_all

    def skip_synth_1
    def skip_synth_2
    def skip_synth_3
    def skip_all_synths
    def skip_global

    if skip_all_synths == 0
        for index in 1 to 3
            resolve_question_mark skip_synth_?
            if skip_synth_? == 0
PQ("Bank_?")  = 0;
PQ("Patch_?")  = 0;
PQ("Loop?")  = 0.5;
PQ("CuePos?")  = 0;
PQ("Octave?")  = 0.5;
PQ("Detune?")  = 0.5;
PQ("FCut_?")  = 1;
PQ("FRes_?")  = 0;
PQ("Wavshp?")  = 0;
PQ("Attack?")  = 0;
PQ("Decay?")  = 0;
PQ("Sust.?")  = 1;
PQ("Tail.?")  = 0;
PQ("E2Cut_?")  = 0;
PQ("LFOShp?")  = 0;
PQ("LFOFrq?")  = 0;
PQ("LFO2Pt?")  = 0;
PQ("LFO2Cu?")  = 0;
PQ("LFO2Fl?")  = 0;
PQ("LFO2Lv?")  = 0;
PQ("Pan_?")  = 0.5;
PQ("Level_?")  = 1;
            endif
        loop
    endif

    if skip_global == 0
        Level = 0.5
        Unison = 0
        UDetune = 0
        DelayTm = 0
        DlyFeed = 0
        DlyWet = 0
        OscSeq = 0
        SoftClp = 0
        FlngFrq = 0
        FlngDly = 0
        FlngDpt = 0
        FlngWet = 0
        LfoFreq = 0
        LfoType = 0
        L2Pan = 0
        L2Level = 0
        L2Delay = 0
        L2Flang = 0
        LowCut = 0
        L2LFOs = 0
        L2Clip = 0
        LStereo = 0
    endif
return
*/

void init_all(int skip_synth_1, int skip_synth_2, int skip_synth_3, int skip_all_synths, int skip_global)
{
    int skip_synth[3] = {
        skip_synth_1,
        skip_synth_2,
        skip_synth_3,
    };
    if (!skip_all_synths)
    {
        FOR_EACH_PSET
        {
            if (!skip_synth[pindex - 1])
            {
                PQ("Bank_?") = 0;
                PQ("Patch_?") = 0;
                PQ("Loop?") = 0.5;
                PQ("CuePos?") = 0;
                PQ("Octave?") = 0.5;
                PQ("Detune?") = 0.5;
                PQ("FCut_?") = 1;
                PQ("FRes_?") = 0;
                PQ("Wavshp?") = 0;
                PQ("Attack?") = 0;
                PQ("Decay?") = 0;
                PQ("Sust.?") = 1;
                PQ("Tail.?") = 0;
                PQ("E2Cut_?") = 0;
                PQ("LFOShp?") = 0;
                PQ("LFOFrq?") = 0;
                PQ("LFO2Pt?") = 0;
                PQ("LFO2Cu?") = 0;
                PQ("LFO2Fl?") = 0;
                PQ("LFO2Lv?") = 0;
                PQ("Pan_?") = 0.5;
                PQ("Level_?") = 1;
            }
        }
    }
    if (!skip_global)
    {
        PS("Level") = 0.5;
        PS("Unison") = 0;
        PS("UDetune") = 0;
        PS("DelayTm") = 0;
        PS("DlyFeed") = 0;
        PS("DlyWet") = 0;
        PS("OscSeq") = 0;
        PS("SoftClp") = 0;
        PS("FlngFrq") = 0;
        PS("FlngDly") = 0;
        PS("FlngDpt") = 0;
        PS("FlngWet") = 0;
        PS("LfoFreq") = 0;
        PS("LfoType") = 0;
        PS("L2Pan") = 0;
        PS("L2Level") = 0;
        PS("L2Delay") = 0;
        PS("L2Flang") = 0;
        PS("LowCut") = 0;
        PS("L2LFOs") = 0;
        PS("L2Clip") = 0;
        PS("LStereo") = 0;
    }
}

#endif