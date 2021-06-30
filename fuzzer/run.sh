# sudo bash -c 'echo core >/proc/sys/kernel/core_pattern'

set -eux

cd $(dirname $0)

cd AFLplusplus; make source-only; cd ..
make fuzznetlink
./qemu/build.sh
cd VMXbench; make; cd ..

rm -f debugf_fuzznetlink.txt

sudo \
    AFL_I_DONT_CARE_ABOUT_MISSING_CRASHES=1 \
    AFL_SKIP_CPUFREQ=1 \
    ./AFLplusplus/afl-fuzz -i ./inp -o ./out -t 20000 -- ./fuzznetlink

# sudo \
#     AFL_I_DONT_CARE_ABOUT_MISSING_CRASHES=1 \
#     AFL_SKIP_CPUFREQ=1 \
#     gdb \
#     --args ./AFLplusplus/afl-fuzz -i ./inp -o ./out -- ./fuzznetlink