#ifndef DELAY_SYNC_H
#define DELAY_SYNC_H
#include "pluginutil.h"
void delay_sync(double delay_sync_length_quarters)
{
    double delay_sync_whole_note_length = 60 / P(tempo);
    double delay_sync_length_factor = 0.25 * delay_sync_length_quarters;
    P(DelayTm) = delay_sync_whole_note_length * delay_sync_length_factor;
    if (P(DelayTm) > 1)
        P(DelayTm) = 1;
}
#endif