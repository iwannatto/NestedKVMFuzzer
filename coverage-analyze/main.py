import argparse
import array
import pathlib

def read_into_array(coverage_dir_or_file_path):
    def read_from_file_into_array(filepath):
        file = open(filepath, "rb")
        array_ = array.array("L", file.read())
        return array_

    if coverage_dir_or_file_path.is_file():
        return read_from_file_into_array(coverage_dir_or_file_path)
    elif coverage_dir_or_file_path.is_dir():
        array_ = array.array("L", [])
        for filepath in coverage_dir_or_file_path.iterdir():
            array_.extend(read_from_file_into_array(filepath))
        return array_

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

def save_coverage_set_as_file(coverage_set, filename):
    list_ = sorted(list(coverage_set))
    list_ = list(map(lambda n: f"0x{n:016x}\n", list_))
    with open(filename, "w") as f:
        f.writelines(list_)

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("coverage_dir_or_file_1", type=dir_or_file_path)
    parser.add_argument("coverage_dir_or_file_2", type=dir_or_file_path)
    parser.add_argument("--output_dir", default=".", type=dir_path)
    args = parser.parse_args()

    coverage_array1 = read_into_array(args.coverage_dir_or_file_1)
    coverage_array2 = read_into_array(args.coverage_dir_or_file_2)

    coverage_set1 = set(coverage_array1)
    coverage_set2 = set(coverage_array2)

    set1_only_addresses = coverage_set1 - coverage_set2
    set2_only_addresses = coverage_set2 - coverage_set1
    common_addresses = coverage_set1 & coverage_set2
    print("set1 only addresses:", len(set1_only_addresses), list(set1_only_addresses)[:10])
    print("set2 only addresses:", len(set2_only_addresses), list(set2_only_addresses)[:10])
    print("common addresses:", len(common_addresses), list(common_addresses)[:10])

    save_coverage_set_as_file(set1_only_addresses, args.output_dir / "set1_only.cov")
    save_coverage_set_as_file(set2_only_addresses, args.output_dir / "set2_only.cov")
    save_coverage_set_as_file(common_addresses, args.output_dir / "common.cov")

if __name__ == "__main__":
    main()