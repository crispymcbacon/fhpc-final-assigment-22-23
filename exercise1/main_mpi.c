#include <stdio.h>
#include <time.h>
#include <omp.h>
#include <mpi.h>

#include <stdlib.h>


void print_playground(int, int, int *);

void update_playground2(int, int, int *);
int upgrade_cell2(int, int, int, int, int *);

int main(int argc, char** argv) {

    // check time
    clock_t start, end;
    double cpu_time_used;
    start = clock();


    // Iniitialize the random number generator
    srand48(time(NULL));

    // Initialize the number of threads
    const int nThreads = 2;
    //omp_set_num_threads(nThreads);

    // Initialize the playground
    const int k = 100;
    int playground[k][k];
    int *p = &playground[0][0];

    // Initialize the playground with random numbers
   #pragma omp parallel for
    for (int i = 0; i < k; i++){
        for (int j = 0; j < k; j++){
            //random
            if (drand48() < 0.2)
                playground[i][j] = 1;
            else
                playground[i][j] = 0;
        }
    }

    // Print the playground
    printf("\nInitial playground:");
    // generate_pgm_image2(p, MAXVAL, k, k, "snapshots.nosync/snapshot_00000.pgm");

    int steps = 1000;
    char filename[100];
    for (int i = 1; i <= steps; i++){
        update_playground2(k, k, p);
        sprintf(filename, "snapshots.nosync/snapshot_%05d.pgm", i);
        #pragma omp barrier
        // generate_pgm_image2(p, MAXVAL, k, k, filename);
    }

    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC; //calculate time used

    printf("\nTime taken: %f seconds\n", cpu_time_used);


}

void print_playground(int k_i, int k_j, int *playground) {
    printf("\n");
    for (int i = 0; i < k_i; i++){
        for (int j = 0; j < k_j; j++){
            printf("%d ", playground[i*k_j + j]);
        }
        printf("\n");
    }
    return;
}

void update_playground2(int k_i, int k_j, int *playground) {
    int tmp_playground[k_i][k_j];
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < k_i; i++){
        for (int j = 0; j < k_j; j++){
            tmp_playground[i][j] = upgrade_cell2(i, j, k_i, k_j, playground);
        }
    }
    
    // Copy tmp_playground back to the original playground
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < k_i; i++){
        for (int j = 0; j < k_j; j++){
            playground[i*k_j + j] = tmp_playground[i][j];
        }
    }
}

int upgrade_cell2(int c_i, int c_j, int k_i, int k_j, int* playground) {
    int neighbors = 0;
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            if (i == 0 && j == 0) continue;
            int n_i = (c_i + i + k_i) % k_i;
            int n_j = (c_j + j + k_j) % k_j;
            if (playground[n_i*k_j + n_j] == 1) neighbors++;
        }
    }
    int idx = c_i*k_j + c_j;
    if (playground[idx] == 1) {
        if (neighbors < 2 || neighbors > 3) {
            return 0;
        } else {
            return 1;
        }
    } else {
        //if (neighbors == 3 || neighbors == 2) {
        if (neighbors == 3) {
            return 1;
        } else {
            return 0;
        }
    }
}


