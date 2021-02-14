
#include <nanosvg.h>
#include <stdio.h>
#include <stdlib.h>

#include "../../test/debug.h"
#include "../utility/error.h";

const char* TEMPLATE_PATH = "template.svg";

///nanosvg copypaste
int NSVG_RGB(int r, int g, int b) {
    return ((unsigned int)r) | ((unsigned int)g << 8) | ((unsigned int)b << 16);
}

void free_template(char* data) {
	free(data);
}

char* format_template(char* template, int width, int height) {
	int extra_len = strlen(template) + 40;
	char *modified_template = calloc(extra_len, sizeof(char));
	snprintf(modified_template, extra_len, template, (float)width, (float)height, (float)width, (float)height);

	free_template(template);
	return modified_template;
}

///nanosvg copypaste
char* gettemplate(int width, int height) {
    FILE* fp = NULL;
	size_t size;
	char* data = NULL;
	fp = fopen(TEMPLATE_PATH, "rb");

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
	DEBUG("reading the template of size: %d \n", size);
	size_t readsize = fread(data, 1, size, fp);
	DEBUG("comparing size of reads\n");

	if (readsize != size) {
        DEBUG("something went wrong reading the file data. \n");
		setError(READ_FILE_ERROR);
        return NULL;
    };
	DEBUG("null terminating the data\n");
	data[size] = '\0';	// Must be null terminated.
	fclose(fp);
	char* formatted = format_template(data, width, height);
	return formatted;
}
