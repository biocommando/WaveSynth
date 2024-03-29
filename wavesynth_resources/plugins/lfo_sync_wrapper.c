#include "lfo_sync.h"

PLUGIN()
{
    int synth_skip[3] = {
        P(Synth_skip_1),
        P(Synth_skip_2),
        P(Synth_skip_3),
    };
    int global_skip = P(Synth_skip_global);

    FOR_EACH_PSET
    {
        if (synth_skip[pindex - 1] == 0)
        {
            double frequency = PQ("LFOFrq?");
            PQ("LFOFrq?") = lfo_sync(frequency, P(noteDivision));
        }
    }
    if (global_skip == 0)
    {
        double frequency = P(LfoFreq);
        P(LfoFreq) = lfo_sync(frequency, P(noteDivision));
    }
}