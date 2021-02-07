#include <nanosvg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "../utility/error.h"
#include "../nsvg/copy.h"
#include "../../test/debug.h"
#include "../chunkmap.h"
#include "../hashmap/usage.h"

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

bool write_svg_file(NSVGimage* input) {
    DEBUG("create a file for read/write and destroy contents if already exists\n");
    FILE* output = fopen(OUTPUT_PATH, "w+"); 

    DEBUG("open the template as a string\n");
    char* template = gettemplate();
    int code = getLastError();

    if(isBadError()) {
        DEBUG("gettemplate failed with code: %d\n", code);
        return false;
    }

    {
        int extra_len = strlen(template) + 40;
        char *modified_template = calloc(extra_len, sizeof(char));
        snprintf(modified_template, extra_len, template, input->width, input->height, input->width, input->height);

        free_template(template);
        template = modified_template;
    }

    DEBUG("copy the template into the output string\n");
    fprintf(output, template);
    fprintf(output, NEW_LINE);

    DEBUG("iterating nsvgshapes\n");
    NSVGshape* currentshape = input->shapes;

    while(currentshape != NULL) {
        bool ran_once = false;
        NSVGpath* currentpath = currentshape->paths;

        DEBUG("creating <path> element\n");
        fprintf(output, "<path fill=\"#");
        unsigned int colour = currentshape->fill.color;
        DEBUG("set the fill attribute to the shapes fill property: %x\n", colour);
        fprintf(output, "%x", colour);
        fprintf(output, "\" d=\"");

        bool ranonce = false;

        while(currentpath != NULL) {
            int x;
            int y;
            if(ranonce == false) {
                DEBUG("start with M moveto command\n");
                fprintf(output, "M ");
                x = currentpath->pts[0];
                y = currentpath->pts[1];
                fprintf(output, "%d ", x);
                fprintf(output, "%d", y);
            }

            else {
                fprintf(output, " L ");
                x = currentpath->pts[2];
                y = currentpath->pts[3];
                fprintf(output, "%d ", x);
                fprintf(output, "%d", y);
            }            
            currentpath = currentpath->next;
            ranonce = true;
        }
        DEBUG("finish the d with Z\n");
        fprintf(output, " Z\"");
        DEBUG("close the path element\n");
        fprintf(output, "/>\n");
        currentshape = currentshape->next;
    }

    fprintf(output, "</svg>");

    DEBUG("freeing template\n");
    free_template(template);

    DEBUG("closing file\n");
    fclose(output);

    return true;
}
