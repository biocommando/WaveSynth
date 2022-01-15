#include "unison.h"

PLUGIN()
{
    unison(P(unison_change), P(unison_change_multiplier));
}