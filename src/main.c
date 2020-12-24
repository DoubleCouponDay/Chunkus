
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <png.h>
#include <nanosvg.h>
#include <jpeglib.h>
#include "entrypoint.h"

#include "vectorize.h"
#include "converter.h"

int main(int argc, char* argv[]) {	
	//entrypoint(argc, argv);
	if (argc < 2)
	{
		printf("Please supply a file path\n");
		return 0;
	}
	
	printf("Using given input file\n");
	char *file = argv[1];
	char *output_file = "bob.bmp";
	if (argc > 2)
	{
		printf("Using given output file\n");
		output_file = argv[2];
	}
	else
		printf("Using default output file\n");

	char *output_map_file = "bob_map.bmp";
	if (argc > 3)
	{
		printf("Using given map output file\n");
		output_map_file = argv[3];
	}
	else
		printf("Using default map output file\n");

	char *output_variance_file = "bob_map_var.bmp";
	if (argc > 4)
	{
		printf("Using given map variance output file\n");
		output_variance_file = argv[4];
	}
	else
		printf("Using default map variance output file\n");
	
	int map_chunk_size = 4;
	if (argc > 5)
	{
		int parsed = strtoimax(argv[5], NULL, 10);
		if (parsed > 0)
		{
			printf("Using given map chunk size\n");
			map_chunk_size = parsed;
		}
		else
		{
			printf("Using default map chunk size\n");
		}
	}
	else
	{
		printf("Using default map chunk size\n");
	}

	printf("Converting image %s...\n", file);
	image my_image = convert_png_to_image(file);
	if (!my_image.pixels)
	{
		fprintf(stderr, "Failed to create image from PNG; exiting\n");
		return 0;
	}

	printf("Generating Node Map\n");
	node_map map = generate_node_map(my_image, (node_map_options){ map_chunk_size });
	if (!map.nodes)
	{
		fprintf(stderr, "Failed to generate node map from image; exiting\n");
		free_image_contents(&my_image);
		return 0;
	}

	printf("Writing image back to disk as %s\n", output_file);
	write_image_to_file(my_image, output_file);
	printf("Writing node map to disk as %s\n", output_map_file);
	write_node_map_to_file(map, output_map_file);
	printf("Writing node variance to file as %s\n", output_variance_file);
	write_node_map_variance_to_file(map, output_variance_file);

	free_image_contents(&my_image);

	return 0;
}