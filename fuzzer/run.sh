# sudo bash -c 'echo core >/proc/sys/kernel/core_pattern'

set -eux

cd $(dirname $0)

# build each component

cd AFLplusplus; make source-only; cd ..
make fuzznetlink
./qemu/build.sh
cd VMXbench; make; cd ..

# run

rm -f debugf_fuzznetlink.txt

readonly current_datetime="$(date '+%Y%m%d_%H%M%S')"
readonly out_dir=./results/"${current_datetime}"

# use nohup to continue to run when ssh is disconnected

nohup sudo \
    AFL_I_DONT_CARE_ABOUT_MISSING_CRASHES=1 \
    AFL_SKIP_CPUFREQ=1 \
    ./AFLplusplus/afl-fuzz -i ./inp -o "${out_dir}" -t 30000 -- ./fuzznetlink &

# save pid for kill.sh

readonly afl_pid="$!"
echo ${afl_pid} > afl_pid.txt