#include <nanosvg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "../utility/error.h"
#include "../algorithm/copy.h"
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

void clear_svg_file(const char* filename) {
    LOG_INFO("clearing old output svg file");
    FILE* output = fopen(filename, "w+");
    
    if(output)
        fclose(output);
}

FILE* start_svg_file(int map_width, int map_height, const char* filename) {
    clear_svg_file(filename);
    FILE* output = fopen(filename, "a+"); 

    if(output == NULL) {
        LOG_ERR("failed to open file: %s", filename);
        setError(ASSUMPTION_WRONG);
        return NULL;
    }
    LOG_INFO("opening the template as a string");
    char* template = gettemplate(map_width, map_height);

    if(isBadError()) {
        LOG_ERR("gettemplate failed with code: %d", getLastError());
        setError(ASSUMPTION_WRONG);
        return NULL;
    }
    LOG_INFO("copy the template into the output string");
    fprintf(output, template);
    fprintf(output, NEW_LINE);
    LOG_INFO("freeing template");
    free(template);
    return output;
}

void finish_svg_file(FILE* output) {
    LOG_INFO("finishing svg file");
    fprintf(output, "</svg>");
    fclose(output);
}

void print_coordinates(FILE* output, float x, float y) {
    fprintf(output, "%.2f ", x);
    fprintf(output, "%.2f", y);
}

void write_svg_file(FILE* output, chunkmap* map, vectorize_options options) {
    LOG_INFO("writing svg shapes for threshold: %.2f", options.threshold);
    LOG_INFO("iterating shapes");

    if(map->first_shape == NULL) {
        LOG_ERR("no first_shape found!");
        setError(ASSUMPTION_WRONG);
        return;
    }

    chunkshape* currentshape = map->first_shape;
    long oneBoundaryCount = 0;

    while(currentshape != NULL) {
        if(currentshape->boundaries_length < 2) {
            currentshape = currentshape->next;
            ++oneBoundaryCount;
            continue;
        }
        pixelchunk_list* currentpath = currentshape->first_boundary;

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
                currentshape->first_boundary->chunk_p->border_location.x,
                currentshape->first_boundary->chunk_p->border_location.y);
            
        }
        fprintf(output, " Z\"");
        fprintf(output, "/>\n");
        currentshape = currentshape->next;
    }
    LOG_INFO("wrote %d shapes", map->shape_count);
    LOG_INFO("%d shapes had invalid single chunk boundaries", oneBoundaryCount);
    return;
}
