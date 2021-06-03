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

int main(int argc, char **argv)
{
	__AFL_COVERAGE_OFF();

	FILE *debugf = fopen("./debugf.txt", "a");

	uint32_t child_pid = getpid() + 1;

	struct forksrv *forksrv = forksrv_new();
	fprintf(debugf, "hongu\n");
	forksrv_welcome(forksrv); // ここで止まっているらしい
	fprintf(debugf, "taiga\n");
	uint8_t *afl_area_ptr = forksrv_area_ptr(forksrv);


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

	/* Convince AFL we started a child. */
	forksrv_cycle(forksrv, child_pid);

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
		forksrv_status(forksrv, 139);
	} else {
		forksrv_status(forksrv, 0);
	}

	forksrv_free(forksrv);
	if (kcov) {
		kcov_free(kcov);
	}
	return 0;
}
