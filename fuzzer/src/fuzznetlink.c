#include <fcntl.h>
#include <getopt.h>
#include <linux/kvm.h>
#include <linux/netlink.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/shm.h>
#include <unistd.h>

#include "common.h"

__AFL_COVERAGE();

FILE *debugf = NULL;

uint8_t *get_afl_area_ptr(void)
{
	// readabilty could be improved
	uint8_t *afl_area_ptr = NULL;
	const char *afl_shm_id_str = getenv("__AFL_SHM_ID");
	if (afl_shm_id_str != NULL) {
		int afl_shm_id = atoi(afl_shm_id_str);
		afl_area_ptr = shmat(afl_shm_id, NULL, 0);
	}

	if (afl_area_ptr == NULL) {
		FILE *debugf = fopen("./debugf.txt", "a");
		fprintf(debugf, "afl_areta_ptr == NULL\n");
		exit(EXIT_FAILURE);
	}

	return afl_area_ptr;
}

int main(int argc, char **argv)
{
	__AFL_COVERAGE_OFF();

	debugf = fopen("./debugf_fuzznetlink.txt", "a");

	uint8_t *afl_area_ptr = get_afl_area_ptr();

	fprintf(debugf, "main start\n");
	fflush(debugf);

	// struct kcov *kcov = NULL;
	// uint64_t *kcov_cover_buf = NULL;
	// kcov = kcov_new();
	// kcov_cover_buf = kcov_cover(kcov);

	/* Read on dmesg /dev/kmsg for crashes. */
	int dmesg_fs = -1;
	dmesg_fs = open("/dev/kmsg", O_RDONLY | O_NONBLOCK);
	if (dmesg_fs < 0) {
		PFATAL("open(/dev/kmsg)");
	}
	lseek(dmesg_fs, 0, SEEK_END);

	/* Load input from AFL (stdin) */
	char buf[512 * 1024];
	memset(buf, 0, 32);
	int buf_len = read(0, buf, sizeof(buf));
	if (buf_len < 0) {
		PFATAL("read(stdin)");
	}
	if (buf_len < 5) {
		buf_len = 5;
	}

	// int kcov_len = 0;

	// /* START coverage collection on the current task. */
	// if (kcov) {
	// 	kcov_enable(kcov);
	// }

	// qemuをforkしてexecするコード

	pid_t pid = fork();
    if (pid < 0) {
        fprintf(debugf, "fork failed\n");
        exit(-1);
    } else if (pid == 0) {
		fprintf(debugf, "exec start\n");
		fflush(debugf);
        execl("./qemu/build/x86_64-softmmu/qemu-system-x86_64", "./qemu/build/x86_64-softmmu/qemu-system-x86_64",
              "-nodefaults", "-machine", "accel=kvm", "-cpu", "host", "-m",
              "128", "-bios", "./VMXbench/OVMF.fd", "-hda",
              "json:{ \"fat-type\": 0, \"dir\": \"./VMXbench/image\", \"driver\": "
              "\"vvfat\", \"floppy\": false, \"rw\": true }",
              "-nographic", "-serial", "mon:stdio", "-no-reboot", NULL);
        fprintf(debugf, "exec failed\n");
        exit(-1);
    }
    int status;
    pid_t r = waitpid(pid, &status, 0);
    if (r < 0) {
        fprintf(debugf, "waitpid failed");
        exit(-1);
    }
    if (WIFEXITED(status)) {
        fprintf(debugf, "child exit-code=%d\n", WEXITSTATUS(status));
    } else {
        fprintf(debugf, "child status=%04x\n", status);
    }

	// /* STOP coverage */
	// if (kcov) {
	// 	kcov_len = kcov_disable(kcov);
	// }

	FILE *coverage_file = fopen("/home/mizutani/NestedKVMFuzzer/fuzzer/coverage.bin", "rb");
	int kcov_len;
	fread(&kcov_len, sizeof(int), 1, coverage_file);
	uint64_t *kcov_cover_buf = malloc(kcov_len * sizeof(uint64_t));
	fread(kcov_cover_buf, sizeof(uint64_t), kcov_len, coverage_file);

	fprintf(debugf, "kcov_len = %d\n", kcov_len);
	fflush(debugf);

	/* Read recorded %rip */
	int i;
	uint64_t afl_prev_loc = 0;
	// srand((unsigned int)time(NULL));
	for (i = 0; i < kcov_len; i++) {
		if (i < 100)
			fprintf(debugf, "kcov_cover_buf[%d + 1] = %lld\n", i, (long long int)kcov_cover_buf[i + 1]);

		uint64_t current_loc = kcov_cover_buf[i + 1];
		// uint64_t current_loc = rand();
		uint64_t hash = hsiphash_static(&current_loc,
						sizeof(unsigned long));
		uint64_t mixed = (hash & 0xffff) ^ afl_prev_loc;
		afl_prev_loc = (hash & 0xffff) >> 1;

		uint8_t *s = &afl_area_ptr[mixed];
		int r = __builtin_add_overflow(*s, 1, s);
		if (r) {
			/* Boxing. AFL is fine with overflows,
				* but we can be better. Drop down to
				* 128 on overflow. */
			*s = 128;
		}
	}

	/* Check dmesg if there was something interesting */
	int crashed = 0;
	while (1) {
		// /dev/kmsg gives us one line per read
		char buf[8192];
		int r = read(dmesg_fs, buf, sizeof(buf) - 1);
		if (r <= 0) {
			break;
		}

		// fprintf(debugf, "%s\n", buf);

		buf[r] = '\x00';
		if (strstr(buf, "Call Trace") != NULL ||
			strstr(buf, "RIP:") != NULL ||
			strstr(buf, "Code:") != NULL) {
			crashed += 1;
		}
	}
	if (crashed) {
		fprintf(debugf, "[!] BUG detected\n");
	}

	// if (kcov) {
	// 	kcov_free(kcov);
	// }
	fprintf(debugf, "main end\n");
	fflush(debugf);
	return 0;
}
