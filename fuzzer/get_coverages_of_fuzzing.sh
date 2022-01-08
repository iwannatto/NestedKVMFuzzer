set -eux

readonly fuzzing_result_dir='results/20211231_201627'

for input in "${fuzzing_result_dir}/default/queue/id:"*; do
    ./run_standalone_qemu_wrapper.sh "${input}" "${fuzzing_result_dir}/coverage/$(basename ${input}).bin"
done