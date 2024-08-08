#!/bin/bash
#PBS -P cd85
#PBS -q normal

module load singularity

set -x

cd /scratch/cd85/dc6693/pushworld

./run_planner $HEURISTIC "$PROBLEM"
