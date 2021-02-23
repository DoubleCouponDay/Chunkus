#include <nanosvg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "../utility/error.h"
#include "../nsvg/copy.h"
#include "../../test/debug.h"
#include "../chunkmap.h"

const char* OUTPUT_PATH = "output.svg";
const SHAPE_SIZE = 41;
const SMALLEST_PATH_SIZE = 4;

#ifdef _WIN32
const char* NEW_LINE = "\r\n";
const int NEW_LINE_LENGTH = 4;
#elif __linux__
const NEW_LINE = "\n";
const int NEW_LINE_LENGTH = 2;
#elif __unix__
const NEW_LINE = "This repo is not for you, Macintosh";
const int NEW_LINE_LENGTH = 0;
#endif

void finish_file(FILE* output, char* template) {
    fprintf(output, "</svg>");

    DEBUG("freeing template\n");
    free_template(template);

    DEBUG("closing file\n");
    fclose(output);
}

bool write_svg_file(NSVGimage* input) {
    DEBUG("create a file for read/write and destroy contents if already exists\n");
    FILE* output = fopen(OUTPUT_PATH, "w+"); 

    DEBUG("open the template as a string\n");
    char* template = gettemplate(input->width, input->height);
    int code = getLastError();

    if(isBadError()) {
        DEBUG("gettemplate failed with code: %d\n", code);
        return false;
    }

    DEBUG("copy the template into the output string\n");
    fprintf(output, template);
    fprintf(output, NEW_LINE);

    if(input->shapes == NULL) {
        DEBUG("no shapes found in nsvg!\n");
        finish_file(output, template);
        return false;
    }

    DEBUG("iterating nsvgshapes\n");
    NSVGshape* currentshape = input->shapes;

    while(currentshape != NULL) {
        NSVGpath* currentpath = currentshape->paths;

        fprintf(output, "<path fill=\"#");
        fprintf(output, "%06X", currentshape->fill.color);
        fprintf(output, "\" d=\"");
        bool ranonce = false;

        while(currentpath != NULL) {
            float x;
            float y;

            if(ranonce == false) {
                fprintf(output, "M ");
                x = currentpath->pts[0];
                y = currentpath->pts[1];
            }

            else {
                fprintf(output, " L ");
                x = currentpath->pts[2];
                y = currentpath->pts[3];
            }
            fprintf(output, "%f ", x);
            fprintf(output, "%f", y);
            currentpath = currentpath->next;
            ranonce = true;
        }
        fprintf(output, " Z\"");
        fprintf(output, "/>\n");
        currentshape = currentshape->next;
    }
    finish_file(output, template);
    return true;
}
