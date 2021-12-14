dir1='../linux-kcov/linux-kcov-src/selftest-coverage'
dir2='../fuzzer/results/20211202_231659/coverage'

python3 main.py "${dir1}" "${dir2}"

vmlinux='../linux-kcov/linux-kcov-src/vmlinux'

for stem in 'set1_only' 'set2_only' 'common'; do
    addr2line --exe="${vmlinux}" --functions < "${stem}.cov" > "${stem}.txt"
done