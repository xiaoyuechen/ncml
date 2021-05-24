import subprocess
import re
import json


def time_flip(output):
    global data_regx
    fs_begin = output.rfind('FS', 0, len(output))
    fs = int(data_regx.findall(output[fs_begin:])[0])
    time_begin = output.rfind('TIME', 0, len(output))
    time = int(data_regx.findall(output[time_begin:])[0])
    return (fs, time)


gnt_sat_path = "../cmake-build-debug/gntsat"
walksat_path = "../cmake-build-debug/walksat"

crossover_types = ["cc", "ff", "uniform", "onepoint", "twopoint"]
benchmark_paths = [
    "../benchmark/uf-250-1065/uf250-01.cnf",
    # "../benchmark/uf-250-1065/uf250-02.cnf",
    # "../benchmark/uf-250-1065/uf250-03.cnf",
]

data_regx = re.compile(".*: (\d+)")
k_times = 10
timeout = 5 * 60
gnt_results = {}
walksat_results = {}
for benchmark in benchmark_paths:
    gnt_results[benchmark] = {}
    for crossover in crossover_types:
        gnt_results[benchmark][crossover] = []
    walksat_results[benchmark] = []

for benchmark in benchmark_paths:
    for crossover in crossover_types:
        for _ in range(k_times):
            try:
                result = subprocess.run(
                    [gnt_sat_path, "--crossover=" + crossover, benchmark],
                    capture_output=True,
                    text=True,
                    timeout=timeout)
                output = result.stdout
                gnt_results[benchmark][crossover].append(time_flip(output))
            except subprocess.TimeoutExpired:
                pass

            print(gnt_results)

    for _ in range(k_times):
        try:
            result = subprocess.run([walksat_path, benchmark],
                                    capture_output=True,
                                    text=True,
                                    timeout=timeout)
            output = result.stdout
            walksat_results[benchmark].append(time_flip(output))

        except subprocess.TimeoutExpired:
            pass

        print(walksat_results)

with open('gnt_results.json', 'w') as outfile:
    json.dump(gnt_results, outfile)
with open('walksat_result.json', 'w') as outfile:
    json.dump(walksat_results, outfile)
