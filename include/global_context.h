#ifndef GLOBAL_CONTEXT_H
#define GLOBAL_CONTEXT_H

#include <stdlib.h>
#include <tuple_double_double.h>

typedef struct BodyInfo {
  double m_mass;
  Tuple(double, double) m_pos;
  Tuple(double, double) m_spd;
} BodyInfo;

typedef struct GlobalContext {
  size_t m_num_bodies;
  size_t m_num_steps;
  double m_dt;
  BodyInfo *m_bodies;
} GlobalContext;

GlobalContext global_ctx_parse(char const *const path);
void global_ctx_free(GlobalContext *ptr_to_auto_var);
void global_ctx_log(GlobalContext const *const iptr);
char global_ctx_is_null(GlobalContext const *const iptr);
#endif // GLOBAL_CONTEXT_H