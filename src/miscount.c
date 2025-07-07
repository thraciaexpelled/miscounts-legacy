// -*- includes needed generally -*-
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// -*- includes needed only for miscount.c -*-
#include <sys/stat.h>
#include <args.h>
#include <errors.h>
#include <miscount.h>
#include <time.h>

#include "../deps/path-join/path-join.h"

static void exit_if_null(void *any, char *msg, int code) {
	if (any == NULL) {
		fprintf(stderr, msg);
		exit(code);
	}
}

static inline const char *getUserHomeDir() {
	#ifdef __WIN32
  		char homeDir[MAX_PATH];
  		if (GetEnvironmentVariable("USERPROFILE", homeDir, MAX_PATH)) {
    		return homeDir;
  		} else {
    		fprintf(stderr, "miscount: cannot get home directory\n");
    		exit(-1);
  		}
	#else
  		return getenv("HOME");
	#endif
}

// check comment inside the function for more information
int __bmp_tries = 10;
static inline const char *buildMiscountPath() {
	const char *home = getUserHomeDir();

	const char *documents_dir = path_join(home, "Documents");
	exit_if_null(documents_dir, "Do you have your Documents folder in your $HOME?\n", EXIT_PROBABLY_NO_HOME);

	const char *retval = path_join(documents_dir, "miscounts.csv");
	exit_if_null(retval, "Cannot retrieve files\n", EXIT_CANT_RETRIEVE);

	// idiotic way to check optimized out variables but i don't care
	while (strcmp(home, "msicounts.csv") == 0) {
		fprintf(stderr, "Compiler deliquency detected. Trying for %d times.\n", __bmp_tries);
		__bmp_tries -= 1;

		buildMiscountPath();
	}

	return retval;
}

static bool miscountPathExists() {
	struct stat s;
	if (stat(buildMiscountPath(), &s) != 0) return false;
	return true;
}

static int mkMiscountPath() {
	const char *miscountPath = buildMiscountPath();

	FILE *miscountFile = fopen(miscountPath, "w");
	// I/O errors need more than just a bullshit function macro
	if (miscountFile == NULL) {
		fprintf(stderr, "%s: %s\n", miscountPath, strerror(errno));
		exit(EXIT_FILEIO_FAIL);
	}

	fprintf(miscountFile, "DateTime,Miscount,Name,Description\n");
	fclose(miscountFile);

	return 0;
}

static const char *inferGoodEditor() {
	if (getenv("EDITOR") == NULL) {
		#ifdef __WIN32
			fprintf(stderr, "It appears that you're on a Windows-like environment without an $EDITOR variable set.\n");
    		fprintf(stderr, "We'll set the default editor to Notepad for you.\n");

    		if (_putenv("EDITOR=notepad") != 0) {
      			fprintf(stderr, "miscount: cannot add environment variable: %s\n",
              	strerror(errno));
      			exit(-1);
    		}

    		return "notepad";
    	#else
    		return "nano";
    	#endif
	}

	return getenv("EDITOR");
}

// Thanks, chqrlie!
static char *strreplace(char *s, const char *s1, const char *s2) {
    char *p = strstr(s, s1);
    if (p != NULL) {
        size_t len1 = strlen(s1);
        size_t len2 = strlen(s2);
        if (len1 != len2)
            memmove(p + len2, p + len1, strlen(p + len1) + 1);
        memcpy(p, s2, len2);
    }
    return s;
}

static const char *buildCmd(char *cmd, char *args) {
	char *buffer = malloc(sizeof(char*));
	size_t bufsize = sizeof(buffer);

	exit_if_null(buffer, "Cannot build command\n", EXIT_CMD_BUILD_FAILED);

	if (snprintf(buffer, bufsize, "%s %s", cmd, args) >= bufsize) {
		fprintf(stderr, "Output of built command was truncated\n");
		free(buffer);
		exit(EXIT_BUILT_CMD_TRUNCATED);
	}

	return buffer;
}

// ---- End of Static Functions ---- //

void miscount_init() {
	if (!miscountPathExists()) {
		fprintf(stderr, "Creating %s\n", buildMiscountPath());
		mkMiscountPath();
	}
}

int miscount_append_miscount(MiscountParams *m) {
	const char *miscountPath = buildMiscountPath();

	FILE *miscountFile = fopen(miscountPath, "a");
	if (miscountFile == NULL) {
		fprintf(stderr, "%s: %s\n", miscountPath, strerror(errno));
		return EXIT_FILEIO_FAIL;
	}

	// DateTime
	time_t raw_time;
    struct tm *local_time_info;
    char buffer[80]; 

    time(&raw_time);
    local_time_info = localtime(&raw_time);
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", local_time_info);
    fprintf(miscountFile, "%s,", buffer);

	// Miscount
	const char *nom = strdup(m->a->nameOfMiscount);
	fprintf(miscountFile, "%s,", strreplace(nom, ",", " AND/OR "));

	// Name of Offender
	const char *noo = strdup(m->a->nameOfOffender);
	fprintf(miscountFile, "%s,", strreplace(noo, ",", " AND/OR "));

	// Description of Miscount
	const char *miscountDescription = strdup(m->a->descriptionOfMiscount);
	const char *sanitizedMiscountDescription = strreplace(miscountDescription, ",", " AND/OR ");

	if (m->b->writeDescriptionInEditor == false) {
		fprintf(miscountFile, "%s\n", sanitizedMiscountDescription);
	} else {
		// uh oh
		const char *cmd = buildCmd(inferGoodEditor(), ".miscount_tmp");

		if (system(cmd) != 0) return -1;

		// read contents of tempfile and put it into miscounts.csv
		FILE *tmp = fopen(".miscount_tmp", "r");
		if (tmp == NULL) {
			fprintf(stderr, ".miscount_tmp: %s\n", strerror(errno));
			return 1;
		}

		char *tmpbuf = 0;
		long buflen;

		fseek(tmp, 0, SEEK_END);
		buflen = ftell(tmp);
		fseek(tmp, 0, SEEK_SET);

		tmpbuf = malloc(buflen);
		exit_if_null(tmpbuf, "Cannot read temp file\n", EXIT_FILEIO_FAIL);

		fread(tmpbuf, 1, buflen, tmp);
		fclose(tmp);

		fprintf(miscountFile, "%s", tmpbuf);

		if (remove(".miscount_tmp") != 0) {
			fprintf(stderr, ".miscount_tmp might already be gone\n");
		}

		free(tmpbuf);
		free(cmd);
	}

	fclose(miscountFile);

	return 0;
}