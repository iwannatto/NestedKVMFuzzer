# sudo bash -c 'echo core >/proc/sys/kernel/core_pattern'

set -e

cd $(dirname $0)

cd AFLplusplus; make; cd ..
make fuzznetlink

rm -f debugf.txt

AFL_I_DONT_CARE_ABOUT_MISSING_CRASHES=1 \
    AFL_SKIP_CPUFREQ=1 \
    ./AFLplusplus/afl-fuzz -i ./inp -o ./out -- ./fuzznetlink
