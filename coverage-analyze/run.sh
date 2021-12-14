dir1='../linux-kcov/linux-kcov-src/selftest-coverage'
dir2='../fuzzer/results/20211202_231659/coverage'

python3 main.py "${dir1}" "${dir2}"
