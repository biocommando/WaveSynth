/*// Little, ugly gnome VST synth script file
// Copyrights 2016 by Joonas Salonpaa
// -----
// This script is used to copy synth 1 -> 2, 2 -> 3, and 3 -> 1 so that sequence start position can be altered without losing sanity

include $/copy_from_to.inc

entrypoint cycle-synth-stack

from = 3
to = 0
call copy_from_to
from = 2
to = 3
call copy_from_to
from = 1
to = 2
call copy_from_to
from = 0
to = 1
call copy_from_to

exit
*/

#include "copyfromto.h"

PLUGIN()
{
    copy_from_to(3, 0);
    copy_from_to(2, 3);
    copy_from_to(1, 2);
    copy_from_to(0, 1);
}