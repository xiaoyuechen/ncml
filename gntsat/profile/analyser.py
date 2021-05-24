import json


def ComputeResults(result_list):
    if (len(result_list) == 0): return 0, 0, 0
    afs = 0
    at = 0
    for fs, t in result_list:
        afs += fs
        at += t
    afs = afs / len(result_list)
    at = at / len(result_list)
    sr = len(result_list) / 5
    return afs, at, sr


with open('gnt_results.json', 'r') as infile:
    gnt_results = json.load(infile)

with open('walksat_result.json', 'r') as infile:
    walksat_results = json.load(infile)

for benchmark, results in gnt_results.items():
    print(f"<<<<{ benchmark }>>>>")
    print("====GNTSAT====")
    for crossover, result_list in results.items():
        print(crossover)
        afs, at, sr = ComputeResults(result_list)
        print(f"AFS={afs}")
        print(f"AT={at}")
        print(f"SR={sr}")
    print("====WALKSAT====")
    afs, at, sr = ComputeResults(walksat_results[benchmark])
    print(f"AFS={afs}")
    print(f"AT={at}")
    print(f"SR={sr}")
    print()
