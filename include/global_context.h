#ifndef GLOBAL_CONTEXT_H
#define GLOBAL_CONTEXT_H

#include <stdlib.h>

typedef enum BodyInfoMemberIdx {
  BI_MASS = 0,
  BI_POS_X = 1,
  BI_POS_Y = 2,
  BI_SPD_X = 3,
  BI_SPD_Y = 4,
} BodyInfoMemberIdx;

typedef double BodyInfo;

typedef struct GlobalContext {
  size_t m_num_bodies;
  size_t m_num_steps;
  double m_dt;
  BodyInfo *m_bodies;
} GlobalContext;

BodyInfo* global_ctx_access_body(GlobalContext const *const iptr, size_t idx);
double* global_ctx_body_access_member(BodyInfo *const body_info_ptr, BodyInfoMemberIdx idx);

GlobalContext global_ctx_parse(char const *const path);
void global_ctx_free(GlobalContext *const ptr_to_auto_var);
void global_ctx_log(GlobalContext const *const iptr);
char global_ctx_is_null(GlobalContext const *const iptr);

#endif // GLOBAL_CONTEXT_H