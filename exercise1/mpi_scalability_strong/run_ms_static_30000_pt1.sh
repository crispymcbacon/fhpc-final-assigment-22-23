#!/bin/bash
#SBATCH --no-requeue
#SBATCH --job-name="3_mpi_strong_1"
#SBATCH --partition=THIN
#SBATCH -N 2
#SBATCH -n 48
#SBATCH --exclusive
#SBATCH --time=02:00:00
#SBATCH --nodelist=thin[002-003]

# Set the environment
module load openMPI/4.1.5/gnu/12.2.1
policy=close # close means threads are bound to cores in the same socket
export OMP_PLACES=cores # determine where threads are placed
export OMP_PROC_BIND=$policy # determine how threads are bound to cores
export OMP_NUM_THREADS=1 # 1 thread per core
size=30000 # size of the matrix

# Compile the program
cd ..
make par

# Run the program
mpirun -np 1 -N 1 --map-by socket main.x -i -f miofile_$size -k $size
for procs in 1 $(seq 2 2 24)
do
	  mpirun -np $procs -N 2 --map-by core main.x -r -f miofile_$size -e 1 -n 30 -s 0 -k $size -t "-n$procs -N2 THIN -k$size -m:core" -l "log_ms_static_30_1_$size.csv"
done