#!/bin/bash

# Default number of processes
NUM_PROCESSES=${1:-4}
CITIES =${2:-10}

mpicc tspBruteForceParallel.c -o tspBruteForceParallel -lm && mpirun -np $NUM_PROCESSES ./tspBruteForceParallel $CITIES