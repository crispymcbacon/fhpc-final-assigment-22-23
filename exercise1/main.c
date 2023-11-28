#include <mpi.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define XWIDTH 256
#define YWIDTH 256
#define MAXVAL 1  // 65535

// #if ((0x100 & 0xf) == 0x0)
// #define I_M_LITTLE_ENDIAN 1
// #define swap(mem) (( (mem) & (short int)0xff00) >> 8) +	\
//   ( ((mem) & (short int)0x00ff) << 8)
// #else
// #define I_M_LITTLE_ENDIAN 0
// #define swap(mem) (mem)
// #endif

void update_playground(int, int, int*);
void print_playground(int, int, int*);
void upgrade_cell(int, int, int, int, int*);
void update_playground_parallel(int, int, int*);

// void export_pgm_image(int *, int, int, int, const char *);
void write_pgm_image(void*, int, int, int, const char*);
void read_pgm_image(void**, int*, int*, int*, const char*);

void generate_pgm_image(int*, int, int, int, const char*);
void generate_pgm_image2(int*, int, int, int, const char*);

void update_playground2(int, int, int*);
int upgrade_cell2(int, int, int, int, int*);

int main(int argc, char** argv) {
    // check time
    clock_t start, end;
    double cpu_time_used;
    start = clock();

    // Iniitialize the random number generator
    srand48(time(NULL));

    // Initialize the number of threads
    const int nThreads = 2;
    // omp_set_num_threads(nThreads);

    // Initialize the playground
    const int k = 100;
    int playground[k][k];
    int* p = &playground[0][0];

    // Initialize the playground with random numbers
#pragma omp parallel for
    for (int i = 0; i < k; i++) {
        for (int j = 0; j < k; j++) {
            // random
            if (drand48() < 0.2)
                playground[i][j] = 1;
            else
                playground[i][j] = 0;
        }
    }

    // Print the playground
    printf("\nInitial playground:");
    generate_pgm_image2(p, MAXVAL, k, k, "snapshots.nosync/snapshot_00000.pgm");

    int steps = 1000;
    char filename[100];
    for (int i = 1; i <= steps; i++) {
        update_playground2(k, k, p);
        ////update_playground_parallel(k, k, p);
        // printf("\nStep %d:", steps);
        // print_playground(k, k, p);
        sprintf(filename, "snapshots.nosync/snapshot_%05d.pgm", i);
#pragma omp barrier
        generate_pgm_image2(p, MAXVAL, k, k, filename);
    }

    // Print the playground

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

    end = clock();
    cpu_time_used =
        ((double)(end - start)) / CLOCKS_PER_SEC;  // calculate time used

    printf("\nTime taken: %f seconds\n", cpu_time_used);
}

void print_playground(int k_i, int k_j, int* playground) {
    printf("\n");
    for (int i = 0; i < k_i; i++) {
        for (int j = 0; j < k_j; j++) {
            printf("%d ", playground[i * k_j + j]);
        }
        printf("\n");
    }
    return;
}

void update_playground(int k_i, int k_j, int* playground) {
    for (int i = 0; i < k_i; i++) {
        for (int j = 0; j < k_j; j++) {
            upgrade_cell(i, j, k_i, k_j, playground);
        }
    }
    return;
}

void update_playground2(int k_i, int k_j, int* playground) {
    int tmp_playground[k_i][k_j];
#pragma omp parallel for collapse(2)
    for (int i = 0; i < k_i; i++) {
        for (int j = 0; j < k_j; j++) {
            tmp_playground[i][j] = upgrade_cell2(i, j, k_i, k_j, playground);
        }
    }

// Copy tmp_playground back to the original playground
#pragma omp parallel for collapse(2)
    for (int i = 0; i < k_i; i++) {
        for (int j = 0; j < k_j; j++) {
            playground[i * k_j + j] = tmp_playground[i][j];
        }
    }
}

int upgrade_cell2(int c_i, int c_j, int k_i, int k_j, int* playground) {
    int neighbors = 0;
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            if (i == 0 && j == 0)
                continue;
            int n_i = (c_i + i + k_i) % k_i;
            int n_j = (c_j + j + k_j) % k_j;
            if (playground[n_i * k_j + n_j] == 1)
                neighbors++;
        }
    }
    int idx = c_i * k_j + c_j;
    if (playground[idx] == 1) {
        if (neighbors < 2 || neighbors > 3) {
            return 0;
        } else {
            return 1;
        }
    } else {
        // if (neighbors == 3 || neighbors == 2) {
        if (neighbors == 3) {
            return 1;
        } else {
            return 0;
        }
    }
}

void update_playground_parallel(int k_i, int k_j, int* playground) {
#pragma omp parallel for collapse(2)
    for (int i = 0; i < k_i; i++) {
        for (int j = 0; j < k_j; j++) {
            upgrade_cell(i, j, k_i, k_j, playground);
        }
    }
    return;
}

void upgrade_cell(int c_i, int c_j, int k_i, int k_j, int* playground) {
    int neighbors = 0;
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            if (i == 0 && j == 0)
                continue;
            int n_i = (c_i + i + k_i) % k_i;
            int n_j = (c_j + j + k_j) % k_j;
            if (playground[n_i * k_j + n_j] == 1)
                neighbors++;
        }
    }
    int idx = c_i * k_j + c_j;
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

// PGM code
// void export_pgm_image(int *playground, int maxval, int k_i, int k_j, const
// char *image_name) {
//     write_pgm_image(playground, maxval, k_i, k_j, image_name);
//     return;
// }

// create a function that generates a pgm image from a given matrix of just 2
// values 0 and 1 and saves it to a file
void generate_pgm_image(int* playground,
                        int maxval,
                        int k_i,
                        int k_j,
                        const char* image_name) {
    // create a new matrix with the same dimensions as the playground
    int new_playground[k_i][k_j];
    int* p = &new_playground[0][0];
    // fill the matrix with the values 0 and 255
    for (int i = 0; i < k_i; i++) {
        for (int j = 0; j < k_j; j++) {
            if (playground[i * k_j + j] == 1) {
                new_playground[i][j] = 255;
            } else {
                new_playground[i][j] = 0;
            }
        }
    }
    // save the matrix to a file
    write_pgm_image(p, maxval, k_i, k_j, image_name);
    return;
}

void generate_pgm_image2(int* playground,
                         int maxval,
                         int k_i,
                         int k_j,
                         const char* image_name) {
    char* cImage;
    cImage = (char*)calloc(k_i * k_j, sizeof(char));
    unsigned char _maxval = (char)maxval;
    int idx = 0;
    void* ptr;
    ptr = (void*)cImage;
    // fill the matrix with the values 0 and 255
    for (int i = 0; i < k_i; i++) {
        for (int j = 0; j < k_j; j++) {
            if (playground[i * k_j + j] == 1) {
                cImage[idx++] = -1;
            } else {
                cImage[idx++] = 0;
            }
        }
    }
    // save the matrix to a file
    write_pgm_image(ptr, maxval, k_i, k_j, image_name);
    return;
}

void write_pgm_image(void* image,
                     int maxval,
                     int xsize,
                     int ysize,
                     const char* image_name)
/*
 * image        : a pointer to the memory region that contains the image
 * maxval       : either 255 or 65536
 * xsize, ysize : x and y dimensions of the image
 * image_name   : the name of the file to be written
 *
 */
{
    FILE* image_file;
    image_file = fopen(image_name, "w");

    // Writing header
    // The header's format is as follows, all in ASCII.
    // "whitespace" is either a blank or a TAB or a CF or a LF
    // - The Magic Number (see below the magic numbers)
    // - the image's width
    // - the height
    // - a white space
    // - the image's height
    // - a whitespace
    // - the maximum color value, which must be between 0 and 65535
    //
    // if he maximum color value is in the range [0-255], then
    // a pixel will be expressed by a single byte; if the maximum is
    // larger than 255, then 2 bytes will be needed for each pixel
    //

    int color_depth = 1 + (maxval > 255);

    fprintf(image_file, "P5\n# generated by\n# put here your name\n%d %d\n%d\n",
            xsize, ysize, maxval);

    // Writing file
    fwrite(image, 1, xsize * ysize * color_depth, image_file);

    fclose(image_file);
    return;

    /* ---------------------------------------------------------------

       TYPE    MAGIC NUM     EXTENSION   COLOR RANGE
             ASCII  BINARY

       PBM   P1     P4       .pbm        [0-1]
       PGM   P2     P5       .pgm        [0-255]
       PPM   P3     P6       .ppm        [0-2^16[

    ------------------------------------------------------------------ */
}

void read_pgm_image(void** image,
                    int* maxval,
                    int* xsize,
                    int* ysize,
                    const char* image_name)
/*
 * image        : a pointer to the pointer that will contain the image
 * maxval       : a pointer to the int that will store the maximum intensity in
 * the image xsize, ysize : pointers to the x and y sizes image_name   : the
 * name of the file to be read
 *
 */
{
    FILE* image_file;
    image_file = fopen(image_name, "r");

    *image = NULL;
    *xsize = *ysize = *maxval = 0;

    char MagicN[2];
    char* line = NULL;
    size_t k, n = 0;

    // get the Magic Number
    k = fscanf(image_file, "%2s%*c", MagicN);

    // skip all the comments
    k = getline(&line, &n, image_file);
    while ((k > 0) && (line[0] == '#'))
        k = getline(&line, &n, image_file);

    if (k > 0) {
        k = sscanf(line, "%d%*c%d%*c%d%*c", xsize, ysize, maxval);
        if (k < 3)
            fscanf(image_file, "%d%*c", maxval);
    } else {
        *maxval = -1;  // this is the signal that there was an I/O error
                       // while reading the image header
        free(line);
        return;
    }
    free(line);

    int color_depth = 1 + (*maxval > 255);
    unsigned int size = *xsize * *ysize * color_depth;

    if ((*image = (char*)malloc(size)) == NULL) {
        fclose(image_file);
        *maxval = -2;  // this is the signal that memory was insufficient
        *xsize = 0;
        *ysize = 0;
        return;
    }

    if (fread(*image, 1, size, image_file) != size) {
        free(image);
        image = NULL;
        *maxval = -3;  // this is the signal that there was an i/o error
        *xsize = 0;
        *ysize = 0;
    }

    fclose(image_file);
    return;
}
