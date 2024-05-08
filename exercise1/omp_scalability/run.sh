#!/bin/bash
#SBATCH --no-requeue
#SBATCH --job-name="omp_scalability"
#SBATCH --partition=THIN
#SBATCH -N 1
#SBATCH -n 24
#SBATCH --exclusive
#SBATCH --time=02:00:00
#SBATCH --nodelist=thin[003]

# Set the environment
module load openMPI/4.1.5/gnu/12.2.1
export OMP_PLACES=cores # determine where threads are placed
export OMP_PROC_BIND=close # determine how threads are bound to cores
size=25000
processes=2

# Compile the program
cd ..
make par

# Run the program
mpirun -np 1 -N 1 main.x -i -k $size -f miofile_omp_$size

for th_socket in $(seq 1 1 12)
do
	export OMP_NUM_THREADS=$th_socket
	mpirun -np $processes --map-by socket main.x -r -f miofile_omp_$size -e 0 -n 5 -s 0 -k $size -t "-n$processes -N1 THIN -k$size -m:socket -numthreads:$th_socket" -l "log_omp_ord_$size.csv"
	mpirun -np $processes --map-by socket main.x -r -f miofile_omp_$size -e 1 -n 10 -s 0 -k $size -t "-n$processes -N1 THIN -k$size -m:socket -numthreads:$th_socket" -l "log_omp_static_$size.csv"
done

cd ..
make clean
module purge