# Exercise 1: Parallelizing Conway's Game of Life

This directory contains the source code and related files for the first exercise. Here's a brief description of each file:

- `dev.h`: This header file contains development-related functions such as `append_to_logs` for logging and `log_error` for error handling.
- `evolution.h`: This header file contains functions related to the evolution of the playground, such as `update_playground_static` and `print_playground`.
- `generate_video.sh`: This is a shell script used to generate a video from the output of the program.
- `main_vanilla_clean.c`: This is the main C file for the vanilla version of the program. It includes functions like `print_playground` and `update_playground_chessboard`.
- `main.c`: This is the main C file for the parallelized version of the program. It includes functions like `evolve_playground` and `run_playground`.
- [``Makefile``]: This file is used to compile the C files into an executable program.
- `mpi_scalability_strong/` and `mpi_scalability_weak/`: These directories contain the logs for the strong and weak scalability tests of the MPI version of the program.
- `omp_scalability/`: This directory contains the logs for the scalability tests of the OpenMP version of the program.
- `out.nosync/`: This directory contains the output files of the program.
- `pgm.h`: This header file contains functions related to the PGM image format.
- [``README.md``]: This is the file you're currently reading.

## Software Stack

To compile and run the programs in this directory, you need the following software stack:

- GCC: This is the compiler used to compile the C files. It's included in the `CC` variable in the [``Makefile``].
- MPI: This is used for parallelizing the program across multiple processes. The `mpicc` command, which is included in the `CC` variable in the [``Makefile``], is used to compile the MPI version of the program.
- OpenMP: This is used for parallelizing the program across multiple threads. The `-fopenmp` flag, which is included in the `CC` variable in the [``Makefile``], is used to compile the OpenMP version of the program.

## Scripts

The `generate_video.sh` script is used to generate a video from the output of the program. It takes the output images and combines them into a single video file.

## Datasets

The `.csv` files in the `mpi_scalability_strong/`, `mpi_scalability_weak/`, and `omp_scalability/` directories contain the results of the scalability tests. Each row in these files represents a single test run, and the columns are comma-separated values that represent different metrics collected during the run. These datasets can be used to produce figures that show the scalability of the program.