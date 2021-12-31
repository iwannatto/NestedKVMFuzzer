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
rm -f debugf_qemu.txt

readonly current_datetime="$(date '+%Y%m%d_%H%M%S')"
readonly out_dir=./results/"${current_datetime}"
readonly coverage_log_dir="${out_dir}/coverage"

mkdir "${out_dir}"
mkdir "${coverage_log_dir}"

# use nohup to continue to run when ssh is disconnected

# for password
sudo true
sudo \
    AFL_I_DONT_CARE_ABOUT_MISSING_CRASHES=1 \
    AFL_SKIP_CPUFREQ=1 \
    nohup ./AFLplusplus/afl-fuzz -i ./inp -o "${out_dir}" -t 30000 -- ./fuzznetlink "${coverage_log_dir}" &

# save pid for kill.sh

readonly afl_pid="$!"
echo ${afl_pid} > afl_pid.txt