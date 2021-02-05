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

bool iterate_hashies(const void* item, void* udata) {
    long* coordcount = udata;
    pixelchunk* chunk = item;
    *coordcount += chunk->location.x_unit_length;
    *coordcount += chunk->location.y_unit_length;
}

long get_total_count(chunkmap* map, long templatelength, int shapecount, int pathcount) {
    long coordcount = 0;
    chunkshape* currentshape = map->shape_list;

    while(currentshape != NULL) {
        hashmap_scan(currentshape->chunks, iterate_hashies, &coordcount);
        currentshape = currentshape->next;
    }

        /**
     * the string must start with strlen(template)
        
        1 new line is 2 char / 4 char
        
        each nsvgshape has <path fill="rgb(xxx,xxx,xxx)" d="z" />\n 
            41 char
        
        each nsvgpath has 6 char minimum. 4 required chars plus variable 2 chars
            L x y
    **/
    unsigned long totalsize = templatelength + NEW_LINE_LENGTH + (SHAPE_SIZE * shapecount) + (SMALLEST_PATH_SIZE * (pathcount - 1)) + coordcount;
    return totalsize;
}

bool write_svg_file(NSVGimage* input, chunkmap* map) {
    DEBUG("create a file for read/write and destroy contents if already exists");
    FILE* output = fopen(OUTPUT_PATH, "w+"); 

    DEBUG("open the template as a string");
    char* template = gettemplate();

    DEBUG("iterate to the location of the first closing bracket >");
    unsigned long i = 0;
    unsigned long templatelength = strlen(template);

    while(template[i] != ">") {
        ++i;

        if(i >= templatelength || template[i] == NULL) {
            DEBUG("something wrong with the svg template\n");
            setError(ASSUMPTION_WRONG);
            return false;
        }
    }

    DEBUG("calculate the length of the output string");
    long total_chars = get_total_count(map, templatelength, map->shapecount, map->pathcount);

    DEBUG("dynamically allocate a string with this length");
    size_t memoryneeded = sizeof(char) * (total_chars + 1);
    char* filetext = calloc(1, memoryneeded);

    DEBUG("include null at the end of the string");
    unsigned long outputlength = strlen(output);
    filetext[outputlength - 1] = NULL;

    DEBUG("copy the template into the output string");
    strcpy(output, template);
    strcat(output, NEW_LINE);

    DEBUG("iterating nsvgshapes");
    NSVGshape* currentshape = input->shapes;

    while(currentshape != NULL) {
        bool ran_once = false;
        NSVGpath* currentpath = currentshape->paths;

        DEBUG("creating <path> element\n");
        strcat(output, "<path fill=\"#");
        DEBUG("set the fill attribute to the shapes fill property\n");
        int colour = currentshape->fill.color;
        strcat(output, colour);
        strcat(output, "\" d=\"");
        DEBUG("iterating nsvgpaths");

        bool ranonce = true;

        while(currentpath != NULL) {
            if(ranonce == true) {
                DEBUG("start with M moveto command\n");
                strcat(output, "M ");
            }

            else {
                strcat(output, " L ");
            }            
            DEBUG("add a new coordinate to the d property\n");

            DEBUG("each coordinate starts with L followed by x and y space separated values\n");

            DEBUG("finish the d with Z\n");

            DEBUG("close the path element\n");
            currentpath = currentpath->next;
            ranonce = true;
        }
        currentshape = currentshape->next;
    }

    DEBUG("freeing template");
    free_template(template);

    DEBUG("freeing the output string");
    free(output);

    return true;
}
