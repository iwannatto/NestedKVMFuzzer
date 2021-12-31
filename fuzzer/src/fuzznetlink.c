#include <fcntl.h>
#include <getopt.h>
#include <linux/kvm.h>
#include <linux/netlink.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/shm.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "common.h"

#ifdef AFL
__AFL_COVERAGE();
#endif

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
		fprintf(debugf, "afl_areta_ptr == NULL\n");
		fflush(debugf);
		exit(EXIT_FAILURE);
	}

	return afl_area_ptr;
}

void write_input_to_uefi_image(char *buf)
{
	FILE *f = fopen("./VMXbench/image/input.bin", "w");
	if (fwrite(buf, sizeof(char), 4096, f) != 4096) {
		fprintf(debugf, "write_input_to_image_file_failed\n");
		fflush(debugf);
		exit(EXIT_FAILURE);
	}
}

int main(int argc, char **argv)
{
#ifdef AFL
	__AFL_COVERAGE_OFF();
#endif

	// open log file
#ifdef AFL
	debugf = fopen("./debugf_fuzznetlink.txt", "a");
#else
	debugf = stdout;
#endif
	if (debugf == NULL)
		exit(EXIT_FAILURE);
	fprintf(debugf, "debugf is opened\n");
	fflush(debugf);

	if (argc < 2) {
		fprintf(debugf, "error: argc < 2\n");
		fflush(debugf);
		exit(EXIT_FAILURE);
	}

#ifdef AFL
	uint8_t *afl_area_ptr = get_afl_area_ptr();
#endif

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
	memset(buf, 0, 4096);
	int buf_len = read(0, buf, sizeof(buf));
	if (buf_len < 0) {
		PFATAL("read(stdin)");
	}
	if (buf_len < 5) {
		buf_len = 5;
	}

	write_input_to_uefi_image(buf);

	// qemuをforkしてexecするコード

	pid_t pid = fork();
	if (pid < 0) {
		fprintf(debugf, "fork failed\n");
		exit(EXIT_FAILURE);
	} else if (pid == 0) {
		fprintf(debugf, "exec start\n");
		fflush(debugf);
		execl("./qemu/build/x86_64-softmmu/qemu-system-x86_64",
		      "./qemu/build/x86_64-softmmu/qemu-system-x86_64",
		      "-nodefaults", "-machine", "accel=kvm", "-cpu", "host",
			  "-smp", "1",
		      "-m", "128", "-bios", "./VMXbench/OVMF.fd", "-hda",
		      "json:{ \"fat-type\": 0, \"dir\": \"./VMXbench/image\", "
		      "\"driver\": "
		      "\"vvfat\", \"floppy\": false, \"rw\": true }",
		      "-nographic", "-serial", "mon:stdio", "-no-reboot", NULL);
		fprintf(debugf, "exec failed\n");
		fflush(debugf);
		exit(EXIT_FAILURE);
	}
	int status;
	pid_t r = waitpid(pid, &status, 0);
	if (r < 0) {
		fprintf(debugf, "waitpid failed");
		fflush(debugf);
		exit(EXIT_FAILURE);
	}
	if (WIFEXITED(status)) {
		fprintf(debugf, "child exit-code=%d\n", WEXITSTATUS(status));
		fflush(debugf);
	} else {
		fprintf(debugf, "child status=%04x\n", status);
		fflush(debugf);
	}

#ifndef AFL

	// in standalone mode, coverage file is raw coverage file
	// the program simply rename it to argv[1]/current_time.bin
	char coverage_log_filename[200];
	char *coverage_log_dirname = argv[1];
	unsigned long long current_time = (unsigned long long)time(NULL);
	sprintf(coverage_log_filename, "%s/%llu.bin", coverage_log_dirname, current_time);

	if (rename("/home/mizutani/NestedKVMFuzzer/fuzzer/standalone_coverage.bin", coverage_log_filename) == -1) {
		fprintf(debugf, "rename failed\n");
		exit(EXIT_FAILURE);
	}

	return 0;

#else

	FILE *fuzzing_bitmap = fopen("/home/mizutani/NestedKVMFuzzer/fuzzer/fuzzing_bitmap.bin", "rb");
	if (fuzzing_bitmap == NULL) {
		fprintf(debugf, "fopen failed\n");
		fflush(debugf);
		exit(EXIT_FAILURE);
	}
	int bitmap_size = 65536;
	if (fread(afl_area_ptr, 1, bitmap_size, fuzzing_bitmap) != (size_t)bitmap_size) {
		fprintf(debugf, "fread failed\n");
		fflush(debugf);
		exit(EXIT_FAILURE);
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

#endif

}
