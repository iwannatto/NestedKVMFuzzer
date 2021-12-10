file1='../selftest-coverage/cr4_cpuid_sync_test.c.bin'
file2='../selftest-coverage/debug_regs.c.bin'

python3 main.py "${file1}" "${file2}"

echo '-----'

dir1='../fuzzer/results/20211202_231659/coverage'

python3 main.py "${file1}" "${dir1}"
python3 main.py "${file2}" "${dir1}"