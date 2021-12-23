set -eux

datetime='20211218_181151'

dir1='../linux-kcov/linux-kcov-src/selftest-coverage'
dir2="../fuzzer/results/${datetime}/coverage"

mkdir -p "${datetime}"

python3 main.py "${dir1}" "${dir2}" --output_dir "${datetime}"

vmlinux='../linux-kcov/linux-kcov-src/vmlinux'

for stem in 'set1_only' 'set2_only' 'common'; do
    addr2line --exe="${vmlinux}" --functions < "${datetime}/${stem}.cov" > "${datetime}/${stem}.txt"
done