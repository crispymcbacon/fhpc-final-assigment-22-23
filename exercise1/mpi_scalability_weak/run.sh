#!/bin/bash
#SBATCH --no-requeue
#SBATCH --job-name="mpi_weak"
#SBATCH --partition=THIN
#SBATCH -N 3
#SBATCH -n 36
#SBATCH --exclusive
#SBATCH --time=02:00:00
#SBATCH --nodelist=thin[002-003]

cd ..

# Set the environment
module load openMPI/4.1.5/gnu/12.2.1
policy=close # close means threads are bound to cores in the same socket
export OMP_PLACES=cores # determine where threads are placed
export OMP_PROC_BIND=$policy # determine how threads are bound to cores

## initialize a playgrounds
export OMP_NUM_THREADS=12
for size in 10000 14143 17321 20000 22361 24495
do
	mpirun -np 1 -N 1 --map-by socket main.o -i -f playground_${size} -k $size
done


mpirun -np 1 -N 3 --map-by socket main.o -r -n 5 -e 0 -s 0 -f playground_10000 -k 10000 -t "-n1 -N3 THIN -k10000 -m:socket" -l "log_mw_ord_$size.csv"
mpirun -np 1 -N 3 --map-by socket main.o -r -n 5 -e 1 -s 0 -f playground_10000 -k 10000 -t "-n1 -N3 THIN -k10000 -m:socket" -l "log_mw_static_$size.csv"

mpirun -np 2 -N 3 --map-by socket main.o -r -n 5 -e 0 -s 0 -f playground_14143 -k 14143 -t "-n2 -N3 THIN -k14143 -m:socket" -l "log_mw_ord_$size.csv"
mpirun -np 2 -N 3 --map-by socket main.o -r -n 5 -e 1 -s 0 -f playground_14143 -k 14143 -t "-n2 -N3 THIN -k14143 -m:socket" -l "log_mw_static_$size.csv"

mpirun -np 3 -N 3 --map-by socket main.o -r -n 5 -e 0 -s 0 -f playground_17321 -k 17321 -t "-n3 -N3 THIN -k17321 -m:socket" -l "log_mw_ord_$size.csv"
mpirun -np 3 -N 3 --map-by socket main.o -r -n 5 -e 1 -s 0 -f playground_17321 -k 17321 -t "-n3 -N3 THIN -k17321 -m:socket" -l "log_mw_static_$size.csv"

mpirun -np 4 -N 3 --map-by socket main.o -r -n 5 -e 0 -s 0 -f playground_20000 -k 20000 -t "-n4 -N3 THIN -k20000 -m:socket" -l "log_mw_ord_$size.csv"
mpirun -np 4 -N 3 --map-by socket main.o -r -n 5 -e 1 -s 0 -f playground_20000 -k 20000 -t "-n4 -N3 THIN -k20000 -m:socket" -l "log_mw_static_$size.csv"

mpirun -np 5 -N 3 --map-by socket main.o -r -n 5 -e 0 -s 0 -f playground_22361 -k 22361 -t "-n5 -N3 THIN -k22361 -m:socket" -l "log_mw_ord_$size.csv"
mpirun -np 5 -N 3 --map-by socket main.o -r -n 5 -e 1 -s 0 -f playground_22361 -k 22361 -t "-n5 -N3 THIN -k22361 -m:socket" -l "log_mw_static_$size.csv"

mpirun -np 6 -N 3 --map-by socket main.o -r -n 5 -e 0 -s 0 -f playground_24495 -k 24495 -t "-n6 -N3 THIN -k24495 -m:socket" -l "log_mw_ord_$size.csv"
mpirun -np 6 -N 3 --map-by socket main.o -r -n 5 -e 1 -s 0 -f playground_24495 -k 24495 -t "-n6 -N3 THIN -k24495 -m:socket" -l "log_mw_static_$size.csv"