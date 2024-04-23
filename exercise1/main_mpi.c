#include <mpi.h>
#include <omp.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "dev.h"
#include "pgm.h"
#include "updates.h"

#define RANDOMNESS 0.3
#define MAXVAL 255
#define DIRNAME "out.nosync"

void initialize_playground(int k, const char *filename, int rank);
void run_playground(const char *filename, int steps, int evolution_mode, int save_step, int rank, int size);
void evolve_playground(int k, int *playground, int evolution_mode, int steps, int save_step, const char *filename, int rank, int size);
// void update_playground_ordered(int k, int *playground, int rank, int size);
// void update_playground_static(int k, int *playground, int rank, int size);
// void update_playground_random_start(int k, int *playground, int rank, int size);
// void update_playground_chessboard(int k, int *playground, int rank, int size);
// int upgrade_cell(int c_i, int c_j, int k, int *playground);

int main(int argc, char **argv) {
    int option;
    bool initialize = false, run = false;
    int evolution_type = 0, steps = 0, save_step = 0;
    int k = 0;
    char *filename = NULL;

    MPI_Init(NULL, NULL);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    while ((option = getopt(argc, argv, "irk:e:f:n:s:")) != -1) {
        switch (option) {
            case 'i':  // Initialize playground
                initialize = true;
                break;
            case 'r':  // Run playground
                run = true;
                break;
            case 'k':  // Playground size
                k = atoi(optarg);
                break;
            case 'e':  // Evolution type
                evolution_type = atoi(optarg);
                break;
            case 'f':  // Filename
                filename = optarg;
                break;
            case 'n':  // Number of steps
                steps = atoi(optarg);
                break;
            case 's':  // Save step
                save_step = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Usage: %s [-i] [-r] [-k size] [-e evolution_type] [-f filename] [-n steps] [-s save_step]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    // Perform the requested actions based on parsed arguments
    printf("init: %i, k: %d, filename: %s, steps: %d, evolution_type: %d, save_step: %d\n", initialize, k, filename, steps, evolution_type, save_step);

    if (initialize && filename != NULL && k > 0) {
        initialize_playground(k, filename, rank);
    } else if (run && filename != NULL && steps > 0 && (evolution_type >= 0 && evolution_type <= 3)) {
        run_playground(filename, steps, evolution_type, save_step, rank, size);
    } else {
        if (rank == 0) {
            fprintf(stderr, "Error: Missing or incorrect arguments provided.\n");
        }
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    MPI_Finalize();
    return 0;
}

void initialize_playground(int k, const char *filename, int rank) {
    int *playground = NULL;
    if (rank == 0) {
        srand48(time(NULL));
        playground = (int *)calloc(k * k, sizeof(int));

        if (playground == NULL) {
            fprintf(stderr, "Error: Memory allocation for playground failed.\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

#pragma omp parallel for collapse(2)
        for (int i = 0; i < k; i++) {
            for (int j = 0; j < k; j++) {
                playground[i * k + j] = drand48() < RANDOMNESS ? 1 : 0;
            }
        }
    }

    MPI_Bcast(playground, k * k, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        char filename_buffer[256];
        sprintf(filename_buffer, "%s/%s.pgm", DIRNAME, filename);
        generate_pgm_image(playground, MAXVAL, k, filename_buffer);
        free(playground);
    }
}

void run_playground(const char *filename, int steps, int evolution_mode, int save_step, int rank, int size) {
    clock_t start, end;
    double cpu_time_used;
    start = clock();

    int *playground = NULL;
    int k;
    char filename_buffer[256];
    sprintf(filename_buffer, "%s/%s.pgm", DIRNAME, filename);
    read_generated_pgm_image(&playground, &k, filename_buffer);

    if (playground == NULL) {
        fprintf(stderr, "Error: Memory allocation for playground failed.\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    printf("Rank %d: k = %d\n", rank, k);

    evolve_playground(k, playground, evolution_mode, steps, save_step, filename, rank, size);

    free(playground);

    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    if (rank == 0) {
        printf("Time taken: %f seconds\n", cpu_time_used);
        sprintf(filename_buffer, "mpi_openmp");
        append_to_logs(filename, filename_buffer, evolution_mode, cpu_time_used, k, steps);
    }
}

void evolve_playground(int k, int *playground, int evolution_mode, int steps, int save_step, const char *filename, int rank, int size) {
    char filename_buffer[256];
    for (int step = 0; step < steps; step++) {
        switch (evolution_mode) {
            case 0:
                update_playground_ordered(k, playground, rank, size);
                break;
            case 1:
                update_playground_static(k, playground, rank, size);
                break;
            // case 2:
            //     update_playground_random_start(k, playground, rank, size);
            //     break;
            // case 3:
            //     update_playground_chessboard(k, playground, rank, size);
            //     break;
            default:
                if (rank == 0) {
                    fprintf(stderr, "Error: Invalid evolution mode.\n");
                }
                MPI_Abort(MPI_COMM_WORLD, 1);
        }

        if (save_step > 0 && (step + 1) % save_step == 0) {
            if (rank == 0) {
                sprintf(filename_buffer, "%s/%s_%05d.pgm", DIRNAME, filename, step + 1);
                generate_pgm_image(playground, MAXVAL, k, filename_buffer);
            }
        }
    }

    if (save_step == 0 && rank == 0) {
        sprintf(filename_buffer, "%s/%s_final.pgm", DIRNAME, filename);
        generate_pgm_image(playground, MAXVAL, k, filename_buffer);
    }
}