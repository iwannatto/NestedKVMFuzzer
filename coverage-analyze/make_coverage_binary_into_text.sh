set -eux

coverage_binary='/home/mizutani/NestedKVMFuzzer/linux-kcov/linux-kcov-src/selftest-coverage/vmx_set_nested_state_test.c.bin'
coverage_cov='vmx_set_nested_state_test.cov'
python3 make_coverage_binary_into_cov.py "${coverage_binary}" "${coverage_cov}"

vmlinux='../linux-kcov/linux-kcov-src/vmlinux'
coverage_text='vmx_set_nested_state_test.txt'
addr2line --exe="${vmlinux}" --functions < "${coverage_cov}" > "${coverage_text}"
