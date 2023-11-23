#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include <omp.h>

void update_playground_ordered(int, int, int *);
int* update_playground_static(int, int, int *);
void print_playground(int, int, int *);
void upgrade_cell(int, int, int, int, int *, int *);
int upgrade_cell_static(int, int, int, int, int *);

int main(int argc, char** argv) {
    // Initialize the time counter
    clock_t start, end;
    double cpu_time_used;
    start = clock(); // start time

    // Initialize the playground
    const int k = 6;
    int playground[k][k];
    int *p = &playground[0][0];

    // Initialize the playground with random numbers
    srand48(time(NULL)); // set seed 
    for (int i = 0; i < k; i++){
        for (int j = 0; j < k; j++){
            if (drand48() < 0.3) // probability 30% for a cell to be alive
                playground[i][j] = 1;
            else
                playground[i][j] = 0;
        }
    }

    // Print the initial playground
    printf("\nInitial playground:\n");
    print_playground(k, k, p);
    
    // Set the number of steps
    int steps = 1;

    bool static_mode = false;

    if (static_mode){
        // Update and print the playground after each step
        for (int i = 0; i < steps; i++){
            p = update_playground_static(k, k, p);

            // Print the playground
            printf("\nStep %d:\n", i+1); // i+1 because we start at 0
            print_playground(k, k, p);
        }
    } else {
        // Update and print the playground after each step
        for (int i = 0; i < steps; i++){
            update_playground_ordered(k, k, p);

            // Print the playground
            printf("\nStep %d:\n", i+1); // i+1 because we start at 0
            print_playground(k, k, p);
        }
    }
    

    // Print the time used
    end = clock(); // stop time
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("\nTime taken: %f seconds\n", cpu_time_used);

    return 0;
}

// Print the playground as a matrix
void print_playground(int k_i, int k_j, int *playground) {
    // Print each cell
    for (int i = 0; i < k_i; i++){
        for (int j = 0; j < k_j; j++){
            printf("%d ", playground[j*k_j + i]);
        }
        printf("\n");
    }
    return;
}

// Update the playground in the classic way
void update_playground_ordered(int k_i, int k_j, int *playground) {
    int* playground_temp = (int*) malloc(k_i * k_j * sizeof(int));

    // Update each cell
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < k_i; i++){
        for (int j = 0; j < k_j; j++){
            upgrade_cell(i, j, k_i, k_j, playground, playground_temp);
        }
    }

    // Copy the updated playground back to the original
    memcpy(playground, playground_temp, k_i * k_j * sizeof(int));

    free(playground_temp);
    return;
}

// Upgrade a single cell
void upgrade_cell(int c_i, int c_j, int k_i, int k_j, int* playground, int* playground_temp) {
    // Count the number of neighbors
    int neighbors = 0;
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            if (i != 0 || j != 0) {
                int n_i = (c_i + i + k_i) % k_i;
                int n_j = (c_j + j + k_j) % k_j;
                if (playground[n_i * k_j + n_j] == 1) {
                    neighbors++;
                }
            }
        }
    }

    // Update the cell
    playground_temp[c_i*k_j + c_j] = (neighbors == 2 || neighbors == 3) ? 1 : 0;
}

// Update the playground in the static way
int* update_playground_static(const int k_i, const int k_j, int *playground) {
    int* playground_b = (int*) malloc(k_i * k_j * sizeof(int));

    // Update each cell in parallel
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < k_i; i++){
        for (int j = 0; j < k_j; j++){
            playground_b[i*k_j + j] = upgrade_cell_static(i, j, k_i, k_j, playground);
        }
    }

    // Return the pointer to playground B
    return playground_b;
}

// Upgrade a single cell
int upgrade_cell_static(int c_i, int c_j, int k_i, int k_j, int* playground) {
    // Count the number of neighbors
    int neighbors = 0;
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            if (i != 0 || j != 0) {
                int n_i = (c_i + i + k_i) % k_i;
                int n_j = (c_j + j + k_j) % k_j;
                if (playground[n_i * k_j + n_j] == 1) {
                    neighbors++;
                }
            }
        }
    }

    // Return cell status
    return (neighbors == 2 || neighbors == 3) ? 1 : 0;
}
