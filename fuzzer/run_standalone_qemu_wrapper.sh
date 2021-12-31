set -eux

cd $(dirname $0)

cd AFLplusplus; make source-only; cd ..
make standalone_qemu_wrapper
./qemu/build.sh --standalone
cd VMXbench; make; cd ..

rm -f debugf_qemu_wrapper.txt
rm -f debugf_qemu.txt

# input_for_standalone_qemu_wrapper.bin must be prepared in advance
readonly out_dir='standalone_coverage'
sudo ./standalone_qemu_wrapper "${out_dir}" < input_for_standalone_qemu_wrapper.bin