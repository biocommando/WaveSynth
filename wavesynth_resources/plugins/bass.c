#include "init_all.h"

/*
// Little, ugly gnome VST synth script file
// Copyrights 2016, 2021 by Joonas Salonpaa

include $/init-all.inc
include $/random.inc

entrypoint bass

generate_random_array.max = 20
call generate_random_array
call init_all

params_as_integer Bank_?, Patch_?, Octave?, LFOShp?, Loop?

Bank_1 = 1
Bank_2 = 1
Patch_1 = random() * 8
Patch_2 = Patch_1
Patch_3 = random() * 5
#detune = 0.1 * random()
#detune = 0.02 + #detune
Detune1 = 0.5 - #detune
Detune2 = 0.5 + #detune
Octave1 = 2
Octave2 = 2
Octave3 = 1
Loop1 = random() * 2
Loop2 = Loop1
Loop3 = 2
FCut_1 = random() * 0.4
FCut_1 = FCut_1 + 0.2
FCut_2 = FCut_1
FCut_3 = FCut_1 * 0.8
FRes_1 = random() - FCut_1
FRes_1 = FRes_1 * 2
if FRes_1 < 0.4
    FRes_1 = 0.4
endif
if FRes_1 > 0.93
    FRes_1 = 0.93
endif
FRes_2 = FRes_1
FRes_3 = FRes_1
Wavshp1 = random() * 0.25
Wavshp2 = Wavshp1
Wavshp3 = Wavshp1 * 2
Attack1 = 0.02 * random()
Attack2 = Attack1
Attack3 = Attack1
Decay1 = random() * 0.1
Decay2 = Decay1
Decay3 = Decay1
Sust.1 = random()
Sust.2 = Sust.1
Sust.3 = Sust.1
Tail.1 = random() * 0.9
Tail.2 = Tail.1
Tail.3 = random() * 0.01
Tail.3 = 0.01 + Tail.3
E2Cut_1 = random()
E2Cut_2 = E2Cut_1
E2Cut_3 = E2Cut_1
#LFO2Pt = random() * 0.05
#LFO2Fl = random() * 0.3
#LFORate = random() * 0.15
#LFORate = #LFORate + 0.05
for index in 1 to 3
    call random
    LFOShp? = random * 2
%	LFO2Pt? = #LFO2Pt
%	LFO2Fl? = #LFO2Fl
%	LFO2Lv? = #LFO2Fl * 0.5
%	LFOFrq? = #LFORate
loop
Level_3 = 1.5 - random()
LfoFreq = #LFORate
L2Level = random() * 0.2
L2Clip = random()
LfoType = random() * 0.49
Level = 0.5
SoftClp = random()

*/

PLUGIN()
{
    init_all(0, 0, 0, 0, 0);
    params_as_integer("Bank_?");
    params_as_integer("Patch_?");
    params_as_integer("Octave?");
    params_as_integer("LFOShp?");
    params_as_integer("Loop?");

    P(Bank_1) = 1;
    P(Bank_2) = 1;
    P(Patch_1) = random() * 8;
    P(Patch_2) = P(Patch_1);
    P(Patch_3) = random() * 5;
    double _detune = 0.1 * random();
    _detune = 0.02 + _detune;
    P(Detune1) = 0.5 - _detune;
    P(Detune2) = 0.5 + _detune;
    P(Octave1) = 2;
    P(Octave2) = 2,
    P(Octave3) = 1;
    P(Loop1) = random() * 2;
    P(Loop2) = P(Loop1);
    P(Loop3) = 2;
    P(FCut_1) = random() * 0.4;
    P(FCut_1) = P(FCut_1) + 0.2;
    P(FCut_2) = P(FCut_1);
    P(FCut_3) = P(FCut_1) * 0.8;
    P(FRes_1) = random() - P(FCut_1);
    P(FRes_1) = P(FRes_1) * 2;
    if (P(FRes_1) < 0.4)
        P(FRes_1) = 0.4;

    if (P(FRes_1) > 0.93)
        P(FRes_1) = 0.93;

    P(FRes_2) = P(FRes_1);
    P(FRes_3) = P(FRes_1);
    P(Wavshp1) = random() * 0.25;
    P(Wavshp2) = P(Wavshp1);
    P(Wavshp3) = P(Wavshp1) * 2;
    P(Attack1) = 0.02 * random();
    P(Attack2) = P(Attack1);
    P(Attack3) = P(Attack1),
    P(Decay1) = random() * 0.1;
    P(Decay2) = P(Decay1);
    P(Decay3) = P(Decay1);
    P(Sust .1) = random();
    P(Sust .2) = P(Sust .1);
    P(Sust .3) = P(Sust .1);
    P(Tail .1) = random() * 0.9;
    P(Tail .2) = P(Tail .1);
    P(Tail .3) = random() * 0.01;
    P(Tail .3) = 0.01 + P(Tail .3);
    P(E2Cut_1) = random();
    P(E2Cut_2) = P(E2Cut_1);
    P(E2Cut_3) = P(E2Cut_1);
    double _LFO2Pt = random() * 0.05;
    double _LFO2Fl = random() * 0.3;
    double _LFORate = random() * 0.15 + 0.05;
    FOR_EACH_PSET
    {
        PQ("LFOShp?") = random() * 2;
        PQ("LFO2Pt?") = _LFO2Pt;
        PQ("LFO2Fl?") = _LFO2Fl;
        PQ("LFO2Lv?") = _LFO2Fl * 0.5;
        PQ("LFOFrq?") = _LFORate;
    }
    P(Level_3) = 1.5 - random();
    P(LfoFreq) = _LFORate;
    P(L2Level) = random() * 0.2;
    P(L2Clip) = random();
    P(LfoType) = random() * 0.49;
    P(Level) = 0.5;
    P(SoftClp) = random();

    mark_interger_params();
}