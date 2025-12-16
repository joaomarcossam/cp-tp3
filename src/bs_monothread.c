#include <bs_monothread.h>
#include <bs_simulation_utils.h>
#include <global_context.h>
#include <tuple_double_double.h>
#include <tuple_template.h>
#include <stdlib.h>

static void get_acceleration_array(GlobalContext const *const ctx, Tuple(double, double) * acc_vector) {
  Tuple(double, double) force_sum;
  Tuple(double, double) current_gforce;

  for (uint64_t i = 0; i < ctx->m_num_bodies; i++) {
    force_sum = (Tuple(double, double)){0.0, 0.0};
    for (uint64_t j = 0; j < ctx->m_num_bodies; j++) {
      if (i == j)
        continue;
      current_gforce = get_gforce(global_ctx_access_body(ctx, i), global_ctx_access_body(ctx, j));
      force_sum.m_1 += current_gforce.m_1;
      force_sum.m_2 += current_gforce.m_2;
    }
    acc_vector[i].m_1 = force_sum.m_1 / *global_ctx_body_access_member(global_ctx_access_body(ctx, i), BI_MASS);
    acc_vector[i].m_2 = force_sum.m_2 / *global_ctx_body_access_member(global_ctx_access_body(ctx, i), BI_MASS);
  }
}

static void update_state(GlobalContext *const state, Tuple(double, double) * acceleration_vector) {
  for (uint64_t i = 0; i < state->m_num_bodies; i++) {
    BodyInfo* body = global_ctx_access_body(state, i);
    *global_ctx_body_access_member(body, BI_POS_X) = (*global_ctx_body_access_member(body, BI_POS_X) + *global_ctx_body_access_member(body, BI_SPD_X) * state->m_dt + acceleration_vector[i].m_1 * (state->m_dt * state->m_dt / 2));
    *global_ctx_body_access_member(body, BI_POS_Y) = (*global_ctx_body_access_member(body, BI_POS_Y) + *global_ctx_body_access_member(body, BI_SPD_Y) * state->m_dt + acceleration_vector[i].m_2 * (state->m_dt * state->m_dt / 2));
    *global_ctx_body_access_member(body, BI_SPD_X) = (*global_ctx_body_access_member(body, BI_SPD_X) + acceleration_vector[i].m_1 * state->m_dt);
    *global_ctx_body_access_member(body, BI_SPD_Y) = (*global_ctx_body_access_member(body, BI_SPD_Y) + acceleration_vector[i].m_2 * state->m_dt);
  }
}

void iterate_states_monothread(GlobalContext *const init_state) {
  Tuple(double, double) *acc_vector = malloc(sizeof(Tuple(double, double)) * init_state->m_num_bodies);
  if(! acc_vector)
    return;
  for(uint64_t i = 0; i < init_state->m_num_steps; i++) {
    get_acceleration_array(init_state, acc_vector);
    update_state(init_state, acc_vector);
  }
  free(acc_vector);
}