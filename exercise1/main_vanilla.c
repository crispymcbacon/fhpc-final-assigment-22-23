#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "pgm.h"
#include "dev.h"

#define RANDOMNESS 0.3 // Probability of a cell being alive at the start of the simulation
#define MAXVAL 255     // Maximum value of a pixel in the PGM image
#define DIRNAME "out.nosync"  // Name of the directory where the output files will be saved

// Function prototypes
void initialize_playground(int k, const char* filename);
void run_playground(const char* filename, int steps, int evolution_mode, int save_step);
void evolve_playground(int k, int *playground, int evolution_mode, int steps, int save_step, const char* filename);
void update_playground_ordered(int k, int *playground);
void update_playground_static(int k, int *playground);
void update_playground_random_start(int k, int *playground);
void update_playground_chessboard(int k, int *playground);
int upgrade_cell(int c_i, int c_j, int k, int *playground);
void shuffle(int *array, int n);


int main(int argc, char **argv) {
    int option;
    bool initialize = false, run = false;
    int evolution_type = 0, steps = 0, save_step = 0;
    int *k = (int *)malloc(sizeof(int));
    char *filename = NULL;

     // Parse command-line arguments
    while ((option = getopt(argc, argv, "irk:e:f:n:s:")) != -1) {
        switch (option) {
            case 'i': // Initialize playground
                initialize = true;
                break;
            case 'r': // Run playground
                run = true;
                break;
            case 'k': // Playground size
                *k = atoi(optarg);
                break;
            case 'e': // Evolution type
                evolution_type = atoi(optarg);
                break;
            case 'f': // Filename
                filename = optarg;
                break;
            case 'n': // Number of steps
                steps = atoi(optarg);
                break;
            case 's': // Save step
                save_step = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Usage: %s [-i] [-r] [-k size] [-e evolution_type] [-f filename] [-n steps] [-s save_step]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }
       // Perform the requested actions based on parsed arguments
    if (initialize && filename != NULL && *k > 0) {
        initialize_playground(*k, filename);
    } else if (run && filename != NULL && steps > 0 && (evolution_type == 0 || evolution_type == 1)) {
        run_playground(filename, steps, evolution_type, save_step);
    } else {
        fprintf(stderr, "Error: Missing or incorrect arguments provided.\n");
        exit(EXIT_FAILURE);
    }

    return 0;
}

// Initialize playground with random binary values and save to file
void initialize_playground(int k, const char* filename) {
    srand48(time(NULL)); // Set seed for random number generation

    int *playground = (int *)malloc(k * k * sizeof(int)); // Allocate memory for playground

    // Check if memory allocation was successful
    if (playground == NULL) {
        fprintf(stderr, "Error: Memory allocation for playground failed.\n");
        exit(EXIT_FAILURE);
    }
    
    // Generate random binary values for the playground
    for (int i = 0; i < k; i++) {
        for (int j = 0; j < k; j++) {
            playground[i * k + j] = drand48() < RANDOMNESS ? 1 : 0;
        }
    }

    // Save the playground to a file
    char filename_buffer[256];
    sprintf(filename_buffer, "%s/%s.pgm", DIRNAME, filename);
    generate_pgm_image(playground, MAXVAL, k, filename_buffer);

    // Free the allocated memory
    free(playground);
}

// Initialize and run playground evolution for a given number of steps
void run_playground(const char* filename, int steps, int evolution_mode, int save_step) {
    // Initialize the time counter
    clock_t start, end;
    double cpu_time_used;
    start = clock();  // start time

    // Read the initial playground state from the inital file
    int *playground = NULL;
    int k;
    char filename_buffer[256];
    sprintf(filename_buffer, "%s/%s.pgm", DIRNAME, filename);
    read_generated_pgm_image(&playground, &k, filename_buffer);

    // Check if memory allocation was successful
    if (playground == NULL) {
        fprintf(stderr, "Error: Memory allocation for playground failed.\n");
        exit(EXIT_FAILURE);
    }
    
    // Evolve the playground
    evolve_playground(k, playground, evolution_mode, steps, save_step, filename);

    // Free the allocated memory
    free(playground);

    // Print the time used
    end = clock();  // stop time
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("\nTime taken: %f seconds\n", cpu_time_used);

    // DEVELOPMENT ONLY
    sprintf(filename_buffer, "vanilla");
    append_to_logs(filename_buffer, evolution_mode, cpu_time_used, k, steps);
}

void evolve_playground(int k, int *playground, int evolution_mode, int steps, int save_step, const char* filename) {
    // Evolve the playground based on the evolution mode
    char filename_buffer[256];
    for (int step = 0; step < steps; step++) {
        switch (evolution_mode) {
            case 0: // Ordered
                update_playground_ordered(k, playground);
                break;
            case 1: // Static
                update_playground_static(k, playground);
                break;
            case 2: // Random Start
                update_playground_random_start(k, playground);
                break;
            case 3: // Chessboard
                update_playground_chessboard(k, playground);
                break;
            default:
                fprintf(stderr, "Error: Invalid evolution mode.\n");
                exit(EXIT_FAILURE);
        }

        // Save the playground state to a file every save_step if save_step is not zero
        if (save_step > 0 && (step + 1) % save_step == 0) {
            sprintf(filename_buffer, "%s/%s_%05d.pgm", DIRNAME, filename, step + 1);
            generate_pgm_image(playground, MAXVAL, k, filename_buffer);
        }
    }

    // If save_step is zero, only save the final state
    if (save_step == 0) {
        sprintf(filename_buffer, "%s/%s_final.pgm", DIRNAME, filename);
        generate_pgm_image(playground, MAXVAL, k, filename_buffer);
    }
}

// Upgrade a single cell using the rules of the Game of Life
int upgrade_cell(int c_i, int c_j, int k, int *playground) {
    int n_i, n_j, neighbors = 0;
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            if (i == 0 && j == 0) continue;  // Skip the cell itself
            n_i = (c_i + i + k) % k;     // Wrap around edges if necessary
            n_j = (c_j + j + k) % k;
            neighbors += playground[n_i * k + n_j];
        }
    }

    int current_state = playground[c_i * k + c_j];
    if ((current_state == 1 && (neighbors == 2 || neighbors == 3)) ||
        (current_state == 0 && neighbors == 3)) {
        return 1;  // Cell stays alive or is born
    } else {
        return 0;  // Cell dies or remains dead
    }
}

// Update playground in an ordered manner with dynamically allocated memory
void update_playground_ordered(int k, int *playground) {
    int *temp_playground = (int *)malloc(k * k * sizeof(int));
    if (temp_playground == NULL) {
        perror("Failed to allocate memory for temp_playground");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < k; i++) {
        for (int j = 0; j < k; j++) {
            temp_playground[i * k + j] = upgrade_cell(i, j, k, playground);
        }
    }

    memcpy(playground, temp_playground, k * k * sizeof(int));
    free(temp_playground);  // Free the dynamically allocated memory
}

// Static evolution
void update_playground_static(int k, int *playground) {
    int new_states[k * k];  // First pass - calculate new states

    for (int i = 0; i < k; i++) {
        for (int j = 0; j < k; j++) {
            new_states[i * k + j] = upgrade_cell(i, j, k, playground);
        }
    }

    memcpy(playground, new_states, k * k * sizeof(int));  // Second pass - update all cells at once
}

// Helper function to shuffle an array
void shuffle(int *array, int n) {
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = array[i];
        array[i] = array[j];
        array[j] = temp;
    }
}

/*
Optimizing `update_playground_random_start` for parallelization requires
rethinking the sequential nature of the square wave propagation model.
This approach involve updating the playground using a shuffled list of cell coordinates.
Each thread will be assigned a subset of the entire playground to
operate on, reducing the introduction of race conditions.
*/
// Random Start evolution
void update_playground_random_start(int k, int *playground) {
    int *temp_playground = (int *)malloc(k * k * sizeof(int));
    int *indices = (int *)malloc(k * k * sizeof(int));  // Array to store cell indices
    if (temp_playground == NULL || indices == NULL) {
        perror("Failed to allocate memory");
        exit(EXIT_FAILURE);
    }

    // Populate array with indices
    for (int i = 0; i < k * k; ++i) {
        indices[i] = i;
    }

    // Shuffle indices to randomize update order
    shuffle(indices, k * k);

    // Update cells in random order
    for (int idx = 0; idx < k * k; ++idx) {
        int i = indices[idx] / k;  // Get row index
        int j = indices[idx] % k;  // Get column index
        temp_playground[indices[idx]] = upgrade_cell(i, j, k, playground);
    }

    // Copy the new cell states to the original playground
    memcpy(playground, temp_playground, k * k * sizeof(int));

    // Clean up
    free(temp_playground);
    free(indices);
}

// Helper function that updates a color subset of cells ('color' is 0 for black, 1 for white)
void update_chessboard_cells(int k, int *playground, int *temp_playground, int color) {
    for (int i = 0; i < k; i++) {
        for (int j = (i % 2) ^ color; j < k; j += 2) {  // use XOR to switch between 0 and 1
            temp_playground[i * k + j] = upgrade_cell(i, j, k, playground);
        }
    }
}

// Chessboard evolution refactored to reduce repetition
void update_playground_chessboard(int k, int *playground) {
    int *temp_playground = (int *)malloc(k * k * sizeof(int));
    if (!temp_playground) {
        perror("Failed to allocate memory for temp_playground");
        exit(EXIT_FAILURE);
    }

    // Update white cells
    update_chessboard_cells(k, playground, temp_playground, 1);

    // Update black cells
    update_chessboard_cells(k, playground, temp_playground, 0);

    memcpy(playground, temp_playground, k * k * sizeof(int));
    free(temp_playground);
}
