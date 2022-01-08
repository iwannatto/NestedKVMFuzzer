import argparse
import pathlib

import compare_to_selftest
import make_procedure

def dir_or_file_path(s):
    p = pathlib.Path(s)
    if p.exists():
        return p
    else:
        raise FileNotFoundError

def dir_path(s):
    p = dir_or_file_path(s)
    if p.is_dir():
        return p
    else:
        raise NotADirectoryError

def main():
    parser = argparse.ArgumentParser()
    # fuzzing_result_dir is like ../fuzzer/results/20211231_201627
    parser.add_argument("fuzzing_result_dir", type=dir_path)
    parser.add_argument("selftest_coverage_dir", type=dir_path)
    parser.add_argument("--output_dir")
    args = parser.parse_args()
    if args.output_dir:
        args.output_dir = dir_path(args.output_dir)
    else:
        # default of output_dir is like ./20211231_201627
        args.output_dir = pathlib.Path(".") / args.fuzzing_result_dir.name

    args.output_dir.mkdir(exist_ok=True)

    make_procedure.make_procedure(args.fuzzing_result_dir, args.output_dir)

    compare_to_selftest.compare_to_selftest(args.output_dir, args.selftest_coverage_dir)

if __name__ == "__main__":
    main()