#include <stdlib.h>
#include <ug_sequencial.h>
#include <global_context.h>
#include <stdio.h>

int main(int argc, char **argv) {
    if(argc < 2) {
        fprintf(stderr, "Missing file path argument: aborting execution\n");
        return EXIT_FAILURE;
    }
    GlobalContext info = global_ctx_parse(argv[1]);
    if(global_ctx_is_null(&info)) {
        fprintf(stderr, "Algorithm input malformed: aborting execution\n");
        return EXIT_FAILURE;
    }
    printf("Initial state:\n");
    global_ctx_log(&info);
    iterate_states(&info);
    printf("\nEnd state:\n");
    global_ctx_log(&info);
    printf("\n");
    global_ctx_free(&info);
}