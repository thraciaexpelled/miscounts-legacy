#include <stdbool.h>

typedef struct Args {
	char *nameOfMiscount;
	char *nameOfOffender;
	char *descriptionOfMiscount;
} Args;

typedef struct FurtherOptions {
	bool writeDescriptionInEditor;
	bool writeToExistingMiscountFile;
} FurtherOptions;
