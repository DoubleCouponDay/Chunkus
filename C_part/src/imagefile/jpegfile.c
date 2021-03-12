#include <stdlib.h>
#include <jpeglib.h>
#include <stdbool.h>
#include <stdio.h>

#include "../image.h"
#include "../utility/error.h"
#include "../utility/logger.h"

free_jsamparray(int width, int height, JSAMPARRAY input) {
	if(!input) {
		return;
	}

	for(int x = 0; x < width; ++x) {
		JSAMPROW current = input[x];

		if(!current) {
			continue;
		}
		free(current);
	}
	free(input);
}

JSAMPARRAY allocate_jsamparray(int output_width, int output_height, int colour_components) {
	JSAMPARRAY array = calloc(output_width, sizeof(JSAMPROW*));
	
	for(int i = 0; i < output_width; ++i) {
		int heightamount = output_height * colour_components; 
		array[i] = calloc(heightamount, sizeof(JSAMPROW)); //unsigned chars 1d
	}
	return array;
}

bool file_is_jpeg(char* fileaddress) {
	char* current = fileaddress[0];
	int index = 0;

	while(current != NULL) {		
		index++;
		current = fileaddress[index];
	}
	char* secondlast = fileaddress[index - 1];
	char* thirdlast = fileaddress[index - 2];
	char* fourthlast = fileaddress[index - 3];
	char* fifthlast = fileaddress[index - 3];

	if(fifthlast == 'j' && fourthlast == 'p' && thirdlast == 'e' && secondlast == 'p' ||
		fourthlast == 'j' && thirdlast == 'p' && secondlast == 'p')
		return true;

	return false;
}

image convert_jpeg_to_image(char* fileaddress) {
    FILE* file_p = openfile(fileaddress);

	if(isBadError()) {
		LOG_ERR("openfile failed");
		return (image){NULL, 0, 0};
	}

    // Allocate and initialize a JPEG decompression object
    struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;	
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);

	// Specify the source of the compressed data (eg, a file)
	jpeg_stdio_src(&cinfo, file_p);

	// Call jpeg_read_header() to obtain image info
	int result = jpeg_read_header(&cinfo, TRUE);

	if(result != JPEG_HEADER_OK) {
		LOG_ERR("jpeg_read_header failed with code: %d", result);
		setError(NOT_PNG_OR_JPEG);
		return (image){NULL, 0, 0};
	}

	// Set parameters for decompression
	// ¯\_(ツ)_/¯

	//start decompression
	bool startedfine = jpeg_start_decompress(&cinfo);

	if(startedfine == FALSE) {
		LOG_ERR("jpeg_start_decompress failed");
		setError(ASSUMPTION_WRONG);
		return (image){NULL, 0, 0};
	}

	if(cinfo.output_components == 1) {
		LOG_ERR("did not expect libjpeg to be quantizing!");
		setError(ASSUMPTION_WRONG);
		return (image){NULL, 0, 0};
	}
	image output = create_image(cinfo.output_width, cinfo.output_height);

	if(cinfo.out_color_space == JCS_GRAYSCALE) {
		LOG_INFO("INPUT IMAGE IS GRAYSCALE");
		output.is_greyscale = true;
	}
	JSAMPARRAY array = allocate_jsamparray(cinfo.output_width, cinfo.output_height, cinfo.output_components);
	jpeg_read_scanlines(&cinfo, array, cinfo.output_height);

	//map JSAMPARRAY to image
	int increment = (output.is_greyscale) ? 1 : 3;

	for(int x = 0; x < cinfo.output_width; ++x) {
		for(int y = 0; y < cinfo.output_height; y += increment) {
			JSAMPLE current = array[x][y];
			output.pixels_array_2d[x][y].r = current;

			if(output.is_greyscale) {
				output.pixels_array_2d[x][y].g = array[x][y + 1];
				output.pixels_array_2d[x][y].b = array[x][y + 2];
			}

			else {
				output.pixels_array_2d[x][y].g = current;
				output.pixels_array_2d[x][y].b = current;
			}	
		}
	}

	free_jsamparray(cinfo.output_width, cinfo.output_height, array);
	bool finishedfine = jpeg_finish_decompress(&cinfo);
	fclose(file_p);

	if(finishedfine == FALSE) {
		LOG_ERR("jpeg_finish_decompress failed");
		setError(ASSUMPTION_WRONG);
		return (image){NULL, 0, 0};
	}

	// Release the JPEG decompression object	
	jpeg_destroy(&cinfo);

	return output;
}
