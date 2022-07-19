import argparse
from ctypes.wintypes import RGB
from pathlib import Path
from matplotlib import pyplot as plt 
import random

def read_data(path: Path):
    with path.open() as file:
        bytes = [ int(line.split(",")[0]) for line in file.readlines() ]
    with path.open() as file:
        times = [ int(line.split(",")[1]) for line in file.readlines() ]
    return bytes, times

def median(arr: list):
    arr.sort()
    if len(arr) % 2:
        return arr[len(arr) // 2]
    else:
        return (arr[len(arr) // 2] + arr[len(arr) // 2 - 1]) / 2

def draw_data(x, y):
    plt.scatter(x, y)
    plt.show()

def main():
    arg_parser = argparse.ArgumentParser()
    arg_parser.add_argument("--path", "-p", help="Path to folder with results.", type=Path, required=True)
    arg_parser.add_argument("--num", "-n", help="Files num.", type=int, required=True)
    args = arg_parser.parse_args()

    bytes_per_t = {}
    for i in range(args.num):
        bytes, times = read_data((args.path / f"results{i}").with_suffix(".txt"))
        for j, b in enumerate(bytes):
            if b not in bytes_per_t:
                bytes_per_t[b] = []
            bytes_per_t[b].append(b / times[j])
    
    for b in bytes_per_t:
        bytes_per_t[b] = min(bytes_per_t[b])
    
    print(bytes_per_t)
    print(max(bytes_per_t, key=bytes_per_t.get))
    print({k: v for k, v in sorted(bytes_per_t.items(), key=lambda item: item[1])})


if __name__ == "__main__":
    main()