#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "entrypoint.h"
#include "algorithm/usage.h"
#include "utility/logger.h"
#include "utility/error.h"
#include "imagefile/pngfile.h"
#include "imagefile/svg.h"
#include "simplify.h"
#include "imagefile/converter.h"
#include "algorithm/thresholds.h"

const char *format1_p = "png";
const char *format2_p = "jpeg";
const int DEFAULT_CHUNKSIZE = 1;
const int DEFAULT_COLOURS = 256;

int execute_program(vectorize_options options) {
	image img = convert_file_to_image(options.file_path);

	if (isBadError())
	{
		LOG_ERR("convert_file_to_image failed with: %d", getLastError());
		return getAndResetErrorCode();
	}

	vectorize(img, options);
	int code = getLastError();
	free_image_contents(img);
	return getAndResetErrorCode();
}

int entrypoint(int argc, char* argv[]) {
	clear_logfile();
	LOG_INFO("entrypoint with: ");

	for (int i = 1; i < argc; ++i)
	{
		LOG_INFO("argument %d: %s, ", i, argv[i]);
	}

	if (argc <= 1)
	{
		LOG_ERR("error: argc indicates no arguments given");
		return SUCCESS_CODE;
	}
    char* firstargument_p = argv[1];

	// Grab input file path
	char* input_file_path = firstargument_p;

	int chunk_size = DEFAULT_CHUNKSIZE;

	if (argc > 3)
		chunk_size = atoi(argv[2]);

	if (chunk_size < 1)
		chunk_size = DEFAULT_CHUNKSIZE;

	int thresholds = DEFAULT_THRESHOLDS;

	if (argc > 4)
		thresholds = (int)atoi(argv[3]);
		
	if (thresholds < 1)
		thresholds = DEFAULT_THRESHOLDS;

	int num_colours = DEFAULT_COLOURS;

	if(argc > 5)
		num_colours = (int)atoi(argv[4]);

	if (num_colours > DEFAULT_COLOURS)
		num_colours = DEFAULT_COLOURS;

	if (num_colours < 1)
		num_colours = 1;

	// Halt execution if either path is bad
	if (input_file_path == NULL)
	{
		LOG_ERR("Empty input or output file");
		return SUCCESS_CODE;
	}

	LOG_INFO("Vectorizing with input: '%s', chunk size: '%d', thresholds: '%.2f', colours: %d", input_file_path, chunk_size, thresholds, num_colours);

	vectorize_options options = {
		input_file_path,
		chunk_size,
		thresholds,
		0,
		num_colours
	};

	return execute_program(options);
}

int just_crash() {
	clear_logfile();
	LOG_ERR("crashing this plane; with no survivors");
	void* crash = (void*)1;
	free(crash);
	return 0;
}
