#include "gnome.h"

PLUGIN()
{
    FOR_EACH_PSET
    {
        PQ("Bank_?") = random();
        PQ("Patch_?") = random();
    }
}