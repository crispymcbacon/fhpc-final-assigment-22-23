// Update the playground in the static way
int* update_playground_static(const int k_i, const int k_j, int *playground) {

    // Allocate memory for playground B
    int *playground_b = (int*) malloc(k_i * k_j * sizeof(int));

    // Copy the contents of playground A to playground B
    memcpy(playground_b, playground, k_i * k_j * sizeof(int));

    // Update each cell in playground B
    for (int i = 0; i < k_i; i++){
        for (int j = 0; j < k_j; j++){
            playground_b[i*k_j + j] = upgrade_cell_static(i, j, k_i, k_j, playground);
        }
    }

    // Return the pointer to playground B
    return playground_b;
}