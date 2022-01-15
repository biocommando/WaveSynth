#include "gnome.h"

double lfo_sync(double frequency, double noteDivision)
{
    double lfosync_noteLength = 60.0 / P(tempo) / noteDivision * 4;
    // to Hz (the range is 0.01 to 5.01 Hz)
    double lfosync_freq = (frequency * 5) + 0.01;
    double lfosync_length = (int)((1 / lfosync_freq) / lfosync_noteLength + 0.4999);
    lfosync_length = lfosync_length * lfosync_noteLength;
    if (lfosync_length < 0.001)
        lfosync_length = lfosync_noteLength;
    lfosync_freq = 1 / lfosync_length;
    return (lfosync_freq - 0.01) * 0.2;
}