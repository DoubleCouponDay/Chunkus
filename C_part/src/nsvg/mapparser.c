#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <nanosvg.h>
#include <limits.h>

#include "../image.h"
#include "../chunkmap.h"
#include "../utility/logger.h"
#include "../utility/error.h"
#include "copy.h"
#include "mapping.h"
#include "../sort.h"

const int NONE_FILLED = -1;

//builds nsvg paths with a minimum of two boundary chunks
void add_to_path(pixelchunk* chunk, NSVGpath* path, image input) {

    //fill first point
    if(path->pts[0] == NONE_FILLED) { // 1st point of path
        path->pts[0] = chunk->border_location.x; //x1
        path->pts[1] = chunk->border_location.y; //y1
        return;
    }
    //fill second point
    else if(path->pts[2] == NONE_FILLED) {
        path->pts[2] = chunk->border_location.x; //x2
        path->pts[3] = chunk->border_location.y; //y2
    }

    // If both points have been filled, create a new path between them
    int x = chunk->location.x;
    int y = chunk->location.y;
    
    vector2 previous_coord = {
        path->pts[0],
        path->pts[1],
    };

    NSVGpath* nextsegment = create_path(
        input, 
        previous_coord,
        chunk->border_location
    );
    int code = getLastError();

    if(isBadError()) {
        LOG_ERR("create_path failed with code: %n", code);
        return;
    }
    path->next = nextsegment;
    return;
}

// Adds the final segment of the path that links that last path to the first
void close_path(chunkmap* map, NSVGimage* output, NSVGpath* firstpath) {
    vector2 realstart = {
        output->shapes->paths->pts[2],
        output->shapes->paths->pts[3],
    };

    vector2 realend = {
        firstpath->pts[0],
        firstpath->pts[1],
    };
    NSVGpath* path = create_path(map->input, realstart, realend);
    int code = getLastError();
    
    if(isBadError()) {
        LOG_ERR("create_path failed with code: %d", code);
        return;
    }
    output->shapes->paths->next = path;
}

void throw_on_max(int subject) {
    if(subject == INT_MAX) {
        LOG_INFO("long is way too big!");
        setError(OVERFLOW_ERROR);
    }
}

void parse_map_into_nsvgimage(chunkmap* map, NSVGimage* output)
{
    LOG_INFO("checking if shapelist is null");
    //create the svg
    if(map->shape_list == NULL) {
        LOG_ERR("no shapes given to mapparser");
        setError(ASSUMPTION_WRONG);
        return;
    }

    LOG_INFO("creating first shape");
    const char* firstid = "firstshape";
    NSVGshape* firstshape = create_shape(map, firstid, strlen(firstid));

    LOG_INFO("iterating shapes list");
    //iterate shapes
    int index = 0;
    while(map->shape_list != NULL) {

        if(map->shape_list->boundaries->chunk_p == NULL) {
            LOG_ERR("boundary found without any chunks!");
            setError(LOW_BOUNDARIES_CREATED);
            return;
        }
        
        // Select which nsvg shape to use (first or new)
        if(output->shapes == NULL) {
            LOG_INFO("using first shape");
            output->shapes = firstshape;
        }

        else {
            LOG_INFO("creating new shape");
            char shape_number_char = ((char)index) + '0';
            NSVGshape* newshape = create_shape(map, &shape_number_char, 1);

            output->shapes->next = newshape;
            output->shapes = newshape;
        }

        // We have a shape, now we need to iterate its chunks
        vector2 empty = {NONE_FILLED, NONE_FILLED};
        // Create and store the first path for winding back
        NSVGpath* firstpath = create_path(map->input, empty, empty);

        int code = getLastError();

        if(isBadError()) {
            LOG_ERR("create_path failed with code: %d", code);
            return;
        }

        // Case 1: normal amount of boundaries:
        if (map->shape_list->boundaries_length > 1)
        {
            LOG_INFO("iterating boundaries, count: %d ", map->shape_list->boundaries_length);
            int iterationCount = 0;
            //this loop must iterate at least twice
            
            for (pixelchunk_list* current = map->shape_list->boundaries->first; current != NULL; current = current->next)
            {
                add_to_path(current->chunk_p, firstpath, map->input);
                ++iterationCount;
                code = getLastError();

                if(isBadError()) {
                    LOG_ERR("iterate_new_path failed with code: %d", code);
                    return;
                }         
            }
            LOG_INFO("add_to_path ran: %d times", iterationCount);

            if(firstpath->pts[2] == NONE_FILLED) { //didnt form at least one path between two coordinates
                LOG_ERR("NO PATHS FOUND");
                setError(ASSUMPTION_WRONG);
                return;
            }


            if(output->shapes->paths == NULL) {
                output->shapes->paths = firstpath;
            }

            else {
                output->shapes->paths->next = firstpath;
            }

            LOG_INFO("closing path");
            close_path(map, output, firstpath); //requires that paths is set before running

            //set the colour of the shape
            NSVGpaint fill = {
                NSVG_PAINT_COLOR,
                NSVG_RGB(
                    map->shape_list->colour.r, 
                    map->shape_list->colour.g, 
                    map->shape_list->colour.b
                )
            };
            output->shapes->fill = fill;

            NSVGpaint stroke = {
                NSVG_PAINT_NONE,
                NSVG_RGB(0, 0, 0)
            };
            output->shapes->stroke = stroke;

            throw_on_max(index);
            code = getLastError();

            if(isBadError()) {
                LOG_ERR("throw_on_max failed with code: %d", code);
                return;
            }
        }
        //boundaries with less than 1 item accounted for in algorithm.make_triangle() 
        ++index;
        map->shape_list = map->shape_list->next;
    }

    // Done iterating, clean up now
    map->shape_list = map->first_shape;
    LOG_INFO("Iterated %d shapes", map->shape_count);

    if(firstshape->paths != NULL) {
        output->shapes = firstshape;
    }

    else {
        LOG_INFO("not giving any paths to nsvgimage since no paths found");
    }    
}
