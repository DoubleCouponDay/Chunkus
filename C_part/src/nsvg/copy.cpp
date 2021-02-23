#include "copy.h"

#include <string.h>
#include <nanosvg.h>
#include <stdio.h>
#include <stdlib.h>

#include "utility/error.h";
#include "utility/logger.h"

const char* TEMPLATE_PATH = "template.svg";

///nanosvg copypaste
int NSVG_RGB(int r, int g, int b) {
    return ((unsigned int)r << 16) | ((unsigned int)g << 8) | ((unsigned int)b);
}

void free_template(char* data) {
	free(data);
}

char* format_template(char* t, int width, int height) {
	int extra_len = strlen(t) + 40;
	char *modified_t = reinterpret_cast<char*>(calloc(extra_len, sizeof(char)));
	snprintf(modified_t, extra_len, t, (float)width, (float)height, (float)width, (float)height);

	free_template(t);
	return modified_t;
}

///nanosvg copypaste
char* gettemplate(int width, int height) {
    FILE* fp = NULL;
	size_t size;
	char* data = NULL;
	fp = fopen(TEMPLATE_PATH, "rb");

	if (!fp) {
        LOG_INFO("could not find svg t file.");
        setError(TEMPLATE_FILE_NOT_FOUND);
		return NULL;
    };
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	data = (char*)calloc(1, size+1);

	if (data == NULL) {
        LOG_INFO("something went wrong allocating svg space. ");
		setError(SVG_SPACE_ERROR);
        return NULL;
    }
	size_t readsize = fread(data, 1, size, fp);

	if (readsize != size) {
        LOG_INFO("something went wrong reading the file data. ");
		setError(READ_FILE_ERROR);
        return NULL;
    };
	data[size] = '\0';	// Must be null terminated.
	fclose(fp);
	char* formatted = format_template(data, width, height);
	return formatted;
}
