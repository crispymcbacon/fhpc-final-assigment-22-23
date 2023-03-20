// Source code from https://mpitutorial.com/tutorials/mpi-hello-world/

//#include <mpi.h>
#include <stdio.h>
#include <time.h>
#include <omp.h>

void update_playground(int, int, int *);
void print_playground(int, int, int *);
void upgrade_cell(int, int, int, int, int *);

int main(int argc, char** argv) {

    // Iniitialize the random number generator
    srand48(time(NULL));

    // Initialize the number of threads
    const int nThreads = 2;
    omp_set_num_threads(nThreads);

    // Initialize the playground
    const int k = 5;
    int playground[k][k];
    int *p = &playground[0][0];

    // Initialize the playground with random numbers
    #pragma omp parallel for
    for (int i = 0; i < k; i++){
        for (int j = 0; j < k; j++){
            if (drand48() < 0.5)
                playground[i][j] = 0;
            else
                playground[i][j] = 1;
        }
    }

    // Print the playground
    printf("\nInitial playground:");
    print_playground(k, k, p);

    int steps = 1;
    while (steps > 0){
        update_playground(k, k, p);
        steps--;
    }

    // Print the playground
    printf("\nStep 1:");
    print_playground(k, k, p);
    

    /*
    // Initialize the MPI environment
    MPI_Init(NULL, NULL);

    // Get the number of processes
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Get the rank of the process
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    // Get the name of the processor
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_Get_processor_name(processor_name, &name_len);

    // Print off a hello world message
    printf("Hello world from processor %s, rank %d out of %d processors\n",
           processor_name, world_rank, world_size);

    // Finalize the MPI environment.
    MPI_Finalize();
    */


}

void print_playground(int k_i, int k_j, int *playground) {
    printf("\n");
    for (int i = 0; i < k_i; i++){
        for (int j = 0; j < k_j; j++){
            printf("%d ", playground[i*j + j]);
        }
        printf("\n");
    }
    return;
}


void update_playground(int k_i, int k_j, int *playground) { 
    for (int i = 0; i < k_i; i++){
        for (int j = 0; j < k_j; j++){
            upgrade_cell(i, j, k_i, k_j, playground);
        }
    }
    return;
}

void upgrade_cell(int c_i, int c_j, int k_i, int k_j, int *playground) {
    int neighbours = 0;
    int idx_i = 0;
    int idx_j = 0;
    for (int i = c_i - 1; i <= c_i + 1; i++){
        idx_i = i;
        if (i < 0){
            idx_i = k_i - i;
        } else if (i >= k_i)
        {
            idx_i = i - k_i;
        }
            
        for (int j = k_j - 1; j <= k_j + 1; j++){
            idx_j = j;
            if (j < 0){
                idx_j = k_j - j;
            } else if (j >= k_j)
            {
                idx_j = i - k_j;
            }
            
            if (idx_i == c_i && idx_j == c_j)
                continue;
            if (idx_i < 0 || idx_i >= c_i || idx_j < 0 || idx_j >= c_j)
                continue;
            if (playground[idx_i*idx_j + idx_j] == 1)
                neighbours++;
        }
    }
    if (playground[k_i*k_j + k_j] == 1){
        if (neighbours < 2 || neighbours > 3)
            playground[k_i*k_j + k_j] = 0;
    }
    else{
        if (neighbours == 3)
            playground[k_i*k_j + k_j] = 1;
    }
    return;
}