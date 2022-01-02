set -eux

cd $(dirname $0)

readonly input_file="${1:-input_for_standalone_qemu_wrapper.bin}"
readonly output_file="${2:-standalone_coverage/standalone_coverage.bin}"

cd AFLplusplus; make source-only; cd ..
make standalone_qemu_wrapper
./qemu/build.sh --standalone
cd VMXbench; make; cd ..

rm -f debugf_qemu_wrapper.txt
rm -f debugf_qemu.txt

sudo ./standalone_qemu_wrapper "${output_file}" < "${input_file}"