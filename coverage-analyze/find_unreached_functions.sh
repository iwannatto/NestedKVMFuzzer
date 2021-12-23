set -eux

dir='20211218_181151'
c='/home/mizutani/NestedKVMFuzzer/linux-kcov/linux-kcov-src/arch/x86/kvm/vmx/vmx.c'
ctags='vmx.ctags'
output='vmx.txt'

ctags -x --c-kinds=f "${c}" > "${dir}/${ctags}"

python3 find_unreached_functions.py \
    --ctags "${dir}/${ctags}" --cov_txts "${dir}/common.txt" "${dir}/set1_only.txt" --output "${dir}/${output}"