#include "init_all.h"
#include "spread.h"

//' This code will create an "orchestral" sound

PLUGIN()
{
    init_all(0, 0, 0, 0, 0);
    double width = random() * 0.5;
    double detune = random() * 0.2;
    detune = detune + 0.08;
    spread(width, detune, 1);

    double CutOff = random() * 0.5 + 0.5;
    double Resonance = random() * 0.9;

    params_as_integer("Bank_?");
    params_as_integer("Patch_?");
    params_as_integer("Octave?");
    params_as_integer("Loop?");

    FOR_EACH_PSET
    {
        PQ("Bank_?") = 5;
        PQ("Patch_?") = random() * 8;
        PQ("FCut_?") = CutOff;
        PQ("FRes_?") = Resonance;
        PQ("Octave?") = pindex + 1;
        PQ("Loop?") = (int)(1.5 + random());
        if (PQ("Loop?") == 2)
        {
            PQ("Tail.?") = random() * 0.7;
            PQ("Tail.?") = PQ("Tail.?") + 0.3;
        }
        PQ("CuePos?") = random();
        PQ("Attack?") = random();
        PQ("Sust.?") = random();
        PQ("Decay?") = random();
        PQ("E2Cut_?") = random();
    }
    P(FlngFrq) = random();
    P(FlngDly) = random();
    P(FlngWet) = random();
    P(FlngDpt) = random();

    mark_interger_params();
}