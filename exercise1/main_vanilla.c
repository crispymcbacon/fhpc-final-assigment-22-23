/*
- **Ordered Evolution**: This method updates the cells in a row-major order, which can cause
dependencies and introduce artifacts due to the order of updates.

- **Static Evolution**: This method evaluates the statuses first without actually updating
the cells. In the second pass, it updates all cells at once based on the evaluated statuses.

- **Random Start Evolution**: This method chooses a random starting point and updates cells
in a propagating square wave pattern. This simulates a non-deterministic update order.

- **Chessboard (Checkerboard) Evolution**: This method updates cells in a two-pass approach:
first the "white" cells and then the "black" cells, analogous to a chessboard pattern.
This helps to maintain some of the locality during updates but still introduces a
structured update order that is less biased than row-major.
*/

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
void update_playground_chessboard(int k_i, int k_j, int *playground);

int main(int argc, char **argv) {
    // Initialize the time counter
    clock_t start, end;
    double cpu_time_used;
    start = clock();  // start time

    // Initialize the playground
    const int k = 100;
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

    // Print the initial playground
    printf("\nInitial playground:\n");
    print_playground(k, k, p);

    // Set the number of steps
    int steps = 1e5;
    int mode = 3;

    // Update and print the playground after each step
    for (int i = 0; i < steps; i++) {
        // Print the playground
        // printf("\nStep %d:\n", i+1); // i+1 because we start at 0

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
        // print_playground(k, k, p);
    }

    // Print the time used
    end = clock();  // stop time
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("\nTime taken: %f seconds\n", cpu_time_used);

    return 0;
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

// Ordered evolution
void update_playground_ordered(int k_i, int k_j, int *playground) {
    int temp_playground[k_i * k_j];  // Create a temporary playground to store new cell states

    for (int i = 0; i < k_i; i++) {
        for (int j = 0; j < k_j; j++) {
            temp_playground[i * k_j + j] = upgrade_cell(i, j, k_i, k_j, playground);  // Copy the new cell states to the original playground
        }
    }

    memcpy(playground, temp_playground, k_i * k_j * sizeof(int));
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

// Random Start evolution with square wave propagation
void update_playground_random_start(int k_i, int k_j, int *playground) {
    int temp_playground[k_i * k_j];
    memset(temp_playground, 0, sizeof(int) * k_i * k_j);  // Initialize with zeros

    // Choose a random starting point
    int start_i = rand() % k_i;
    int start_j = rand() % k_j;

    // Calculate the maximum distance from the starting point to any corner
    int max_dist = 0;
    for (int i = 0; i < k_i; i++) {
        for (int j = 0; j < k_j; j++) {
            int dist = abs(i - start_i) + abs(j - start_j);
            if (dist > max_dist) {
                max_dist = dist;
            }
        }
    }

    // Propagate the evolution in a square wave pattern
    for (int dist = 0; dist <= max_dist; dist++) {
        for (int i = 0; i < k_i; i++) {
            for (int j = 0; j < k_j; j++) {
                // Check if (i, j) is in the current "wavefront"
                if (abs(i - start_i) + abs(j - start_j) == dist) {
                    // Update the cell at (i, j)
                    temp_playground[i * k_j + j] = upgrade_cell(i, j, k_i, k_j, playground);
                }
            }
        }
    }

    memcpy(playground, temp_playground, k_i * k_j * sizeof(int));
}

// Chessboard evolution
void update_playground_chessboard(int k_i, int k_j, int *playground) {
    int temp_playground[k_i * k_j];

    // Update "white" cells
    for (int i = 0; i < k_i; i++) {
        for (int j = (i % 2); j < k_j; j += 2) {
            temp_playground[i * k_j + j] = upgrade_cell(i, j, k_i, k_j, playground);
        }
    }

    // Update "black" cells
    for (int i = 0; i < k_i; i++) {
        for (int j = ((i % 2) ? 0 : 1); j < k_j; j += 2) {
            temp_playground[i * k_j + j] = upgrade_cell(i, j, k_i, k_j, playground);
        }
    }

    memcpy(playground, temp_playground, k_i * k_j * sizeof(int));
}
