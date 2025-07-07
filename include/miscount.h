typedef struct MiscountParams {
	Args *a;
	FurtherOptions *b;
} MiscountParams;

void miscount_init();
int miscount_append_miscount(MiscountParams *m);