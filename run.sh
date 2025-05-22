#!/bin/bash

# Default number of processes
NUM_PROCESSES=${1:-4}

mpicc tspBruteForceParallel.c -o tspBruteForceParallel -lm && mpirun -np $NUM_PROCESSES ./tspBruteForceParallel