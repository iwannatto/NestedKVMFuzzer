set -eux

if [[ "$#" -ne 1 ]]; then
    echo "exactly 1 argument (fuzzing_result_dir) is required, but given $#" 1>&2
    exit 1
fi

readonly fuzzing_result_dir="$1"

for input in "${fuzzing_result_dir}/default/queue/id:"*; do
    ./run_standalone_qemu_wrapper.sh "${input}" "${fuzzing_result_dir}/coverage/$(basename ${input}).bin"
done