#include <bs_multithread.h>
#include <bs_simulation_utils.h>
#include <global_context.h>
#include <stdint.h>
#include <tuple_double_double.h>
#include <tuple_template.h>
#include <stdlib.h>
#include <omp.h>
#include <mpi.h>
#include <macros.h>
#include <limits.h>

static void get_acceleration_array(
  GlobalContext const *const ctx,
  Tuple(double, double) * acc_vector,
  uint64_t slice_idx,
  uint64_t slice_len) {

  Tuple(double, double) force_sum;
  Tuple(double, double) current_gforce;

  #pragma omp parallel for default(none) \
    shared(ctx, acc_vector,slice_len,slice_idx) \
    private(force_sum, current_gforce)
  for (uint64_t i = 0; i < slice_len; i++) {
    force_sum = (Tuple(double, double)){0.0, 0.0};
    for (uint64_t j = 0; j < slice_len; j++) {
      if (i == j)
        continue;
      current_gforce =
        get_gforce(
          global_ctx_access_body(ctx, slice_idx + i)
          , global_ctx_access_body(ctx, slice_idx + j));
      force_sum.m_1 += current_gforce.m_1;
      force_sum.m_2 += current_gforce.m_2;
    }
    acc_vector[i].m_1 =
      force_sum.m_1 / *global_ctx_body_access_member(global_ctx_access_body(ctx, slice_idx + i), BI_MASS);
    acc_vector[i].m_2 =
      force_sum.m_2 / *global_ctx_body_access_member(global_ctx_access_body(ctx, slice_idx + i), BI_MASS);
  }
}

static void update_state(
  GlobalContext *const state,
  Tuple(double, double) *acceleration_vector,
  uint64_t slice_idx,
  uint64_t slice_len) {
  for (uint64_t i = 0; i < slice_len; i++) {
    BodyInfo* body = global_ctx_access_body(state, slice_idx + i);
    *global_ctx_body_access_member(body, BI_POS_X) = (*global_ctx_body_access_member(body, BI_POS_X) + *global_ctx_body_access_member(body, BI_SPD_X) * state->m_dt + acceleration_vector[i].m_1 * (state->m_dt * state->m_dt / 2));
    *global_ctx_body_access_member(body, BI_POS_Y) = (*global_ctx_body_access_member(body, BI_POS_Y) + *global_ctx_body_access_member(body, BI_SPD_Y) * state->m_dt + acceleration_vector[i].m_2 * (state->m_dt * state->m_dt / 2));
    *global_ctx_body_access_member(body, BI_SPD_X) = (*global_ctx_body_access_member(body, BI_SPD_X) + acceleration_vector[i].m_1 * state->m_dt);
    *global_ctx_body_access_member(body, BI_SPD_Y) = (*global_ctx_body_access_member(body, BI_SPD_Y) + acceleration_vector[i].m_2 * state->m_dt);
  }
}

static void sync_buffer(
  double *const global_buffer
  , uint64_t global_buffer_len
  , uint64_t const *const recvbuf
  , uint64_t const *const displs
  , int num_procs
  , int caller_rank)
{
  static const char ELEMS_PER_BODY = 5;
  static const int MAX_ELEMS_PER_CALL = INT_MAX;
  for (uint64_t base = 0; base < global_buffer_len; base += MAX_ELEMS_PER_CALL) {
    uint64_t block_elems =
      (global_buffer_len - base > MAX_ELEMS_PER_CALL)
      ? MAX_ELEMS_PER_CALL
      : global_buffer_len - base;
    int recvbuf_i[num_procs];
    int displs_i[num_procs];
    for (int p = 0; p < num_procs; ++p) {
      uint64_t start = displs[p];
      uint64_t end   = start + recvbuf[p];
      uint64_t overlap_start = start > base ? start : base;
      uint64_t overlap_end =
        end < (base + block_elems)
        ? end
        : (base + block_elems);
      uint64_t overlap =
        overlap_end > overlap_start
        ? overlap_end - overlap_start
        : 0;
      recvbuf_i[p]  = (int)overlap;
      displs_i[p]   = (int)(overlap_start - base);
    }
    uint64_t my_start = displs[caller_rank];
    uint64_t my_end   = my_start + recvbuf[caller_rank];
    uint64_t send_overlap_start =
        my_start > base ? my_start : base;
    uint64_t send_offset =
        send_overlap_start > my_start
        ? send_overlap_start - my_start
        : 0;
    double *sendbuf = global_buffer + my_start + send_offset;
    int sendcount   = recvbuf_i[caller_rank];
    MPI_Allgatherv(
      sendbuf,
      sendcount,
      MPI_DOUBLE,
      global_buffer + base,
      recvbuf_i,
      displs_i,
      MPI_DOUBLE,
      MPI_COMM_WORLD);
  }
}

void iterate_states_parallel(
  GlobalContext *const init_state
  , uint64_t slice_idx
  , uint64_t slice_len
  , uint64_t const *const recvbuf
  , uint64_t const *const displs
  , int num_procs
  , int called_by) {
    
  Tuple(double, double) *acc_vector = malloc(sizeof(Tuple(double, double)) * slice_len);
  if(! acc_vector)
    return;
  for(uint64_t i = 0; i < slice_len; i++) {
    get_acceleration_array(init_state, acc_vector, slice_idx, slice_len);
    update_state(init_state, acc_vector, slice_idx, slice_len);
    sync_buffer(
      init_state->m_bodies
      , init_state->m_num_bodies * 5
      , recvbuf
      , displs
      , num_procs
      , called_by);
  }
  free(acc_vector);
}