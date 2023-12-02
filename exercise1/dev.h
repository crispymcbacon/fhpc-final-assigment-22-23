#include <stdio.h>   // for using the standard input and output functions

// DEVELOPMENT ONLY
void append_to_logs(const char *filename, const char *program, int mode, double time_taken, int k, int steps) {
    // Open the file in "a+" mode, which allows both appending and reading.
    FILE *file = fopen("logs.csv", "a+");
    if (file == NULL) {
        printf("Failed to open or create logs.csv\n");
        return;
    }
    // Use ftell to check if the file is empty (i.e., the current position is 0).
    if (ftell(file) == 0) {
        fprintf(file, "file;program;mode;size;step;time_taken\n");  // The file is empty, so add the header line.
    }

    fprintf(file, "%s;%s;%d;%d;%d;%f\n", filename, program, mode, k, steps, time_taken);  // Append the new log data to the file.
    fclose(file);                                                             // Close the file.
}