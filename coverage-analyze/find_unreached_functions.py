import argparse
import pathlib
from typing import ContextManager

def existing_file(s):
    p = pathlib.Path(s)
    if p.exists():
        return p
    else:
        raise FileNotFoundError

def make_ctags_set(ctags):
    ctags_list = []
    with open(ctags) as f:
        for line in f:
            ctags_list.append(line.split(maxsplit=1)[0])
    return set(ctags_list)

def make_cov_set(cov_txt):
    lines = None
    with open(cov_txt) as f:
        lines = [line.strip() for line in f.readlines()]
    return set(lines[::2])

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--ctags", type=existing_file)
    parser.add_argument("--cov_txts", nargs="+", type=existing_file)
    parser.add_argument("--output", type=pathlib.Path)
    args = parser.parse_args()

    ctags_set = make_ctags_set(args.ctags)

    for cov_txt in args.cov_txts:
        cov_set = make_cov_set(cov_txt)
        ctags_set -= cov_set

    with open(args.output, mode="w") as f:
        f.write('\n'.join(list(ctags_set)))

if __name__ == "__main__":
    main()