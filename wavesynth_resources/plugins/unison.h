#ifndef UNISON_H
#define UNISON_H

#include "pluginutil.h"
/*
// Little, ugly gnome VST synth script file
// Copyrights 2016 by Joonas Salonpaa

sub unison
    unison_change = unison_change / 32
    orig_unison_count = Unison * 32
    if orig_unison_count < 5
        Unison = Unison + unison_change
    else
        Unison = Unison * unison_change_multiplier
    endif

    if Unison < 0
        Unison = 0
    endif
    if Unison > 1
        Unison = 1
    endif

    UDetune = (2 - 2 / (1 + Unison)) * 0.7

    #temp = Unison * 32
    if #temp <= 0
        #temp = 1
    endif
    Level = 1 / #temp
return

entrypoint unison

call unison

exit
*/
void unison(double unison_change, double unison_change_multiplier)
{
    unison_change /= 32;
    double orig_unison_count = P(Unison) * 32;

    if (orig_unison_count < 5)
        P(Unison) = P(Unison) + unison_change;
    else
        P(Unison) = P(Unison) * unison_change_multiplier;

    if (P(Unison) < 0)
        P(Unison) = 0;
    if (P(Unison) > 1)
        P(Unison) = 1;
    P(UDetune) = (2 - 2 / (1 + P(Unison))) * 0.7;

    double temp = P(Unison) * 32;
    if (temp <= 0)
        temp = 1;
    P(Level) = 1 / temp;
}

#endif