#include "gnome.h"

PLUGIN()
{
    double octave_index = 0;
    FOR_EACH_PSET
    {
        octave_index = (int)(PQ("Octave?") * 7 * 0.99);
        octave_index = octave_index + P(transpose_octave_direction);
        PQ("Octave?") = (octave_index + 0.5) / 7;
    }
}