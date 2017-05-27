#!/bin/bash
for i in {0..10} 
do
	echo "$i"
	exec mpirun -n 4 2_MPI_Ring >> test.txt
	echo "$?"
done
