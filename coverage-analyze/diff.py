import argparse
import array

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("ubin1", type=argparse.FileType("rb"))
    parser.add_argument("ubin2", type=argparse.FileType("rb"))
    parser.add_argument("out", type=argparse.FileType("wb"))
    args = parser.parse_args()

    set1 = set(array.array("Q", args.ubin1.read()))
    set2 = set(array.array("Q", args.ubin2.read()))

    diff = set1 - set2
    array.array("Q", diff).tofile(args.out)

if __name__ == "__main__":
    main()