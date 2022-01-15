#include "gnome.h"
#include "spread.h"
#include "ismelodic.h"

PLUGIN()
{
    double width = random() * 0.125 + 0.125;
    double detune = random() * 0.2;
    double spread_mode = 1;
    spread(width, detune, spread_mode);
    double attack = random() * 0.5 + 0.49;
    // slow attack
    P(Attack1) = attack;
    P(Attack3) = attack;
    // the centered osc will be faster
    P(Attack2) = attack * 0.5;
    P(LFORate) = random() * 0.3333;
    params_as_integer("Bank_?");
    params_as_integer("Patch_?");
    params_as_integer("Loop?");
    param_as_integer("LfoType");
    FOR_EACH_PSET
    {
        if (P(wavePackIndexReadOnly) == 0)
        {
            do
            {
                PQ("Bank_?") = random() * 7 + 1;
                PQ("Patch_?") = random() * 8;
            } while (!is_melodic_selected());
        }
        else
        {
            PQ("Bank_?") = random() * 8;
            PQ("Patch_?") = random() * 8;
        }
        double cutoff = random() * 0.65 + 0.15;
        PQ("FCut_?") = cutoff;
        PQ("FRes_?") = 1 - cutoff;
        double waveshape = random() * (1 - cutoff) * 0.5;
        while (waveshape < 0.1)
        {
            waveshape += 0.001;
            waveshape *= 1.1;
        }
        PQ("Wavshp?") = waveshape;
        PQ("E2Cut_?") = random();
        PQ("LFOShp?") = random();
        PQ("LFOFrq?") = P(LFORate) * pindex;
        PQ("LFO2Pt?") = random() * 0.08;
        PQ("LFO2Cu?") = random() * 0.1;
        double lfoval = random() * 0.5;
        PQ("LFO2Fl?") = lfoval * lfoval;
        lfoval = random() * 0.5;
        PQ("LFO2Lv?") = lfoval * lfoval;
        PQ("CuePos?") = random() * 0.1;
        double octaveSpread = random() * 0.42;
        PQ("Octave?") = octaveSpread + 0.29;
        PQ("Decay?") = random() * 0.5;
        PQ("Decay?") = PQ("Decay?") + 0.1;
        PQ("Sust.?") = random() * 0.3;
        PQ("Sust.?") = PQ("Sust.?") + 0.3;
        PQ("Tail.?") = PQ("Attack?") * 0.5;
        PQ("Level_?") = 1;
        PQ("Loop?") = 2;
    }

    // *** global params ***

    // Unison
    double unison = random() * 0.2 + 0.05;
    P(unison) = unison;
    P(UDetune) = unison + unison * random() * 0.5;
    // Levels
    double level = 0.5 - unison;
    P(level) = level;
    P(SoftClp) = level;

    // Delay
    P(DelayTm) = random() * 0.5;
    P(DlyFeed) = random() * 0.5;
    P(DlyWet) = random() * 0.5;

    P(OscSeq) = 0;

    // Flanger
    P(FlngFrq) = random() * 0.7 + 0.3;
    P(FlngDly) = random() * 0.9 + 0.1;
    P(FlngWet) = random() * 0.8 + 0.2;
    P(FlngDpt) = random() * 0.2 + 0.05;

    // Modulation
    P(LfoFreq) = (random() * 0.2 + 0.05) / 5;
    P(LfoType) = random() * 2;
    P(L2Pan) = random();
    P(L2Level) = random() * 0.2;
    P(L2Delay) = random() * 0.5;
    P(L2Flang) = random() * 0.5;
    P(L2LFOs) = random();
    P(LowCut) = 0;
    P(L2Clip) = random();
    double l2stereo = random() * 0.8;
    P(LStereo) = 0.2 + l2stereo;
    P(L2LFOs) = random() * 0.5;

    mark_interger_params();
}