#ifndef _UNIVERSAL_GRAVITY_H
#define _UNIVERSAL_GRAVITY_H

#define G 6.67430e-11

#include <global_context.h>
#include <tuple_double_double.h>

void iterate_states(GlobalContext *const init_state);

#endif // _UNIVERSAL_GRAVITY_H