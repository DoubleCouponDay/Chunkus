#include <stdio.h>
#include <stdlib.h>

#include "entrypoint.h"
#include "nsvg/usage.h"
#include "../test/debug.h"
#include "utility/error.h"
#include "imagefile/pngfile.h"
#include "imagefile/svg.h"
#include "simplify.h"

const char *format1_p = "png";
const char *format2_p = "jpeg";
const int DEFAULT_CHUNKSIZE = 1;
const int DEFAULT_THRESHOLD = 1;
const int DEFAULT_COLOURS = 5;

typedef NSVGimage* (*algorithm)(image, vectorize_options);
typedef void (*algorithm_debug)(image, vectorize_options, char*,char*);
algorithm target_algorithm = dcdfill_for_nsvg;

int execute_program(vectorize_options options) {
	image img = convert_png_to_image(options.file_path);

	if (isBadError())
	{
		DEBUG("convert_png_to_image failed with: %d\n", getLastError());
		return getAndResetErrorCode();
	}

	NSVGimage* nsvg = target_algorithm(img, options);
	int code = getLastError();

	if(isBadError() || nsvg == NULL) {
		free_image_contents(img);
		free_nsvg(nsvg);
		DEBUG("vectorize_image failed with code: %d\n", code);
		return getAndResetErrorCode();
	}
	bool result = write_svg_file(nsvg);
	code = getLastError();

	if(result == false || isBadError()) {
		free_image_contents(img);
		free_nsvg(nsvg);
		DEBUG("write_svg_file failed with code: %d\n", code);
		return getAndResetErrorCode();
	}

	free_image_contents(img);
	free_nsvg(nsvg);
	return getAndResetErrorCode();
}


int entrypoint(int argc, char* argv[]) {
	printf("entrypoint with: ");
	for (int i = 1; i < argc; ++i)
	{
		printf("%s, ", argv[i]);
	}
	printf("\n");

	if (argc <= 1)
	{
		printf("error: No Arguments given");
		return SUCCESS_CODE;
	}

    char* firstargument_p = argv[1];

	// Grab input file path
	char* input_file_path = firstargument_p;

	// Grag output file path
	char* output_file_p;

	// If no output path given use default one
	if (argc > 2)
		output_file_p = "output.svg";
	else
		output_file_p = argv[2];

	int chunk_size = 0;

	if (argc > 3)
		chunk_size = atoi(argv[3]);

	if (chunk_size < 1)
		chunk_size = DEFAULT_CHUNKSIZE;

	float threshold = DEFAULT_THRESHOLD;

	if (argc > 4)
		threshold = (float)atof(argv[4]);	
		
	if (threshold < 0.f)
		threshold = DEFAULT_THRESHOLD;

	int num_colours = DEFAULT_COLOURS;

	if(argc > 5)
		num_colours = (int)atoi(argv[5]);

	if (num_colours < 1)
		num_colours = DEFAULT_COLOURS;

	// Halt execution if either path is bad
	if (input_file_path == NULL || output_file_p == NULL)
	{
		printf("Empty input or output file");
		return SUCCESS_CODE;
	}

	printf("Vectorizing with input: '%s' output: '%s' chunk size: '%d' threshold: '%f' \n", input_file_path, output_file_p, chunk_size, threshold);

	vectorize_options options = {
		input_file_path,
		chunk_size,
		threshold,
		num_colours
	};

	return execute_program(options);
}

int set_algorithm(int algo)
{
	switch (algo)
	{
	case 0:
		target_algorithm = dcdfill_for_nsvg;
		break;
	case 1:
		target_algorithm = bobsweep_for_nsvg;
		break;
	default:
		return BAD_ARGUMENT_ERROR;
	}
	return 0;
}