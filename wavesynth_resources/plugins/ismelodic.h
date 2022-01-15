#ifndef IS_MELODIC_H
#define IS_MELODIC_H

#include "gnome.h"

// Little, ugly gnome VST synth script file
// Copyrights 2016 by Joonas Salonpaa

// This code will verify if the selected Bank/Patch combination can be used as a melodic instrument.
// Applies only to the "alternative" wavepack.
// #index must be set to the corresponding value in order to get this to work
// output: 1 if instrument can be used melodically, 0 otherwise
int is_melodic_selected()
{
    int is_melodic_selected = 1;
    int bank = PQ("Bank_?");
    int patch = PQ("Patch_?");
    // all noise types
    if (bank == 0 && patch >= 5)
        return 0;

    // percs
    if (bank == 3)
    {
        // hihats
        if (patch == 3 || patch == 4)
            return 0;
        // shaker
        if (patch == 5)
            return 0;
        // tambourine
        if (patch == 7)
            return 0;
    }
    // blips -> noisy
    if (bank == 7 && patch == 7)
            return 0;
    return 1;
}

#endif