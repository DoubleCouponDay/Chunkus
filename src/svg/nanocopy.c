#include <nanosvg.h>
#include <stdio.h>
#include "../../test/tools.h"

///nanosvg copypaste
int NSVG_RGB(int r, int g, int b) {
    return ((unsigned int)r) | ((unsigned int)g << 8) | ((unsigned int)b << 16);
}

///nanosvg copypaste
NSVGimage* parsetemplate(char* pathtofile) {
    FILE* fp = NULL;
	size_t size;
	char* data = NULL;
	NSVGimage* image = NULL;
	fp = fopen(pathtofile, "rb");

	if (!fp) {
        DEBUG("could not find svg template file.\n");
        int crash[1];
        crash[1];
    };
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	data = (char*)malloc(size+1);

	if (data == NULL) {
        DEBUG("something went wrong allocating svg space. \n");
        int crash[1];
        crash[1];
    }
	if (fread(data, 1, size, fp) != size) {
        DEBUG("something went wrong reading the file data. \n");
        int crash[1];
        crash[1];
    };
	data[size] = '\0';	// Must be null terminated.
	fclose(fp);
	image = calloc(1, sizeof(NSVGimage));
	free(data);

	return image;
}
