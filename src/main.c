// -*- includes needed generally -*-
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// -*- includes needed only for main.c -*-
#include <args.h>
#include <errors.h>
#include <miscount.h>

// NOTE: this only works with MSYS. for MSVC, install a getopt port from vcpkg
#ifdef __WIN32
#include <getopt.h>
#endif

// -*- miscallaneous defines -*-
#define MISCOUNT_VERSION "1.0.1-legacy"

int main(int argc, char **argv) {
	if (argc < 2) {
		fprintf(stderr, "Usage: miscount -m <Miscount> -n <Name> -d <Short Description>\n");
		fprintf(stderr, "You lack options.\n");
		exit(EXIT_NO_ARGS);
	}

	Args *args = malloc(sizeof(Args*) + (sizeof(char*) * 4));
	if (args == NULL) {
		fprintf(stderr, "Cannot initialize program: %s\n", strerror(errno));
		exit(EXIT_MEM_ERROR);
	}

	FurtherOptions *furtheroptions = malloc(sizeof(FurtherOptions*));
	if (furtheroptions == NULL) {
		fprintf(stderr, "Cannot initialize program: %s\n", strerror(errno));
		exit(EXIT_MEM_ERROR);
	}

	int c;
  	while ((c = getopt(argc, argv, "m:n:d:ve")) != -1) {
    	switch (c) {
    		case 'm':
      			args->nameOfMiscount = strdup(optarg);
      			break;
    		case 'n':
      			args->nameOfOffender = strdup(optarg);
      			break;
    		case 'd':
      			args->descriptionOfMiscount = strdup(optarg);
      			furtheroptions->writeDescriptionInEditor = false;

      			break;

    		case 'v':
	      		printf("%s\n", MISCOUNT_VERSION);

		      	free(args);
		      	free(furtheroptions);
		      	return 0;

		     case 'e':
		     	furtheroptions->writeDescriptionInEditor = true;
        		break;
    	}
  	}

  	MiscountParams *miscountParams = malloc((sizeof(Args*)) + (sizeof(FurtherOptions*)));
  	if (miscountParams == NULL) return -1;

  	miscountParams->a = args;
  	miscountParams->b = furtheroptions;

  	miscount_init();
  	if (miscount_append_miscount(miscountParams) != 0) return 1;

  	free(args);
  	free(furtheroptions);
  	free(miscountParams);

  	return 0;
}