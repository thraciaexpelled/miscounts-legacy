#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wstrict-prototypes"

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

static void print_help() {
  printf("miscounts-legacy - log miscounts and other events\n");
  printf("\n\n\t-m -> name of miscount\n");
  printf("\t-n -> name of offender\n");
  printf("\t-d -> description of miscount\n");
  printf("\t\tNOTE: description can be written in the system's default text editor instead by replacing the flag with `-e`\n");
  printf("\t-v -> shows program version\n");
  printf("\t-h -> shows this message\n\n");
  printf("* a miscounts.csv file is created in your $HOME/Documents folder if none is detected\n");
  printf("* this program doesn't confirm whether an existing miscounts.csv file is the one created my miscounts-legacy.\n");
  printf("  * this means in a scenario, if you happened to have a miscounts.csv file prior to installing this program, it will just append miscounts to that existing file. Stay Safe.\n");
}

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
	while ((c = getopt(argc, argv, "m:n:d:vhe")) != -1) {
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

			case 'h':
				print_help();

				free(args);
				free(furtheroptions);
				return 0;

			case 'e':
				furtheroptions->writeDescriptionInEditor = true;
				break;

			default:
				free(args);
				free(furtheroptions);
				return EXIT_BAD_ARGS;
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

#pragma clang diagnostic pop
