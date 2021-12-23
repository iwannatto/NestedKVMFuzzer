import argparse
import array
import pathlib

def read_from_file_into_array(filepath):
    file = open(filepath, "rb")
    array_ = array.array("L", file.read())
    return array_

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("coverage_binary", type=argparse.FileType("rb"))
    parser.add_argument("output", type=argparse.FileType("w"))
    args = parser.parse_args()

    binary_array = array.array("L", args.coverage_binary.read())
    cov_list = list(map(lambda n: f"0x{n:016x}\n", binary_array))
    args.output.writelines(cov_list)

if __name__ == "__main__":
    main()