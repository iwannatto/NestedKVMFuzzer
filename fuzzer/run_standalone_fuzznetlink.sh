set -eux

cd $(dirname $0)

cd AFLplusplus; make source-only; cd ..
make standalone_fuzznetlink
./qemu/build_for_standalone_fuzznetlink.sh
cd VMXbench; make; cd ..

rm -f debugf_fuzznetlink.txt
rm -f debugf_qemu.txt

readonly out_dir='standalone_coverage'
# input_for_standalone_fuzznetlink.bin must be prepared in advance
sudo ./standalone_fuzznetlink "${out_dir}" < input_for_standalone_fuzznetlink.bin