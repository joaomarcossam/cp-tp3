#include <stdlib.h>
#include <ug_sequencial.h>
#include <global_context.h>
#include <stdio.h>
#include <mpi.h>

/*
MPI_Bcast(
void * buffer:		dados.
int count:		quantidade de elementos.
MPI_Datatype type:	tipo dos elementos.
int root:		rank do processo que envia o broadcast.
MPI_Comm comm:	comunicador.)
 
Todos os processos do comunicador devem chamar o bcast, se não tem deadlock
Os receptores recebem o dado no buffer passado, enquanto o outro envia por esse buffer

*/

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);
    int mpi_my_rank;                // INITIALIZED BY MPI
    int mpi_num_procs;              // INITIALIZED BY MPI
    GlobalContext info;             // INITIALIZED BY EVERY PROC
    MPI_Comm_rank(MPI_COMM_WORLD, &mpi_my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &mpi_num_procs);
    if(argc < 2) {
        if(mpi_my_rank == 0)
            fprintf(stderr, "Process[master] = Missing file path argument: aborting execution in all processes\n");
        MPI_Finalize();
        return EXIT_FAILURE;
    }
    info = global_ctx_parse(argv[1]);
    if(global_ctx_is_null(&info)) {
        if(mpi_my_rank)
            fprintf(stderr, "Process[master] = Algorithm input malformed: aborting execution in all processes\n");
        MPI_Finalize();
        return EXIT_FAILURE;
    }
    if(mpi_my_rank == 0) {
        printf("Process[master] = The input file parse occurred with no errors: initiating parallel processing\n");
        printf("Process[master] = The initial state got was:\n");
        global_ctx_log(&info);
        putchar('\n');
    }
    size_t const base = info.m_num_bodies / mpi_num_procs;
    if(base < 1) {
        if(mpi_my_rank == 0)
            fprintf(stderr, "Process[master] = To much processes were emitted for a small input: aborting execution in all processes\n");
        global_ctx_free(&info);
        MPI_Finalize();
        return EXIT_FAILURE;
    }
    size_t const extra              = info.m_num_bodies % mpi_num_procs;
    size_t const mpi_my_slice_len   = base + (mpi_my_rank < extra ? 1 : 0);
    size_t const mpi_my_slice_idx   = mpi_my_rank * base + (mpi_my_rank < extra ? mpi_my_rank : extra);

    printf("P[%d] = [%zu, %zu)\n", mpi_my_rank, mpi_my_slice_idx, mpi_my_slice_idx + mpi_my_slice_len);
    //----- SINCRONIZAR E COMEÇAR AS OPERAÇÕES AQUI -----

    //----- SINCRONIZAR E COMEÇAR AS OPERAÇÕES AQUI -----
    global_ctx_free(&info);
    MPI_Finalize();
    return EXIT_SUCCESS;
}