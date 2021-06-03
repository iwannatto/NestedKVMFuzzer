#define PFATAL(x...)                                                           \
	do {                                                                   \
		fprintf(stderr, "[-] SYSTEM ERROR : " x);                      \
		fprintf(stderr, "\n\tLocation : %s(), %s:%u\n", __FUNCTION__,  \
			__FILE__, __LINE__);                                   \
		perror("      OS message ");                                   \
		fprintf(stderr, "\n");                                         \
		exit(EXIT_FAILURE);                                            \
	} while (0)

/* siphash.c */
uint32_t hsiphash_static(const void *src, unsigned long src_sz);

/* kcov.c */
struct kcov;
struct kcov *kcov_new(void);
void kcov_enable(struct kcov *kcov);
int kcov_disable(struct kcov *kcov);
void kcov_free(struct kcov *kcov);
uint64_t *kcov_cover(struct kcov *kcov);
