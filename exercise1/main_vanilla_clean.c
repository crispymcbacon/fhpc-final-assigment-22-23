#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void print_playground(int k_i, int k_j, int *playground);
int upgrade_cell(int c_i, int c_j, int k_i, int k_j, int *playground);
void update_playground_ordered(int k_i, int k_j, int *playground);
void update_playground_static(int k_i, int k_j, int *playground);
void update_playground_random_start(int k_i, int k_j, int *playground);
void update_chessboard_cells(int k_i, int k_j, int *playground, int *temp_playground, int color);
void update_playground_chessboard(int k_i, int k_j, int *playground);

int main(int argc, char **argv) {
    // Initialize the time counter
    clock_t start, end;
    double cpu_time_used;
    start = clock();  // start time

    // Initialize the playground
    const int k = 5;
    const int steps = 100;
    const int mode = 3;
    int playground[k][k];
    int *p = &playground[0][0];
    double cell_alive_prob = 0.3;  // probability for a cell to be alive

    // Initialize the playground with random numbers
    srand48(time(NULL));  // set seed
    for (int i = 0; i < k; i++) {
        for (int j = 0; j < k; j++) {
            if (drand48() < cell_alive_prob)
                playground[i][j] = 1;
            else
                playground[i][j] = 0;
        }
    }

    // // Print the initial playground
    printf("\nInitial playground:\n");
    print_playground(k, k, p);
    
    // Update and print the playground after each step
    for (int i = 0; i < steps; i++) {
        // Print the playground
        printf("\nStep %d:\n", i+1); // i+1 because we start at 0

        switch (mode) {
            case 0:
                update_playground_ordered(k, k, p);
                break;
            case 1:
                update_playground_static(k, k, p);
                break;
            case 2:
                update_playground_random_start(k, k, p);
                break;
            case 3:
                update_playground_chessboard(k, k, p);
                break;
            default:
                printf("\nInvalid mode", mode);
                break;
        }
        print_playground(k, k, p);
    }

    // Print the time used
    end = clock();  // stop time
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("\nTime taken: %f seconds\n", cpu_time_used);
}

// Print the playground as a matrix
void print_playground(int k_i, int k_j, int *playground) {
    for (int i = 0; i < k_i; i++) {
        for (int j = 0; j < k_j; j++) {
            printf("%d ", playground[j * k_j + i]);
        }
        printf("\n");
    }
    return;
}

// Upgrade a single cell using the rules of the Game of Life
int upgrade_cell(int c_i, int c_j, int k_i, int k_j, int *playground) {
    int n_i, n_j, neighbors = 0;
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            if (i == 0 && j == 0) continue;  // Skip the cell itself
            n_i = (c_i + i + k_i) % k_i;     // Wrap around edges if necessary
            n_j = (c_j + j + k_j) % k_j;
            neighbors += playground[n_i * k_j + n_j];
        }
    }

    int current_state = playground[c_i * k_j + c_j];
    if ((current_state == 1 && (neighbors == 2 || neighbors == 3)) ||
        (current_state == 0 && neighbors == 3)) {
        return 1;  // Cell stays alive or is born
    } else {
        return 0;  // Cell dies or remains dead
    }
}

// Update playground in an ordered manner with dynamically allocated memory
void update_playground_ordered(int k_i, int k_j, int *playground) {
    int *temp_playground = (int *)malloc(k_i * k_j * sizeof(int));
    if (temp_playground == NULL) {
        perror("Failed to allocate memory for temp_playground");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < k_i; i++) {
        for (int j = 0; j < k_j; j++) {
            temp_playground[i * k_j + j] = upgrade_cell(i, j, k_i, k_j, playground);
        }
    }

    memcpy(playground, temp_playground, k_i * k_j * sizeof(int));
    free(temp_playground);  // Free the dynamically allocated memory
}

// Static evolution
void update_playground_static(int k_i, int k_j, int *playground) {
    int new_states[k_i * k_j];  // First pass - calculate new states

    for (int i = 0; i < k_i; i++) {
        for (int j = 0; j < k_j; j++) {
            new_states[i * k_j + j] = upgrade_cell(i, j, k_i, k_j, playground);
        }
    }

    memcpy(playground, new_states, k_i * k_j * sizeof(int));  // Second pass - update all cells at once
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

// Random Start evolution
void update_playground_random_start(int k_i, int k_j, int *playground) {
    int *temp_playground = (int *)malloc(k_i * k_j * sizeof(int));
    int *indices = (int *)malloc(k_i * k_j * sizeof(int));  // Array to store cell indices
    if (temp_playground == NULL || indices == NULL) {
        perror("Failed to allocate memory");
        exit(EXIT_FAILURE);
    }

    // Populate array with indices
    for (int i = 0; i < k_i * k_j; ++i) {
        indices[i] = i;
    }

    // Shuffle indices to randomize update order
    shuffle(indices, k_i * k_j);

    // Update cells in random order
    for (int idx = 0; idx < k_i * k_j; ++idx) {
        int i = indices[idx] / k_j;  // Get row index
        int j = indices[idx] % k_j;  // Get column index
        temp_playground[indices[idx]] = upgrade_cell(i, j, k_i, k_j, playground);
    }

    // Copy the new cell states to the original playground
    memcpy(playground, temp_playground, k_i * k_j * sizeof(int));

    // Clean up
    free(temp_playground);
    free(indices);
}

// Helper function that updates a color subset of cells ('color' is 0 for black, 1 for white)
void update_chessboard_cells(int k_i, int k_j, int *playground, int *temp_playground, int color) {
    for (int i = 0; i < k_i; i++) {
        for (int j = (i % 2) ^ color; j < k_j; j += 2) {  // use XOR to switch between 0 and 1
            temp_playground[i * k_j + j] = upgrade_cell(i, j, k_i, k_j, playground);
        }
    }
}

// Chessboard evolution refactored to reduce repetition
void update_playground_chessboard(int k_i, int k_j, int *playground) {
    int *temp_playground = (int *)malloc(k_i * k_j * sizeof(int));
    if (!temp_playground) {
        perror("Failed to allocate memory for temp_playground");
        exit(EXIT_FAILURE);
    }

    // Update white cells
    update_chessboard_cells(k_i, k_j, playground, temp_playground, 1);

    // Update black cells
    update_chessboard_cells(k_i, k_j, playground, temp_playground, 0);

    memcpy(playground, temp_playground, k_i * k_j * sizeof(int));
    free(temp_playground);
}
