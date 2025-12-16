#ifndef PARALLEL
#include <stdio.h>
#include <stdlib.h>
#include <global_context.h>
#include <bs_monothread.h>
#include <macros.h>

int main(int argc, char **argv) {
  if(argc < 2) {
    fprintf(stderr, "Missing file path argument: aborting execution;\n");
    return EXIT_FAILURE;
  }
  GlobalContext info = global_ctx_parse(argv[1]);
  if(global_ctx_is_null(&info)) {
    fprintf(stderr, "Algorithm input malformed: aborting execution;\n");
    return EXIT_FAILURE;
  }
  #ifdef LOG_CODE_ENABLED
  printf("Initial state:\n");
  global_ctx_log(&info);
  putchar('\n');
  #endif // LOG_CODE_ENABLED
  iterate_states_monothread(&info);
  #ifdef PRINT_RESULT
  printf("\nEnd state:\n");
  global_ctx_log(&info);
  putchar('\n');
  #endif // PRINT_RESULT
  global_ctx_free(&info);
  return EXIT_SUCCESS;
}

#else

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <mpi.h>
#ifdef LOG_CODE_ENABLED
#include <omp.h>
#endif // LOG_CODE_ENABLED
#include <global_context.h>
#include <bs_multithread.h>
#include <macros.h>

int main(int argc, char **argv) {
  MPI_Init(&argc, &argv);
  int mpi_my_rank;
  int mpi_num_procs;
  GlobalContext info = {.m_bodies = NULL};
  uint8_t err_reduction = 0;
  uint8_t err_mem = 0;
  uint64_t temp_u64;
  MPI_Comm_rank(MPI_COMM_WORLD, &mpi_my_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &mpi_num_procs);
  if(argc < 2) {
    if (mpi_my_rank == 0)
      fprintf(stderr, "Process[master] = Missing file path argument: aborting execution in all processes;\n");
    MPI_Finalize();
    return EXIT_FAILURE;
  }
  
  if(mpi_my_rank == 0) {
    #ifdef LOG_CODE_ENABLED
    printf("Process[master] = Starting execution with %d processes and %d threads;\n", mpi_num_procs, omp_get_max_threads());
    #endif // LOG_CODE_ENABLED
    info = global_ctx_parse(argv[1]);
    if(global_ctx_is_null(&info)) {
      temp_u64 = 0;
      MPI_Bcast(&temp_u64, 1, MPI_UINT64_T, 0, MPI_COMM_WORLD);
      fprintf(stderr, "Process[master] = Algorithm input malformed: aborting execution in all processes;\n");
      MPI_Finalize();
      return EXIT_FAILURE;
    }
    MPI_Bcast(&(info.m_num_bodies), 1, MPI_UINT64_T, 0, MPI_COMM_WORLD);
    #ifdef LOG_CODE_ENABLED
    printf("Process[master] = The input file parse occurred with no errors: initiating global context broadcast;\n");
    printf("Process[master] = The initial state got was:\n");
    global_ctx_log(&info);
    putchar('\n');
    #endif // LOG_CODE_ENABLED
    MPI_Bcast(&(info.m_num_steps), 1, MPI_UINT64_T, 0, MPI_COMM_WORLD);
    MPI_Bcast(&(info.m_dt), 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Allreduce(&err_mem, &err_reduction, 1, MPI_UINT8_T, MPI_SUM, MPI_COMM_WORLD);
    if(err_reduction > 0) {
      fprintf(stderr, "Process[master] = Received a memory error from a process: aborting;\n");
      global_ctx_free(&info);
      MPI_Finalize();
      return EXIT_FAILURE;
    }
    MPI_Bcast(info.m_bodies, info.m_num_bodies * 5, MPI_DOUBLE, 0, MPI_COMM_WORLD);
  }
  
  else {
    MPI_Bcast(&(info.m_num_bodies), 1, MPI_UINT64_T, 0, MPI_COMM_WORLD);
    if(! info.m_num_bodies) {
      MPI_Finalize();
      return EXIT_FAILURE;
    }
    MPI_Bcast(&(info.m_num_steps), 1, MPI_UINT64_T, 0, MPI_COMM_WORLD);
    MPI_Bcast(&(info.m_dt), 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    info.m_bodies = malloc(sizeof(double) * info.m_num_bodies * 5);
    if(! info.m_bodies) {
      err_mem = 1;
      #ifdef LOG_CODE_ENABLED
      fprintf(stderr, "Process[%d] = I got a memory error and set my local error to 1;\n", mpi_my_rank);
      #endif // LOG_CODE_ENABLED
    }
    MPI_Allreduce(&err_mem, &err_reduction, 1, MPI_UINT8_T, MPI_SUM, MPI_COMM_WORLD);
    if(err_reduction > 0) {
      free(info.m_bodies);
      MPI_Finalize();
      return EXIT_FAILURE;
    }
    MPI_Bcast(info.m_bodies, info.m_num_bodies * 5, MPI_DOUBLE, 0, MPI_COMM_WORLD);
  }
  #ifdef LOG_CODE_ENABLED
  for (int p = 0; p < mpi_num_procs; p++) {
    if(p != 0 && mpi_my_rank == p) {
      printf("Process[%d] = Received info successfully;\n", mpi_my_rank);
      global_ctx_log(&info);
      putchar('\n');
      fflush(stdout);
    }
    MPI_Barrier(MPI_COMM_WORLD);
  }

  #endif // LOG_CODE_ENABLED
  uint64_t const base = info.m_num_bodies / mpi_num_procs;
  if(base < 1) {
    if (mpi_my_rank == 0)
      fprintf(stderr, "Process[master] = To much processes were emitted for a small input: aborting execution in all processes;\n");
    global_ctx_free(&info);
    MPI_Finalize();
    return EXIT_FAILURE;
  }
  uint64_t const extra = info.m_num_bodies % mpi_num_procs;
  uint64_t const mpi_my_slice_len = base + (mpi_my_rank < extra ? 1 : 0);
  uint64_t const mpi_my_slice_idx = mpi_my_rank * base + (mpi_my_rank < extra ? mpi_my_rank : extra);
  uint64_t recvbuf[mpi_num_procs];
  uint64_t displs[mpi_num_procs];
  displs[0] = 0;
  for(int p = 0; p < mpi_num_procs; p++) {
    recvbuf[p] = (p * base + (p < extra ? p : extra)) * 5;
    if(0 == p)
      continue;
    displs[p] = displs[p-1] + recvbuf[p-1];
  }
  #ifdef LOG_CODE_ENABLED
  MPI_Barrier(MPI_COMM_WORLD);
  for(int p = 0; p < mpi_num_procs; p++) {
    if(p == mpi_my_rank)
      printf("P[%d] = working in [%" PRIu64 ", %" PRIu64 ");\n"
      , mpi_my_rank
      , mpi_my_slice_idx
      , mpi_my_slice_idx + mpi_my_slice_len);
    MPI_Barrier(MPI_COMM_WORLD);
  }
  #endif // LOG_CODE_ENABLED

  iterate_states_parallel(&info, mpi_my_slice_idx, mpi_my_slice_len);

  global_ctx_free(&info);
  MPI_Finalize();
  return EXIT_SUCCESS;
}
#endif // PARALLEL

/*
Fazer sincronização de info do MPI no iterate states, onde tem o barrier no bs_multithread
Fazer uma macro Size que depende do MPI (se tiver parallel ela define para o MPI_Aint, se não, size_t)
*/