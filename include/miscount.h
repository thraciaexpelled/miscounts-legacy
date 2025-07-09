#define MISCOUNT_VERSION "1.0.2:1-legacy"

#define MISCOUNT_BUILD_CMD_MAX_BUFSIZE 512

typedef struct MiscountParams {
	Args *a;
	FurtherOptions *b;
} MiscountParams;

void miscount_init();
int miscount_append_miscount(MiscountParams *m);