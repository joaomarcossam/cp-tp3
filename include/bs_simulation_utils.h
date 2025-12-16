#ifndef BS_SIMULATION_UTILS
#define BS_SIMULATION_UTILS

#include <global_context.h>
#include <tuple_double_double.h>
#include <tuple_template.h>

#define G 6.67430e-11

Tuple(double, double) get_gforce(BodyInfo *const b1, BodyInfo *const b2);

#endif // BS_SIMULATION_UTILS