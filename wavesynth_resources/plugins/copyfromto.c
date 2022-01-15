#include "copyfromto.h"

PLUGIN()
{
    int to = P(to);
    if (to < 4)
        copy_from_to(P(from), to);
    else
    {
        for (to = 1; to <= 3; to++)
            copy_from_to(P(from), to);
    }
}