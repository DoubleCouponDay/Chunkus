#include "jpegfile.h"

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <turbojpeg.h>

#include "../image.h"
#include "../utility/error.h"
#include "../utility/logger.h"
#include "../utility/defines.h"

bool file_is_jpeg(const char* fileaddress) {
	char current = fileaddress[0];
	int index = 0;

	while((long long)current != (long long)NULL) {		
		index++;
		current = fileaddress[index];
	}
	char secondlast = fileaddress[index - 1];
	char thirdlast = fileaddress[index - 2];
	char fourthlast = fileaddress[index - 3];
	char fifthlast = fileaddress[index - 4];

	if((fifthlast == 'j' && fourthlast == 'p' && thirdlast == 'e' && secondlast == 'g') ||
		(fourthlast == 'j' && thirdlast == 'p' && secondlast == 'g'))
		return true;

	return false;
}

image convert_jpeg_to_image(const char* fileaddress) {
	LOG_INFO("opening jpeg file...");

	FILE* file = openfile(fileaddress);

	if(isBadError()) {
		LOG_ERR("error opening jpeg file");
		return EMPTY_IMAGE;
	}
	LOG_INFO("setting filestream seek position...");
	int sizeTest1 = fseek(file, 0, SEEK_END);
	int sizeTest2 = ftell(file);
	int sizeTest3 = fseek(file, 0, SEEK_SET);

	if(sizeTest1 < 0 || sizeTest2 < 0 || sizeTest3 < 0) {
		LOG_ERR("error setting jpeg file stream position");
		setError(READ_FILE_ERROR);
		return EMPTY_IMAGE;
	}
	LOG_INFO("allocating jpegBuffer...");
	unsigned long jpegSize = (unsigned long)sizeTest2;
	unsigned char* jpegBuffer = (unsigned char*)tjAlloc(jpegSize);

	if(jpegBuffer == NULL) {
		LOG_ERR("error allocating jpeg buffer");
		setError(ASSUMPTION_WRONG);
		return EMPTY_IMAGE;
	}
	LOG_INFO("reading file into buffer...");
	size_t tryRead = fread(jpegBuffer, jpegSize, 1, file);

	if(tryRead < 1) {
		LOG_ERR("error reading jpeg file");
		setError(READ_FILE_ERROR);
		return EMPTY_IMAGE;
	}
	fclose(file);
	file = NULL;
	LOG_INFO("initializing jpeg decompression");

	int width = NOT_FILLED;
	int height = NOT_FILLED;
	int chrominance = NOT_FILLED;
	int colorspace = NOT_FILLED;
	tjhandle handle = tjInitDecompress();

	if(handle == NULL) {
		LOG_ERR("error initializing jpeg decompressor");
		setError(ASSUMPTION_WRONG);
		return EMPTY_IMAGE;
	}
	LOG_INFO("decompressing jpeg header...");

	int decompression = tjDecompressHeader3(
		handle, 
		jpegBuffer,
		jpegSize,
		&width,
		&height,
		&chrominance,
		&colorspace);

	char* possibleError = tjGetErrorStr2(handle);

	if(strcmp(possibleError, "No error") != 0 ||
		decompression < 0) {
		LOG_ERR("error in tjDecompressHeader3: %s", possibleError);
		return EMPTY_IMAGE;
	}

	if(width == NOT_FILLED || height == NOT_FILLED) {
		LOG_ERR("error in tjDecompressHeader3: width or height not filled");
		return EMPTY_IMAGE;
	}

	LOG_INFO("getting scaling factors...");
	int numScalingFactors = NOT_FILLED;
	tjscalingfactor* scalingFactors = tjGetScalingFactors(&numScalingFactors);

	if(scalingFactors == NULL ||
		numScalingFactors == NOT_FILLED) {
		LOG_ERR("error getting jpeg scaling factors");
		return EMPTY_IMAGE;
	}
	LOG_INFO("decompressing jpeg content...");
	tjscalingfactor scalingFactor = scalingFactors[8]; //{1, 1}
	float scaledWidth = TJSCALED(width, scalingFactor);
	float scaledHeight = TJSCALED(height, scalingFactor);
	int pixelFormat = TJPF_RGB;
	int pitch = tjPixelSize[pixelFormat];
	int flags = TJFLAG_ACCURATEDCT; //use the most accurate IDCT algorithm

	char* scaledBuffer = (unsigned char*)tjAlloc(scaledWidth * scaledHeight * pitch);

	if(scaledBuffer == NULL) {
		LOG_ERR("error allocating jpeg scaled buffer");
		setError(ASSUMPTION_WRONG);
		return EMPTY_IMAGE;
	}

	tjDecompress2(
		handle,
		jpegBuffer,
		jpegSize,
		scaledBuffer,
		scaledWidth,
		0,
		scaledHeight,
		pixelFormat,
		flags);

	char* possibleError2 = tjGetErrorStr2(handle);

	if(strcmp(possibleError2, "No error") != 0) {
		LOG_ERR("error in tjDecompress2: %s", possibleError2);
		return EMPTY_IMAGE;
	}
	LOG_INFO("converting buffer to image...");
	image output = create_image(scaledWidth, scaledHeight);

	for(int y = 0; y < output.height; ++y) {
		for(int x = 0; x < output.width; ++x) {
			int index = (y * output.width + x) * 3;
			int scaledR = scaledBuffer[index];
			int scaledG = scaledBuffer[index + 1];
			int scaledB = scaledBuffer[index + 2];
			output.pixels_array_2d[x][y].r = scaledR;
			output.pixels_array_2d[x][y].g = scaledG;
			output.pixels_array_2d[x][y].b = scaledB;
		}
	}

	LOG_INFO("cleaning up conversion...");
	tjFree(jpegBuffer);
	tjFree(scaledBuffer);
	tjDestroy(handle);
	handle = NULL;
	LOG_INFO("jpeg file converted to image");
	return output;
}