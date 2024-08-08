#!/bin/bash

set -x

module load gcc
module load boost
module load cmake

cd ../cpp

cmake -B build; cmake --build build

cd build/bin/test

./run_tests

cd ../../..

cp build/bin/run_planner ../run_planner

cd ..
