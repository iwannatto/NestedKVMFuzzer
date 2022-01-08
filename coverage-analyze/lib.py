import array
import pathlib
import subprocess

def read_into_set(filepath: pathlib.Path, s: set):
    with open(filepath, "rb") as f:
        bs = 8
        n = 10000
        while True:
            address = f.read(bs * n)
            if not address:
                break
            arr = array.array("Q", address)
            for a in arr:
                s.add(a)

def bin_to_cov(bin: pathlib.Path, cov: pathlib.Path):
    bin_array = None
    with open(bin, "rb") as f:
        bin_array = array.array("Q", f.read())
    cov_list = list(map(lambda n: f"0x{n:016x}\n", bin_array))
    with open(cov, "w") as f:
        f.writelines(cov_list)

def cov_to_txt(cov: pathlib.Path, txt: pathlib.Path):
    vmlinux = "../linux-kcov/linux-kcov-src/vmlinux"
    with open(cov, "r") as covf, open(txt, "w") as txtf:
        subprocess.run(["addr2line", f"--exe={vmlinux}", "--functions"], stdin=covf, stdout=txtf)

def save_set_as_ubin_cov_txt(s: set, dir: pathlib.Path, stem: str):
    ubin_dir = dir / "ubin"
    ubin_dir.mkdir(exist_ok=True)
    cov_dir = dir / "cov"
    cov_dir.mkdir(exist_ok=True)
    txt_dir = dir / "txt"
    txt_dir.mkdir(exist_ok=True)

    # save as ubin
    procedure_ubin_filename = ubin_dir / f"{stem}.ubin"
    with open(procedure_ubin_filename, "wb") as procedure_ubin:
        array.array("Q", s).tofile(procedure_ubin)
    # save as cov
    procedure_cov_filename = cov_dir / f"{stem}.cov"
    bin_to_cov(procedure_ubin_filename, procedure_cov_filename)
    # save as txt
    procedure_txt_filename = txt_dir / f"{stem}.txt"
    cov_to_txt(procedure_cov_filename, procedure_txt_filename)

