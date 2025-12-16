#ifndef BS_MULTITHREAD_H
#define BS_MULTITHREAD_H

#include <global_context.h>

void iterate_states_parallel(
  GlobalContext *const init_state
  , uint64_t slice_idx
  , uint64_t slice_len
  , uint64_t const *const recvbuf
  , uint64_t const *const displs
  , int num_procs
  , int called_by);

#endif // BS_MULTITHREAD_H