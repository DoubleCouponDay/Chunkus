#include "svg.h"

#include <nanosvg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "utility/error.h"
#include "nsvg/copy.h"
#include "utility/logger.h"
#include "chunkmap.h"

const char* OUTPUT_PATH = "output.svg";
const int SHAPE_SIZE = 41;
const int SMALLEST_PATH_SIZE = 4;

#ifdef _WIN32
const char* NEW_LINE = "\r\n";
const int NEW_LINE_LENGTH = 4;
#elif __linux__
const char* NEW_LINE = "\n";
const int NEW_LINE_LENGTH = 2;
#elif __unix__
const char* NEW_LINE = "This repo is not for you, Macintosh";
const int NEW_LINE_LENGTH = 0;
#endif

void finish_file(FILE* output, char* t) {
    fprintf(output, "</svg>");

    LOG_INFO("freeing template");
    free_template(t);

    LOG_INFO("closing file");
    fclose(output);
}

bool write_svg_file(const nsvg_ptr& input) {
    LOG_INFO("create a file for read/write and destroy contents if already exists");
    FILE* output = fopen(OUTPUT_PATH, "w+"); 

    LOG_INFO("open the template as a string");
    char* t = gettemplate(input->width, input->height);
    int code = getLastError();

    if(isBadError()) {
        LOG_ERR("gettemplate failed with code: %d", code);
        return false;
    }

    fprintf(output, t);
    fprintf(output, NEW_LINE);

    if(input->shapes == NULL) {
        LOG_ERR("no shapes found in nsvg!");
        finish_file(output, t);
        return false;
    }

    LOG_INFO("iterating nsvgshapes");
    NSVGshape* currentshape = input->shapes;

    while(currentshape != NULL) {
        NSVGpath* currentpath = currentshape->paths;

        fprintf(output, "<path fill=\"#");
        fprintf(output, "%06X", currentshape->fill.color);
        fprintf(output, "\" d=\"");
        bool ranonce = false;

        while(currentpath != NULL) {
            int x;
            int y;

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
            fprintf(output, "%d ", x);
            fprintf(output, "%d", y);
            currentpath = currentpath->next;
            ranonce = true;
        }
        fprintf(output, " Z\"");
        fprintf(output, "/>\n");
        currentshape = currentshape->next;
    }
    finish_file(output, t);
    return true;
}
