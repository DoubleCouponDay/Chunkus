#include "jpegfile.h"

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <turbojpeg.h>

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

	if((fifthlast == 'j' && fourthlast == 'p' && thirdlast == 'e' && secondlast == 'g') ||
		(fourthlast == 'j' && thirdlast == 'p' && secondlast == 'g'))
		return true;

	return false;
}

image convert_jpeg_to_image(char* fileaddress) {
	LOG_INFO("opening jpeg file...");

	FILE* file = openfile(fileaddress);

	if(isBadError()) {
		LOG_ERROR("error opening jpeg file");
		return (image){0, 0, NULL};
	}

	int sizeTest1 = fseek(file, 0, SEEK_END);
	int sizeTest2 = ftell(file);
	int sizeTest3 = fseek(file, 0, SEEK_SET);

	if(sizeTest1 < 0 || sizeTest2 < 0 || sizeTest3 < 0) {
		LOG_ERROR("error reading jpeg file");
		setError(READ_FILE_ERROR);
		return (image){0, 0, NULL};
	}

	tjhandle handle = tdInitDecompress();

	char* possibleError = tjGetErrorStr2(handle);

	if(possibleError != NULL) {
		LOG_ERROR("error in tdInitDecompress(): %s", possibleError);
		return (image){0, 0, NULL};
	}

	char* tjAlloc();

	tjDecompress2(
		handle,
		const unsigned char * 	jpegBuf,
		unsigned long 	jpegSize,
		unsigned char * 	dstBuf,
		int 	width,
		int 	pitch,
		int 	height,
		int 	pixelFormat,
		int 	flags  	handle,
		const unsigned char * 	jpegBuf,
		unsigned long 	jpegSize,
		unsigned char * 	dstBuf,
		int 	width,
		int 	pitch,
		int 	height,
		int 	pixelFormat,
		int 	flags 
	);

	char* possibleError = tjGetErrorStr2(handle);

	if(possibleError != NULL) {
		LOG_ERROR("error in tdDecompress2(): %s", possibleError);
		return (image){0, 0, NULL};
	}

	tjDestroy(handle);
	LOG_INFO("jpeg file opened");
}