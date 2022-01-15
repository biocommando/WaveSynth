/*#define LOG_FILE "D:\\code\\c\\WaveSynth\\TestVST\\wavesynth_resources\\plugins\\log.txt"
#define ENABLE_LOG_TRACE*/
#include "init_all.h"
#include "delaysync.h"
#include "unison.h"
#include "ismelodic.h"

PLUGIN()
{
    init_all(0, 0, 0, 0, 0);
    double dist = random() * 0.6;
    dist = dist * dist;
    double tail = 0.1 * random();

    double fenv = random() * 0.7 + 0.3;

    params_as_integer("Bank_?");
    params_as_integer("Patch_?");
    params_as_integer("Octave?");
    params_as_integer("LFOShp?");

    FOR_EACH_PSET
    {
        PQ("Loop?") = 1;
        PQ("Wavshp?") = dist;
        PQ("Tail.?") = tail;

        PQ("E2Cut_?") = fenv;
        PQ("FCut_?") = fenv * 0.3 + 0.3;
        PQ("FRes_?") = fenv;
        if (PQ("FRes_?") > 0.88)
            PQ("FRes_?") = 0.88;
    }
    double detune = 0.2 * random() + 0.05;
    P(Detune2) = 0.5 - detune;
    P(Detune3) = 0.5 + detune;
    double sust = 0.3 * random();
    sust = sust + 0.3;
    double sawlevel = 0.5 + random();
    for (pindex = 2; pindex <= 3; pindex++)
    {
        PQ("Bank_?") = 7;
        PQ("Patch_?") = 2;
        if (P(wavePackIndexReadOnly) == 0)
        {
            PQ("Bank_?") = 0;
            PQ("Patch_?") = 1;
        }
        PQ("Octave?") = pindex;
        PQ("Pan_?") = PQ("Detune?");
        PQ("Decay?") = tail * 0.5;
        PQ("Sust.?") = sust;
        PQ("Level_?") = sawlevel;
    }

    P(Attack1) = random() * P(Decay2);

    if (P(wavePackIndexReadOnly) == 0)
    {
        do
        {
            P(Bank_1) = random() * 8;
            P(Patch_1) = random() * 8;
        } while (!is_melodic_selected());
    }
    else
    {
        P(Bank_1) = random() * 8;
        P(Patch_1) = random() * 8;
    }
    P(Octave1) = random() * 2 + 2;

    delay_sync(3);

    P(DlyFeed) = random() * 0.3;
    P(DlyFeed) = P(DlyFeed) + 0.2;
    P(DlyWet) = random() * 0.3;
    P(DlyWet) = P(DlyWet) + 0.3;

    P(FlngFrq) = random() + 0.2;
    P(FlngDly) = random() + 0.1;
    P(FlngWet) = random() + 0.2;
    P(FlngDpt) = random() * 0.25;

    double maxiter = random() * 3;
    while (maxiter > 0)
    {
        double unison_change = 2;
        double unison_change_multiplier = 1.5;
        unison(unison_change, unison_change_multiplier);
        maxiter -= 1;
    }
    P(UDetune) = P(UDetune) * (random() + 1);

    mark_interger_params();
}