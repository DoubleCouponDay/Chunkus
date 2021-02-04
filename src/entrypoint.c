

 #include <stdio.h>
 #include "entrypoint.h"
 #include "imagefile.h"
 #include "svg/svg.h"
 #include "../test/tools.h"
 #include "error.h"

 #include <stdlib.h>

const char *format1_p = "png";
const char *format2_p = "jpeg";

int execute_program(char* input_file_p, int chunk_size, float threshold, char* output_file_p) {
	image img = convert_png_to_image(input_file_p);

	vectorize_options options = {
		input_file_p,
		chunk_size,
		threshold
	};

	chunkmap* map = generate_chunkmap(img, options);
	int code = getLastError();

	if(code != SUCCESS_CODE) {
		DEBUG("generate_chunkmap failed with code: %d\n", code);
		free_image_contents(img);
		free_group_map(map);
		return getAndResetErrorCode();
	}

	fill_chunkmap(map, &options);
	code = getLastError();

	if(code != SUCCESS_CODE) {
		free_image_contents(img);
		free_group_map(map);
		DEBUG("fill_chunkmap failed with code: %d\n", code);
		return getAndResetErrorCode();
	}

	wind_back_chunkshapes(&map->shape_list);
	code = getLastError();

	if(code != SUCCESS_CODE) {
		free_image_contents(img);
		free_group_map(map);
		DEBUG("wind_back_chunkshapes failed with code: %d\n", code);
		return getAndResetErrorCode();
	}

	write_chunkmap_to_png(map, output_file_p);
	code = getLastError();
	
	if(code != SUCCESS_CODE) {
		free_image_contents(img);
		free_group_map(map);
		DEBUG("write_chunkmap_to_file failed with code: %d\n", code);
		return getAndResetErrorCode();
	}

	write_image_to_png(img, "yo gotem.png");
	code = getLastError();

	if(code != SUCCESS_CODE) {
		free_image_contents(img);
		free_group_map(map);
		DEBUG("write_image_to_png_file failed with code: %d\n", code);
		return getAndResetErrorCode();
	}

	free_image_contents(img);
	free_group_map(map);

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

	if (!strcmp(firstargument_p, "--help") || !strcmp(firstargument_p, "-h"))
	{
		printf("welcome to the vectorizer.exe help page.\n");
		printf("this program converts a bitmap image into a scalable vector graphic using line tracing techniques.\n");
		printf("supports the png and jpeg formats as input.\n");
		printf("arguments:\n");
		printf("the first argument should be the absolute path to your bitmap.\n");
		printf("the second argument can be -t or --test. it will run the test suite to check if all the characteristics of this program are working\n");
		printf("debug: %s \n", firstargument_p);
		return SUCCESS_CODE;
	}

	// Not a help message, execute the program

	// Grab input file path
	char* input_file_p = firstargument_p;

	// Grag output file path
	char* output_file_p;

	// If no output path given use default one
	if (argc > 2)
		output_file_p = "output.png";
	else
		output_file_p = argv[2];

	int chunk_size = 0;
	if (argc > 3)
		chunk_size = atoi(argv[3]);

	if (chunk_size < 1)
		chunk_size = 4;

	float threshold = 0.f;
	if (argc > 4)
		threshold = (float)atof(argv[4]);
	
	printf("atof-ed threshold=%f", threshold);
	
	if (threshold < 0.f)
		threshold = 0.f;

	// Halt execution if either path is bad
	if (input_file_p == NULL || output_file_p == NULL)
	{
		printf("Empty input or output file");
		return SUCCESS_CODE;
	}

	printf("Vectorizing with input: '%s' output: '%s' chunk size: '%d' threshold: '%f' \n", input_file_p, output_file_p, chunk_size, threshold);

	return execute_program(input_file_p, chunk_size, threshold, output_file_p);
}
