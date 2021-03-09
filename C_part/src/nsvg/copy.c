
#include <nanosvg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "../utility/logger.h"
#include "../utility/error.h"

const char* TEMPLATE_PATH = "template.svg";

///nanosvg copypaste
int NSVG_RGB(int r, int g, int b) {
    return ((unsigned int)r << 16) | ((unsigned int)g << 8) | ((unsigned int)b);
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
	
	if(fp == NULL && chdir("./target/release/") == 0) { //couldnt find template here and alternative location DOES exist
		fp = fopen(TEMPLATE_PATH, "rb");
	}

	if(fp == NULL) { //still null after trying fix
        LOG_ERR("could not find svg template file.");
        setError(TEMPLATE_FILE_NOT_FOUND);
		return NULL;
    };

	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	data = (char*)calloc(1, size+1);

	if (data == NULL) {
        LOG_ERR("something went wrong allocating svg space.");
		setError(SVG_SPACE_ERROR);
        return NULL;
    }
	LOG_INFO("reading the template of size: %d", size);
	size_t readsize = fread(data, 1, size, fp);
	LOG_INFO("comparing size of reads");

	if (readsize != size) {
        LOG_ERR("something went wrong reading the file data.");
		setError(READ_FILE_ERROR);
        return NULL;
    };
	LOG_INFO("null terminating the data");
	data[size] = '\0';	// Must be null terminated.
	fclose(fp);
	char* formatted = format_template(data, width, height);
	return formatted;
}
