

 #include <stdio.h>
 #include "entrypoint.h"
 #include "imagefile.h"
 #include "svg/svg.h"
 #include "../test/tools.h"

 #include <stdlib.h>

const char *format1_p = "png";
const char *format2_p = "jpeg";

int error_code = 0;

int get_error()
{
	int tmp = error_code;
	error_code = 0;
	return tmp;
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
		return 0;
	}

	printf("Vectorizing with input: '%s' output: '%s' chunk size: '%d' threshold: '%f' \n", input_file_p, output_file_p, chunk_size, threshold);

	// Execute program

	image img = convert_png_to_image(input_file_p);

	vectorize_options options = {
		input_file_p,
		chunk_size,
		threshold
	};

	chunkmap map = generate_chunkmap(img, options);

	fill_chunkmap(&map, &options);

	wind_back_chunkshapes(&map.shape_list);

	write_chunkmap_to_file(map, output_file_p);

	write_image_to_png_file(img, "yo gotem.png");

	free_group_map(&map);
	free_image_contents(img);

	return get_error();
}
