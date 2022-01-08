import array
import csv
import pathlib
import re

import lib

def extract_time(coverage_filename: pathlib.Path) -> str:
    s = str(coverage_filename.name)
    p = re.compile("time:(\d+)")
    m = p.search(s)
    return m.group(1)

def make_procedure(fuzzing_result_dir: pathlib.Path, output_dir: pathlib.Path):
    procedure_dir = output_dir / "procedure"
    procedure_dir.mkdir(exist_ok=True)
    def save_set_as_ubin_cov_txt_in_procedure_dir(s: set, stem: str):
        lib.save_set_as_ubin_cov_txt(s, procedure_dir, stem)

    procedure_filename = procedure_dir / "procedure.csv"
    if procedure_filename.exists():
        return

    coverage_filenames = [filename for filename in (fuzzing_result_dir / "coverage").iterdir()]
    coverage_filenames.sort()

    accumulated_coverage = set()
    first_coverage = None
    procedure = [("time", "coverage")]
    for coverage_filename in coverage_filenames:
        print(f"processing {coverage_filename}")

        coverage = set()
        lib.read_into_set(coverage_filename, coverage)
        if len(accumulated_coverage) == 0:
            first_coverage = coverage

        # save diff
        diff = coverage - accumulated_coverage
        coverage_filename_stem = coverage_filename.stem
        save_set_as_ubin_cov_txt_in_procedure_dir(diff, coverage_filename_stem)

        accumulated_coverage |= diff

        procedure.append((extract_time(coverage_filename), len(accumulated_coverage)))

    save_set_as_ubin_cov_txt_in_procedure_dir(accumulated_coverage, "fuzzing_all")
    save_set_as_ubin_cov_txt_in_procedure_dir(accumulated_coverage - first_coverage, "fuzzing_diff")

    with open(procedure_filename, "w") as f:
        writer = csv.writer(f)
        writer.writerows(procedure)

