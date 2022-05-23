#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "entrypoint.h"
#include "nsvg/usage.h"
#include "utility/logger.h"
#include "utility/error.h"
#include "imagefile/pngfile.h"
#include "imagefile/svg.h"
#include "simplify.h"
#include "imagefile/converter.h"

const char *format1_p = "png";
const char *format2_p = "jpeg";
const int DEFAULT_CHUNKSIZE = 1;
const int DEFAULT_THRESHOLD = 1;
const int DEFAULT_COLOURS = 256;

typedef NSVGimage* (*algorithm)(image, vectorize_options);
typedef void (*algorithm_debug)(image, vectorize_options, char*,char*);
algorithm target_algorithm = dcdfill_for_nsvg;

int execute_program(vectorize_options options) {
	image img = convert_file_to_image(options.file_path);

	if (isBadError())
	{
		LOG_ERR("convert_file_to_image failed with: %d", getLastError());
		return getAndResetErrorCode();
	}

	NSVGimage* nsvg = target_algorithm(img, options);
	int code = getLastError();

	if(isBadError() || nsvg == NULL) {
		free_image_contents(img);
		free_nsvg(nsvg);
		LOG_ERR("vectorize_image failed with code: %d", code);
		return getAndResetErrorCode();
	}
	bool result = write_svg_file(nsvg, OUTPUT_PATH);
	code = getLastError();

	if(result == false || isBadError()) {
		free_image_contents(img);
		free_nsvg(nsvg);
		LOG_ERR("write_svg_file failed with code: %d", code);
		return getAndResetErrorCode();
	}

	free_image_contents(img);
	free_nsvg(nsvg);
	return getAndResetErrorCode();
}

//PUBLIC FACING
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

	// Grag output file path
	char* output_file_p;

	// If no output path given use default one
	if (argc > 2)
		output_file_p = "output.svg";
	else
		output_file_p = argv[2];

	int chunk_size = DEFAULT_CHUNKSIZE;

	if (argc > 3)
		chunk_size = atoi(argv[3]);

	if (chunk_size < 1)
		chunk_size = DEFAULT_CHUNKSIZE;

	float threshold = DEFAULT_THRESHOLD;

	if (argc > 4)
		threshold = (float)atof(argv[4]);	
		
	if (threshold < 0.f)
		threshold = 1;

	int num_colours = DEFAULT_COLOURS;

	if(argc > 5)
		num_colours = (int)atoi(argv[5]);

	if (num_colours > DEFAULT_COLOURS)
		num_colours = DEFAULT_COLOURS;

	if (num_colours < 1)
		num_colours = 1;

	// Halt execution if either path is bad
	if (input_file_path == NULL || output_file_p == NULL)
	{
		LOG_ERR("Empty input or output file");
		return SUCCESS_CODE;
	}

	LOG_INFO("Vectorizing with input: '%s' output: '%s' chunk size: '%d' threshold: '%f', colours: %d", input_file_path, output_file_p, chunk_size, threshold, num_colours);

	vectorize_options options = {
		input_file_path,
		chunk_size,
		threshold,
		num_colours
	};

	return execute_program(options);
}

//PUBLIC FACING
int just_crash() {
	clear_logfile();
	LOG_ERR("crashing this plane; with no survivors");
	void* crash = (void*)1;
	free(crash);
	return 0;
}

gui_images* gui_vectorize(vectorizer_data input)
{
	image input_img = convert_png_to_image(input.filename);

	if (input_img.pixels_array_2d == NULL)
	{
		free_image_contents(input_img);
		return NULL;
	}

	vectorize_options options = {
		input.filename,
		input.chunk_size,
		input.threshold,
		256
	};
	gui_images* gui_output = NULL; // eventually set it to target_algorithm(input_img, options); when the algorithms can output gui_images linked lists

	if (isBadError() || gui_output == NULL)
	{
		free_gui_images(gui_output);
		free_image_contents(input_img);
		return NULL;
	}

	if (isBadError())
	{
		free_gui_images(gui_output);
		free_image_contents(input_img);
		return NULL;
	}

	return gui_output;
}

void free_gui_images(gui_images* input) {
    if(!input) {
        LOG_INFO("input is null");
        return;
    }

    while(input->next != NULL) {
		gui_images* next_image = input->next;
		free_nsvg(input->current);
		free(input);
		input = next_image;
    }
    free(input);
}
