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
#include <unistd.h>

#include "common.h"

uint8_t *get_afl_area_ptr(void)
{
	uint8_t *afl_area_ptr = NULL;
	const char *afl_shm_id_str = getenv("__AFL_SHM_ID");
	if (afl_shm_id_str != NULL) {
		int afl_shm_id = atoi(afl_shm_id_str);
		// shm_id 0 is fine
		afl_area_ptr = shmat(afl_shm_id, NULL, 0);
	}

	if (afl_area_ptr == NULL) {
		FILE *debugf = fopen("./debugf.txt", "a");
		fprintf(debugf, "afl_areta_ptr == NULL");
		exit(EXIT_FAILURE);
	}

	return afl_area_ptr;
}

int main(int argc, char **argv)
{
	__AFL_COVERAGE_OFF();

	FILE *debugf = fopen("./debugf.txt", "a");

	uint8_t *afl_area_ptr = get_afl_area_ptr();

	struct kcov *kcov = NULL;
	uint64_t *kcov_cover_buf = NULL;
	kcov = kcov_new();
	kcov_cover_buf = kcov_cover(kcov);

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

	int kcov_len = 0;

	/* START coverage collection on the current task. */
	if (kcov) {
		kcov_enable(kcov);
	}

	int kvmfd = open("/dev/kvm", O_RDWR);
	int vmfd = ioctl(kvmfd, KVM_CREATE_VM, 0);

	/* STOP coverage */
	if (kcov) {
		kcov_len = kcov_disable(kcov);
	}

	/* Read recorded %rip */
	int i;
	uint64_t afl_prev_loc = 0;
	for (i = 0; i < kcov_len; i++) {
		uint64_t current_loc = kcov_cover_buf[i + 1];
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

		buf[r] = '\x00';
		if (strstr(buf, "Call Trace") != NULL ||
			strstr(buf, "RIP:") != NULL ||
			strstr(buf, "Code:") != NULL) {
			crashed += 1;
		}
	}
	if (crashed) {
		fprintf(stderr, "[!] BUG detected\n");
	}

	if (kcov) {
		kcov_free(kcov);
	}
	return 0;
}
