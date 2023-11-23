#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include <mpi.h>

void print_playground(int, int, int *);
void upgrade_cell(int, int, int, int, int *, int *);

int main(int argc, char** argv) {

    printf("DC");
    // Initialize MPI
    MPI_Init(&argc, &argv);

    

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Initialize the playground
    const int k = 6;
    int playground[k][k];
    int *p = &playground[0][0];

    // Initialize the playground with random numbers
    srand48(time(NULL) + rank); // set seed 
    for (int i = 0; i < k; i++){
        for (int j = 0; j < k; j++){
            if (drand48() < 0.3) // probability 30% for a cell to be alive
                playground[i][j] = 1;
            else
                playground[i][j] = 0;
        }
    }

    // Print the initial playground
    if (rank == 0) {
        printf("\nInitial playground:\n");
        print_playground(k, k, p);
    }
    
    // Set the number of steps
    int steps = 1;

    // Update and print the playground after each step
    for (int i = 0; i < steps; i++){
        int* playground_temp = (int*) malloc(k * k * sizeof(int));

        // Update each cell
        for (int i = rank; i < k; i+=size){
            for (int j = 0; j < k; j++){
                upgrade_cell(i, j, k, k, p, playground_temp);
            }
        }
        // Gather the updated playgrounds from all processes
        MPI_Allgather(playground_temp, k*k/size, MPI_INT, p, k*k/size, MPI_INT, MPI_COMM_WORLD);

        // Print the playground
        if (rank == 0) {
            printf("\nStep %d:\n", i+1); // i+1 because we start at 0
            print_playground(k, k, p);
        }

        free(playground_temp);
    }

    // Finalize MPI
    MPI_Finalize();

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