import array
import argparse
import os
import pathlib
import tempfile

import lib

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("src", type=pathlib.Path)
    parser.add_argument("dst", type=pathlib.Path)
    args = parser.parse_args()

    p = pathlib.Path()

    if args.src.suffix == ".bin" and args.dst.suffix == ".cov":
        s = set()
        lib.read_into_set(args.src, s)
        _, t = tempfile.mkstemp()
        try:
            with open(t, "wb") as f:
                a = array.array("Q", s)
                a.tofile(f)
            lib.bin_to_cov(t, args.dst)
        finally:
            print("failed")
            os.remove(t)
    elif args.src.suffix == ".cov" and args.dst.suffix == ".txt":
        lib.cov_to_txt(args.src, args.dst)

if __name__ == "__main__":
    main()