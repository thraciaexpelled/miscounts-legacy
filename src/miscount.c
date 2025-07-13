#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wincompatible-pointer-types-discards-qualifiers"
#pragma clang diagnostic ignored "-Wstrict-prototypes"

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
#include <stdarg.h>
#include <time.h>

#include "../deps/path-join/path-join.h"


static void exit_if_null(void *any, char *msg, int code) {
	if (any == NULL) {
		fprintf(stderr, "%s\n", msg);
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

static inline bool userDocumentsDirExists() {
  struct stat s;
  if (stat(path_join(getUserHomeDir(), "Documents"), &s) != 0) return false;
  return true;
}

static inline void makeUserDocumentsDir() {
  const char *path = path_join(getUserHomeDir(), "Documents");
  if (mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0) {
	fprintf(stderr, "%s: %s\n", path, strerror(errno));
	exit(EXIT_FILEIO_FAIL);
  }
}

// check comment inside the function for more information
static int __bmp_tries = 10;
static inline const char *buildMiscountPath() {
	const char *home = getUserHomeDir();

	// turns out, stephen's `path-join` package was just a souped up strcat.
	// we have to do more checks.
	
	if (!userDocumentsDirExists()) {
		fprintf(stderr, "User Document Directory does not exist. Cannot continue\n");
		printf("Do you want this program to create a Documents directory in your home for you? [y/N] ");

		int ans = fgetc(stdin);
		
		puts("\n");

		switch (ans) {
		case 'y':
			printf("OK. Re-run your command after we make your directory.\n");
			makeUserDocumentsDir();
			exit(0);
		default: abort();
		}

		abort();
	}

	const char *documents_dir = path_join(home, "Documents");
	// exit_if_null(documents_dir, "Do you have your Documents folder in your $HOME?\n", EXIT_PROBABLY_NO_HOME);
  
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
		/*
			fprintf(stderr, "It appears that you're on a Windows-like environment without an $EDITOR variable set.\n");
			fprintf(stderr, "We'll set the default editor to Notepad for you.\n");

			if (_putenv("EDITOR=notepad") != 0) {
				fprintf(stderr, "miscount: cannot add environment variable: %s\n",
				strerror(errno));
				exit(-1);
			}

			return "notepad";
		*/
			fprintf(stderr, "Not implemeted for Windows yet!\n");
			exit(-1);
		#else
			return "vi";
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
	char *buffer = malloc(MISCOUNT_BUILD_CMD_MAX_BUFSIZE);
	if (buffer < MISCOUNT_BUILD_CMD_MAX_BUFSIZE) {
		fprintf(stderr, "Cannot allocate memory for buffer\n");
		exit(EXIT_MEM_ERROR);
	}

	sprintf(buffer, "%s %s", cmd, args);

	return buffer;
}

// Thanks, Vishwesh Pujari!
static int remove_line_from_file(FILE* fp, int bytes) {
	char byte;
	long readPos = ftell(fp) + bytes, writePos = ftell(fp), startingPos = writePos;
	// start reading from the position which comes after the bytes to be deleted
	fseek(fp, readPos, SEEK_SET);
	while (fread(&byte, sizeof(byte), 1, fp)) {
		// modify readPos as we have read right now
		readPos = ftell(fp);
		// set file position to writePos as we are going to write now
		fseek(fp, writePos, SEEK_SET);
		
		// if file doesn't have write permission
		if (fwrite(&byte, sizeof(byte), 1, fp) == 0) 
			return errno;
		// modify writePos as we have written right now
		writePos = ftell(fp);
		// set file position for reading
		fseek(fp, readPos, SEEK_SET);
	}

	// truncate file size to remove the unnecessary ending bytes
	ftruncate(fileno(fp), writePos);
	// reset file position to the same position that we got when function was called.
	fseek(fp, startingPos, SEEK_SET); 
	return 0;
}

// ---- End of Static Functions ---- //

void miscount_init() {
	printf("miscounts-legacy %s\n", MISCOUNT_VERSION);
	fprintf(stderr, "(C) gushtichudi/thraciaexpelled 2025, All rights reserved\n\n");

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
	if (m->b->writeDescriptionInEditor == false) {
		const char *miscountDescription = strdup(m->a->descriptionOfMiscount);
		const char *sanitizedMiscountDescription = strreplace(miscountDescription, ",", " AND/OR ");
		fprintf(miscountFile, "%s\n", sanitizedMiscountDescription);
	} else {
		// uh oh
		const char *cmd = buildCmd(inferGoodEditor(), ".miscount_tmp");

		if (system(cmd) != 0) return -1;

		// read contents of tempfile and put it into miscounts.csv
		FILE *tmp = fopen(".miscount_tmp", "r");
		if (tmp == NULL) {
			fprintf(stderr, "Cannot open .miscount_tmp: %s\n", strerror(errno));
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
		// free(cmd);
	}

	fclose(miscountFile);

	return 0;
}

#pragma clang diagnostic pop
