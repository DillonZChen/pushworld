#!/usr/bin/env python3
from itertools import product
import os
import subprocess
import argparse
import json

## paths
# make everything relative to where this script is located
CUR_DIR = os.path.dirname(os.path.abspath(__file__))
# assume you have built the planner
PLANNER = f"{CUR_DIR}/../run_planner"  
assert os.path.exists(PLANNER), PLANNER
LOG_DIR = f"{CUR_DIR}/logs"
os.makedirs(LOG_DIR, exist_ok=True)
JOB_SCRIPT = f"{CUR_DIR}/job.sh"
assert os.path.exists(JOB_SCRIPT), JOB_SCRIPT

PBS_TEST_NCPU = 2
PBS_TEST_TIMEOUT = "00:30:00"
PBS_TEST_MEMOUT = "8GB"

HEURISTICS = [
    "RGD",
    "N+RGD",
]

PROBLEMS = []  # 223 puzzles
for i in [1, 2, 3, 4]:
    puzzle_dir = f"{CUR_DIR}/../benchmark/puzzles/level{i}"
    for f in sorted(os.listdir(puzzle_dir)):
        assert f.endswith(".pwp")
        problem = f"{puzzle_dir}/{f}"
        PROBLEMS.append(problem)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("submissions", type=int)
    parser.add_argument("-f", "--force", action="store_true")
    args = parser.parse_args()

    submissions = args.submissions
    submitted = 0
    skipped_from_log = 0
    to_go = 0

    for h, problem in product(HEURISTICS, PROBLEMS):
        problem_name = os.path.basename(problem)
        problem_name = problem_name.replace(".pwp", "")
        problem_name = problem_name.replace(" ", "-")
        description = f"{h}_{problem_name}"

        log_file = f"{LOG_DIR}/{description}.log"

        if os.path.exists(log_file) and not args.force:
            skipped_from_log += 1
            continue

        if submitted >= submissions:
            to_go += 1
            continue

        job_cmd = [
            "qsub",
            "-o",
            log_file,
            "-j",
            "oe",
            "-N",
            "test_" + description,
            "-l",
            f"ncpus={PBS_TEST_NCPU}",
            "-l",
            f"walltime={PBS_TEST_TIMEOUT}",
            "-l",
            f"mem={PBS_TEST_MEMOUT}",
            "-v",
            f"HEURISTIC={h},PROBLEM={problem}",
            JOB_SCRIPT,
        ]

        p = subprocess.Popen(job_cmd)
        p.wait()
        print(log_file)
        submitted += 1

    print("Submitted:", submitted)
    print("Skipped from log:", skipped_from_log)
    print("To go:", to_go)


if __name__ == "__main__":
    main()
