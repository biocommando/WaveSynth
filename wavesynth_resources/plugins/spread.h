/*
// Little, ugly gnome VST synth script file
// Copyrights 2016 by Joonas Salonpaa

// Spreads the oscillators in the stereo image, takes parameters:
// %width, %detune and %spread_mode
// spread modes: 1 = pan and detune, 2 = only pan, 3 = only detune

include $/legacy.js

sub spread
    spread_script_width = 0.5 - width
    spread_script_detune = 0.5 - detune
    for index in 1 to 3
        if spread_mode == 3
            spread_script_width = 0.5
        endif
%		Pan_? = spread_script_width
        spread_script_width = spread_script_width + width

        if spread_mode == 2
            spread_script_detune = 0.5
        endif
%		Detune? = spread_script_detune
        spread_script_detune = spread_script_detune + detune
    loop

return
*/

#ifndef SPREAD_H
#define SPREAD_H

#include "gnome.h"

// spread modes: 1 = pan and detune, 2 = only pan, 3 = only detune
void spread(double width, double detune, int spread_mode)
{
    double _width = 0.5 - width;
    double _detune = 0.5 - detune;
    FOR_EACH_PSET
    {
        if (spread_mode == 3)
            _width = 0.5;
        PQ("Pan_?") = _width;
        _width += width;

        if (spread_mode == 2)
            _detune = 0.5;
        PQ("Detune?") = _detune;
        _detune += detune;
    }
}
#endif