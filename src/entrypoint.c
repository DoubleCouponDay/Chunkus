

 #include <stdio.h>
 #include "entrypoint.h"

const char *format1_p = "png";
const char *format2_p = "jpeg";

int entrypoint(int argc, char* argv[]) {
	if (argc <= 1)
	{
		printf("error: No Arguments given");
		return 0;
	}

    char* firstargument_p = argv[1];

	if (!strcmp(firstargument_p, "--help") || !strcmp(firstargument_p, "-h"))
	{
		printf("welcome to the vectorizer.exe help page.\n");
		printf("this program converts a bitmap image into a scalable vector graphic using line tracing techniques.\n");
		printf("supports the png and jpeg formats as input.\n");
		printf("arguments:\n");
		printf("the first argument should be the absolute path to your bitmap.\n");
		printf("the second argument can be -t or --test. it will run the test suite to check if all the characteristics of this program are working\n");
		printf("debug: %s \n", firstargument_p);
		return 1;
	}

	// Not a help message, execute the program

	// Grab input file path
	char* input_file_p = firstargument_p;

	// Grag output file path
	char* output_file_p;

	// If no output path given use default one
	if (argc > 2)
		output_file_p = "output.svg";
	else
		output_file_p = argv[2];

	// Halt execution if either path is bad
	if (input_file_p == NULL || output_file_p == NULL)
	{
		printf("Empty input or output file");
		return 0;
	}

	// Execute program

	return 999;
}
