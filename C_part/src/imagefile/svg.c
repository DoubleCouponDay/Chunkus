#include <nanosvg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "../utility/error.h"
#include "../nsvg/copy.h"
#include "../utility/logger.h"
#include "../chunkmap.h"

const char* OUTPUT_PATH = "output.svg";

#ifdef _WIN32
const char* NEW_LINE = "\r\n";
const int NEW_LINE_LENGTH = 4;
#elif __linux__
const char* NEW_LINE = "\n";
const int NEW_LINE_LENGTH = 2;
#elif __unix__
const NEW_LINE = "This repo is not for you, Macintosh";
const int NEW_LINE_LENGTH = 0;
#endif

void finish_file(FILE* output, char* template) {
    fprintf(output, "</svg>");

    LOG_INFO("freeing template");
    free_template(template);

    LOG_INFO("closing file");
    fclose(output);
}

void print_coordinates(FILE* output, float x, float y) {
    fprintf(output, "%f ", x);
    fprintf(output, "%f", y);
}

bool write_svg_file(chunkmap* map, const char* filename) {
    LOG_INFO("create a file for read/write and destroy contents if already exists");
    FILE* output = fopen(filename, "w+"); 

    LOG_INFO("open the template as a string");
    char* template = gettemplate(map->map_width, map->map_height);
    int code = getLastError();

    if(isBadError()) {
        LOG_ERR("gettemplate failed with code: %d", code);
        return false;
    }

    LOG_INFO("copy the template into the output string");
    fprintf(output, template);
    fprintf(output, NEW_LINE);

    if(map->shape_list == NULL) {
        LOG_ERR("no shapes found!");
        finish_file(output, template);
        return false;
    }

    LOG_INFO("iterating shapes");
    chunkshape* currentshape = map->first_shape;

    while(currentshape != NULL) {
        if(currentshape->boundaries_length < 2) {
            LOG_ERR("current_shape needs at least 2 boundaries!");
            currentshape = currentshape->next;
            return false;
        }
        pixelchunk_list* currentpath = currentshape->boundaries->first;

        int colour = NSVG_RGB(
            currentshape->colour.r,            
            currentshape->colour.g,
            currentshape->colour.b
        );
        fprintf(output, "<path fill=\"#");
        fprintf(output, "%06X", colour);
        fprintf(output, "\" d=\"");
        bool ranonce = false;

        while(currentpath != NULL) {
            if(ranonce == false) {
                fprintf(output, "M ");
            }

            else {
                fprintf(output, " L ");
            }
            print_coordinates(
                output, 
                currentpath->chunk_p->border_location.x,
                currentpath->chunk_p->border_location.y);

            currentpath = currentpath->next;
            ranonce = true;
        }

        if(ranonce) {
            fprintf(output, " L ");

            print_coordinates(
                output, 
                currentshape->boundaries->first->chunk_p->border_location.x,
                currentshape->boundaries->first->chunk_p->border_location.y);
            
        }
        fprintf(output, " Z\"");
        fprintf(output, "/>\n");
        currentshape = currentshape->next;
    }
    LOG_INFO("Iterated %d shapes", map->shape_count);
    finish_file(output, template);
    return true;
}
