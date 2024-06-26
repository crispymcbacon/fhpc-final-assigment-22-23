#include <mpi.h>
#include <omp.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>

#include "dev.h"
#include "pgm.h"
#include "evolution.h"

#define RANDOMNESS 0.5
#define MAXVAL 255
#define DIRNAME "out.nosync"
struct timeval start_time, end_time;

void initialize_playground(int k, const char *filename, int rank);
void run_playground(const char *filename, int steps, int evolution_mode, int save_step, int rank, int size, const char *info_string, const char *log_filename);
void evolve_playground(int k, unsigned char *playground, int evolution_mode, int steps, int save_step, const char *filename, int rank, int size);

int main(int argc, char **argv) {
    int option;
    bool initialize = false, run = false;
    int evolution_type = 0, steps = 0, save_step = 0;
    int k = 0;
    char *filename = NULL;
    char *info_string = NULL;
    char *log_filename = NULL;

    MPI_Init(NULL, NULL);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    while ((option = getopt(argc, argv, "irk:e:f:n:s:t:l:")) != -1) {
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
            case 't':  // Save a string for debugging purposes
                info_string = optarg;
                break;
            case 'l':  // Debugging option
                log_filename = optarg;
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
        run_playground(filename, steps, evolution_type, save_step, rank, size, info_string, log_filename);
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
    unsigned char *playground = NULL;
    if (rank == 0) {
        srand48(time(NULL));
        playground = (unsigned char *)calloc(k * k, sizeof(unsigned char));

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

    MPI_Bcast(playground, k * k, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        char filename_buffer[256];
        sprintf(filename_buffer, "%s/%s.pgm", DIRNAME, filename);
        generate_pgm_image(playground, MAXVAL, k, filename_buffer);
        free(playground);
    }
}

void run_playground(const char *filename, int steps, int evolution_mode, int save_step, int rank, int size, const char *info_string, const char *log_filename) {
    double time_elapsed;

    if (rank == 0){
        gettimeofday(&start_time, NULL);
    }
    
    unsigned char *playground = NULL;
    int k;
    char filename_buffer[256];
    sprintf(filename_buffer, "%s/%s.pgm", DIRNAME, filename);
    read_generated_pgm_image(&playground, &k, filename_buffer);

    if (playground == NULL) {
        fprintf(stderr, "Error: Memory allocation for playground failed.\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    evolve_playground(k, playground, evolution_mode, steps, save_step, filename, rank, size);

    if (playground != NULL) {
        free(playground);
    }

    if (rank == 0) {
        gettimeofday(&end_time, NULL);
        time_elapsed = (end_time.tv_sec - start_time.tv_sec) + (end_time.tv_usec - start_time.tv_usec) / 1e6;
        printf("Time taken: %f seconds\n", time_elapsed);
        sprintf(filename_buffer, "mpi_openmp");
        append_to_logs(log_filename, filename, filename_buffer, evolution_mode, time_elapsed, k, steps, info_string);
    }
}

void evolve_playground(int k, unsigned char *playground, int evolution_mode, int steps, int save_step, const char *filename, int rank, int size) {
    char filename_buffer[256];
    unsigned char *temp_playground = NULL;
    unsigned char *top_ghost_row = NULL;
    unsigned char *bottom_ghost_row = NULL;
    unsigned char *gathered_playground = NULL;

    if (evolution_mode == 0){
        // Allocate memory for ordered evolution
        temp_playground = (unsigned char *)calloc(k * k, sizeof(unsigned char));
        top_ghost_row = (unsigned char *)calloc(k * k, sizeof(unsigned char));
        bottom_ghost_row = (unsigned char *)calloc(k * k, sizeof(unsigned char));
    } else if (evolution_mode == 1) {
        // Allocate memory for static evolution
        temp_playground = (unsigned char *)calloc(k * k, sizeof(unsigned char));
    }
    
    for (int step = 0; step < steps; step++) {
        switch (evolution_mode) {
            case 0:
                update_playground_ordered(k, playground, rank, size, temp_playground, top_ghost_row, bottom_ghost_row);
                break;
            case 1:
                update_playground_static(k, playground, rank, size, temp_playground);
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

        
        // Gather playground from all processes
        if (rank == 0) {
            gathered_playground = (unsigned char *)calloc(k * k, sizeof(unsigned char));
        }

        // Calculate the number of rows that each process is updating
        int rows_per_proc = k / size;
        int remainder = k % size;
        int start_row = rank * rows_per_proc + (rank < remainder ? rank : remainder);
        int end_row = start_row + rows_per_proc + (rank < remainder ? 1 : 0);

        // Calculate the number of cells that each process is updating
        int num_cells = (end_row - start_row) * k;

        // Gather the updated portions of the playground from each process
        MPI_Gather(playground + start_row * k, num_cells, MPI_UNSIGNED_CHAR, gathered_playground, num_cells, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

        // Print playground
        if (save_step > 0 && (step + 1) % save_step == 0) {
            if (rank == 0) {
                sprintf(filename_buffer, "%s/%s_%05d.pgm", DIRNAME, filename, step + 1);
                generate_pgm_image(gathered_playground, MAXVAL, k, filename_buffer);  // Use gathered_playground here
            }
        }
    }

    if (save_step == 0 && rank == 0) {
        sprintf(filename_buffer, "%s/%s_final.pgm", DIRNAME, filename);
        generate_pgm_image(gathered_playground, MAXVAL, k, filename_buffer);  // Use gathered_playground here
    }

    // Free memory for ordered evolution
    if (temp_playground != NULL) {
        free(temp_playground);
    }
    if (top_ghost_row != NULL) {
        free(top_ghost_row);
    }
    if (bottom_ghost_row != NULL) {
        free(bottom_ghost_row);
    }
    if (rank == 0 && gathered_playground != NULL) {
        free(gathered_playground);
    }
}