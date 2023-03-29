#include <stdio.h>
#include <time.h>
#include <omp.h>
#include <mpi.h>

#include <stdlib.h>

#define XWIDTH 256
#define YWIDTH 256
#define MAXVAL 1//65535

#if ((0x100 & 0xf) == 0x0)
#define I_M_LITTLE_ENDIAN 1
#define swap(mem) (( (mem) & (short int)0xff00) >> 8) +	\
  ( ((mem) & (short int)0x00ff) << 8)
#else
#define I_M_LITTLE_ENDIAN 0
#define swap(mem) (mem)
#endif


void update_playground(int, int, int *);
void print_playground(int, int, int *);
void upgrade_cell(int, int, int, int, int *);

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
            if (drand48() < 0.005)
                playground[i][j] = 1;
            else
                playground[i][j] = 0;
        }
    }

    // Print the playground
    printf("\nInitial playground:");
    print_playground(k, k, p);
    generate_pgm_image2(p, MAXVAL, k, k, "snapshots/snapshot_00000.pgm");

    int steps = 100;
    char filename[100];
    for (int i = 1; i <= steps; i++){
        update_playground(k, k, p);
        ////update_playground_parallel(k, k, p);
        //printf("\nStep %d:", steps);
        //print_playground(k, k, p);
        sprintf(filename, "snapshots/snapshot_%05d.pgm", i);
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


void update_playground(int k_i, int k_j, int *playground) { 
    for (int i = 0; i < k_i; i++){
        for (int j = 0; j < k_j; j++){
            upgrade_cell(i, j, k_i, k_j, playground);
        }
    }
    return;
}

void upgrade_cell(int c_i, int c_j, int k_i, int k_j, int* playground) {
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
            playground[idx] = 0;
        }
    } else {
        if (neighbors == 3 || neighbors == 2) {
            playground[idx] = 1;
        }
    }
}