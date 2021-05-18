import subprocess
import re

gnt_sat_path = "../build/gntsat"
walksat_path = "../build/walksat"

crossover_types = ["cc", "ff", "uniform"]
benchmark_paths = [
    "../benchmark/uf-250-1065/uf250-01.cnf",
    "../benchmark/uf-250-1065/uf250-02.cnf",
    "../benchmark/uf-250-1065/uf250-03.cnf",
]

data_regx = re.compile(".*: (\d+)")
k_times = 10

gnt_results = {}
for benchmark in benchmark_paths:
    gnt_results[benchmark] = {}
    for crossover in crossover_types:
        gnt_results[benchmark][crossover] = []

for benchmark in benchmark_paths:
    for crossover in crossover_types:
        for _ in range(k_times):
            result = subprocess.run(
                [gnt_sat_path, "--crossover=" + crossover, benchmark],
                capture_output=True,
                text=True)
            output = result.stdout
            fs_begin = output.rfind('FS', 0, len(output))
            fs = int(data_regx.findall(output[fs_begin:])[0])
            time_begin = output.rfind('TIME', 0, len(output))
            time = int(data_regx.findall(output[time_begin:])[0])
            gnt_results[benchmark][crossover].append((fs, time))
            print(gnt_results)

