#include <mpi.h>
#include <omp.h>

int upgrade_cell_ordered(int c_i, int c_j, int k, int *playground, int *top_ghost_row, int *bottom_ghost_row) {
    int n_i, n_j, neighbors = 0;
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            if (i == 0 && j == 0) continue;
            n_i = (c_i + i + k) % k;
            n_j = (c_j + j + k) % k;
            if (n_i == 0 && c_i == k - 1) {
                neighbors += top_ghost_row[n_j];
            } else if (n_i == k - 1 && c_i == 0) {
                neighbors += bottom_ghost_row[n_j];
            } else {
                neighbors += playground[n_i * k + n_j];
            }
        }
    }

    int current_state = playground[c_i * k + c_j];
    if ((current_state == 1 && (neighbors == 2 || neighbors == 3)) ||
        (current_state == 0 && neighbors == 3)) {
        return 1;
    } else {
        return 0;
    }
}

void update_playground_ordered(int k, int *playground, int rank, int num_procs) {

    int *temp_playground = (int *)malloc(k * k * sizeof(int));
    int *top_ghost_row = (int *)malloc(k * sizeof(int));
    int *bottom_ghost_row = (int *)malloc(k * sizeof(int));

    int top_neighbor = (num_procs > 1) ? (rank - 1 + num_procs) % num_procs : 0;
    int bottom_neighbor = (num_procs > 1) ? (rank + 1) % num_procs : 0;

    MPI_Request request[4];
    MPI_Status status[4];

    MPI_Isend(&playground[0], k, MPI_INT, top_neighbor, 1, MPI_COMM_WORLD, &request[0]);
    MPI_Irecv(bottom_ghost_row, k, MPI_INT, bottom_neighbor, 1, MPI_COMM_WORLD, &request[1]);
    MPI_Isend(&playground[(k - 1) * k], k, MPI_INT, bottom_neighbor, 0, MPI_COMM_WORLD, &request[2]);
    MPI_Irecv(top_ghost_row, k, MPI_INT, top_neighbor, 0, MPI_COMM_WORLD, &request[3]);

    int chunk_size = k / num_procs;
    int remainder = k % num_procs;
    int start = rank * chunk_size + ((rank < remainder) ? rank : remainder);
    int end = start + chunk_size + (rank < remainder);

    #pragma omp parallel for collapse(2)
    for (int i = start; i < end; i++) {
        for (int j = 0; j < k; j++) {
            temp_playground[i * k + j] = upgrade_cell_ordered(i, j, k, playground, top_ghost_row, bottom_ghost_row);
        }
    }

    MPI_Waitall(4, request, status);

    memcpy(playground, temp_playground, k * k * sizeof(int));

    free(temp_playground);
    free(top_ghost_row);
    free(bottom_ghost_row);
}

void update_cell_static(int i, int j, int k, int *playground, int *temp_playground) {
    int alive_neighbors = 0;

    for (int di = -1; di <= 1; di++) {
        for (int dj = -1; dj <= 1; dj++) {
            if (di == 0 && dj == 0) continue;

            int ni = (i + di + k) % k;
            int nj = (j + dj + k) % k;

            alive_neighbors += playground[ni * k + nj];
        }
    }

    int cell = playground[i * k + j];
    if (cell == 1 && (alive_neighbors < 2 || alive_neighbors > 3)) {
        temp_playground[i * k + j] = 0;
    } else if (cell == 0 && alive_neighbors == 3) {
        temp_playground[i * k + j] = 1;
    } else {
        temp_playground[i * k + j] = cell;
    }
}

void update_playground_static(int k, int *playground, int rank, int num_procs) {
    int rows_per_proc = k / num_procs;
    int remainder = k % num_procs;
    int start_row = rank * rows_per_proc + (rank < remainder ? rank : remainder);
    int end_row = start_row + rows_per_proc + (rank < remainder ? 1 : 0);

    int *temp_playground = (int *)calloc(k * k, sizeof(int));

    MPI_Request request;

    // Send top ghost row to previous process and receive from next process
    if (rank > 0) {
        MPI_Isend(playground + start_row * k, k, MPI_INT, rank - 1, 0, MPI_COMM_WORLD, &request);
    }
    if (rank < num_procs - 1) {
        MPI_Recv(playground + end_row * k, k, MPI_INT, rank + 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    // Send bottom ghost row to next process and receive from previous process
    if (rank < num_procs - 1) {
        MPI_Isend(playground + (end_row - 1) * k, k, MPI_INT, rank + 1, 0, MPI_COMM_WORLD, &request);
    }
    if (rank > 0) {
        MPI_Recv(playground + (start_row - 1) * k, k, MPI_INT, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    // Parallel computation on each node
    #pragma omp parallel for collapse(2)
    for (int i = start_row; i < end_row; i++) {
        for (int j = 0; j < k; j++) {
            update_cell_static(i, j, k, playground, temp_playground);
        }
    }

    memcpy(playground, temp_playground, k * k * sizeof(int));
    free(temp_playground);
}

// The main difference between the two functions in terms of MPI is the order and manner
// in which they exchange the ghost rows with the neighboring processes.
// In `update_playground_ordered`, all send and receive operations are initiated at
// the same time and the function waits for all of them to complete before it proceeds.
// In `update_playground_static`, the send and receive operations are performed
// in two steps: first for the top ghost row and then for the bottom ghost row.