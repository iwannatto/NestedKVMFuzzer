import csv
import pathlib

import lib

def compare_to_selftest(output_dir: pathlib.Path, selftest_coverage_dir: pathlib.Path):
    comparison_output_dir = output_dir / "selftest"
    comparison_output_dir.mkdir(exist_ok=True)
    sizes_filename = comparison_output_dir / "sizes.csv"
    if sizes_filename.exists():
        return

    def save_set_as_ubin_cov_txt_in_comparison_output_dir(s: set, stem: str):
        lib.save_set_as_ubin_cov_txt(s, comparison_output_dir, stem)

    procedure_ubin_dir = output_dir / "procedure" / "ubin"
    fuzzing_all = set()
    lib.read_into_set(procedure_ubin_dir / "fuzzing_all.ubin", fuzzing_all)
    fuzzing_diff = set()
    lib.read_into_set(procedure_ubin_dir / "fuzzing_diff.ubin", fuzzing_diff)

    selftest_all = set()
    for f in selftest_coverage_dir.iterdir():
        lib.read_into_set(f, selftest_all)

    sizes = [("category", "size")]
    comparison = {
        "fuzzing_all_only": fuzzing_all - selftest_all,
        "fuzzing_diff_only": fuzzing_diff - selftest_all,
        "selftest_only": selftest_all - fuzzing_all,
        "common": fuzzing_all & selftest_all,
    }
    for name, s in comparison.items():
        save_set_as_ubin_cov_txt_in_comparison_output_dir(s, name)
        sizes.append((name, len(s)))
    with open(sizes_filename, "w") as f:
        writer = csv.writer(f)
        writer.writerows(sizes)
