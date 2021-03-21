#include <stdlib.h>
#include <jpeglib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "../image.h"
#include "../utility/error.h"
#include "../utility/logger.h"

bool file_is_jpeg(char* fileaddress) {
	char current = fileaddress[0];
	int index = 0;

	while(current != NULL) {		
		index++;
		current = fileaddress[index];
	}
	char secondlast = fileaddress[index - 1];
	char thirdlast = fileaddress[index - 2];
	char fourthlast = fileaddress[index - 3];
	char fifthlast = fileaddress[index - 4];

	if(fifthlast == 'j' && fourthlast == 'p' && thirdlast == 'e' && secondlast == 'g' ||
		fourthlast == 'j' && thirdlast == 'p' && secondlast == 'g')
		return true;

	return false;
}

void add_scanline_to_image(image output, JSAMPROW* row, int input_y, int row_length, int column_height) {
	//place a [y][x] array inside an [x][y] array
	for(int row_i = 0; row_i < row_length; row_i += 3) {
		int r = (int)row[row_i];
		int g = (int)row[row_i + 1];
		int b = (int)row[row_i + 2];
		int actual_x = row_i / 3;
		output.pixels_array_2d[actual_x][input_y].r = r;
		output.pixels_array_2d[actual_x][input_y].g = b; //wtf this actual works
		output.pixels_array_2d[actual_x][input_y].b = g;
	}
}

image convert_jpeg_to_image(char* fileaddress) {
    FILE* file_p = openfile(fileaddress);

	if(isBadError()) {
		LOG_ERR("openfile failed");
		return (image){0, 0, NULL};
	}

    // Allocate and initialize a JPEG decompression object
    struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr err; //the error handler
    cinfo.err = jpeg_std_error( &err ); 
	jpeg_create_decompress(&cinfo);

	// Specify the source of the compressed data (eg, a file)
	jpeg_stdio_src(&cinfo, file_p);

	// Call jpeg_read_header() to obtain image info
	int result = jpeg_read_header(&cinfo, TRUE);

	if(result != JPEG_HEADER_OK) {
		LOG_ERR("jpeg_read_header failed with code: %d", result);
		setError(NOT_PNG_OR_JPEG);
		return (image){0, 0, NULL};
	}

	if(cinfo.num_components == 4) {
		LOG_ERR("RGBA jpeg's not supported!");
		setError(RGBA_UNSUPPORTED);
		return (image){0, 0, NULL};
	}

	else if(cinfo.num_components == 1) {
		LOG_ERR("greyscale jpeg's not supported!");
		setError(GREYSCALE_UNSUPPORTED);
		return (image){0, 0, NULL};
	}

	// Set parameters for decompression
	//cinfo.scale_num = 1;
	//cinfo.scale_denom = 1;
	//cinfo.block_size = 1;
	//cinfo.do_fancy_upsampling = FALSE;

	//start decompression
	bool startedfine = jpeg_start_decompress(&cinfo);

	if(startedfine == FALSE) {
		LOG_ERR("jpeg_start_decompress failed");
		setError(ASSUMPTION_WRONG);
		return (image){0, 0, NULL};
	}
	int row_stride = cinfo.output_width * cinfo.output_components;
	image output = create_image(cinfo.output_width, cinfo.output_height);

	if(cinfo.out_color_space == JCS_GRAYSCALE) {
		LOG_INFO("INPUT IMAGE IS GRAYSCALE");
		output.is_greyscale = true;
	}

	//map JSAMPARRAY to image
	
	/* Make a one-row-high sample array that will go away when done with image */
	//JSAMPARRAY buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);
	JSAMPARRAY buffer = calloc(1, sizeof(JSAMPROW*));
	buffer[0] = calloc(1, sizeof(JSAMPROW) * row_stride);

	/* Here we use the library's state variable cinfo.output_scanline as the
	* loop counter, so that we don't have to keep track ourselves.
	*/
	while (cinfo.output_scanline < cinfo.output_height) {
		/* jpeg_read_scanlines expects an array of pointers to scanlines.
		* Here the array is only one element long, but you could ask for
		* more than one scanline at a time if that's more convenient.
		*/
		int current_y = cinfo.output_scanline;
		jpeg_read_scanlines(&cinfo, buffer, 1);
		/* Assume put_scanline_someplace wants a pointer and sample count. */
		add_scanline_to_image(output, buffer[0], current_y, row_stride, cinfo.output_height);
	}

	// Release the jpeg decompression object	
	free(buffer[0]);
	free(buffer);
	jpeg_finish_decompress(&cinfo);   //finish decompressing
	jpeg_destroy_decompress(&cinfo);
	fclose(file_p);                    //close the file
	return output;
}
