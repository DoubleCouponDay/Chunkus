
#include <nanosvg.h>
#include <stdio.h>
#include "../../test/tools.h"
#include "../error.h";

///nanosvg copypaste
int NSVG_RGB(int r, int g, int b) {
    return ((unsigned int)r) | ((unsigned int)g << 8) | ((unsigned int)b << 16);
}

///nanosvg copypaste
char* gettemplate(char* pathtofile) {
    FILE* fp = NULL;
	size_t size;
	char* data = NULL;
	NSVGimage* image = NULL;
	fp = fopen(pathtofile, "rb");

	if (!fp) {
        DEBUG("could not find svg template file.\n");
        setError(TEMPLATE_FILE_NOT_FOUND);
		return NULL;
    };
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	data = (char*)calloc(1, size+1);

	if (data == NULL) {
        DEBUG("something went wrong allocating svg space. \n");
		setError(SVG_SPACE_ERROR);
        return NULL;
    }

	if (fread(data, 1, size, fp) != size) {
        DEBUG("something went wrong reading the file data. \n");
		setError(READ_FILE_ERROR);
        return NULL;
    };
	data[size] = '\0';	// Must be null terminated.
	fclose(fp);
	data;
}

void freetemplate(char* data) {
	free(data);
}
