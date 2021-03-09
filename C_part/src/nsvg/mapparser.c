#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <nanosvg.h>

#include "../image.h"
#include "../chunkmap.h"
#include "../utility/logger.h"
#include "../utility/error.h"
#include "copy.h"
#include "mapping.h"
#include "../sort.h"

const int NONE_FILLED = -1;

typedef struct
{
    chunkmap* map;
    NSVGimage* output;
    NSVGpath* first_path;
    NSVGpaint* shapescolour;
} svg_hashies_iter;

//assumes first path and first shape are given
bool iterate_new_path(pixelchunk* chunk, svg_hashies_iter* udata) {
    NSVGshape* current = udata->output->shapes;
    NSVGpath* currentpath = current->paths;
    NSVGpath* nextsegment;

    //add chunk to path if its a boundary
    if(currentpath->pts[0] == NONE_FILLED) { //first point not supplied
        currentpath->pts[0] = chunk->border_location.x; //x1
        currentpath->pts[1] = chunk->border_location.y; //y1

        udata->shapescolour = calloc(1, sizeof(NSVGpaint));
        NSVGpaint* fill = udata->shapescolour;
        fill->type = NSVG_PAINT_COLOR;

        fill->color = NSVG_RGB(
            chunk->average_colour.r, 
            chunk->average_colour.g, 
            chunk->average_colour.b
        );
        return true; //dont use nextsegment before its defined
    }

    else if(currentpath->pts[2] == NONE_FILLED) { //first point supplied but not first path
        currentpath->pts[2] = chunk->border_location.x; //x2
        currentpath->pts[3] = chunk->border_location.y; //y2

        vector2 previous_coord = {
            currentpath->pts[0],
            currentpath->pts[1],
        };

        nextsegment = create_path(
            udata->map->input, 
            previous_coord,
            chunk->border_location
        );
    }

    else { //first path supplied
        int x = chunk->location.x;
        int y = chunk->location.y;
        
        vector2 previous_coord = {
            currentpath->pts[2],
            currentpath->pts[3],
        };

        nextsegment = create_path(
            udata->map->input, 
            previous_coord,
            chunk->border_location
        );
    }
    int code = getLastError();

    if(isBadError()) {
        return false;
    }
    currentpath->next = nextsegment;
    current->paths = nextsegment;
    return true;
}

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

void throw_on_max(unsigned long* subject) {
    if(subject == (unsigned long*)0xffffffff) {
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
    int low_boundary_shapes = 0;
    LOG_INFO("creating first shape");
    char* firstid = "firstshape";
    long firstidlength = 10;
    chunkshape* firstchunkshape = map->shape_list;
    NSVGshape* firstshape = create_shape(map, firstid, firstidlength);
    unsigned long i = 0;
    LOG_INFO("iterating shapes list");

    //iterate shapes
    while(map->shape_list != NULL) {        
        int chunkcount = map->shape_list->chunks_amount;

        if(low_boundary_shapes >= map->shape_count) {
            LOG_ERR("MOST BOUNDARIES NOT BIG ENOUGH");
            setError(LOW_BOUNDARIES_CREATED);
            return;
        }

        else if(map->shape_list->boundaries_length < 2) {
            LOG_INFO("skipping shape with too small boundary");
            ++i;
            ++low_boundary_shapes;
            map->shape_list = map->shape_list->next;
            continue;
        }

        else if(map->shape_list->boundaries->chunk_p == NULL) {
            LOG_ERR("boundary creation broken!");
            setError(LOW_BOUNDARIES_CREATED);
            return;
        }

        if(output->shapes == NULL) {
            LOG_INFO("using first shape");
            output->shapes = firstshape;
        }

        else {
            LOG_INFO("creating new shape");
            char longaschar = i;
            NSVGshape* newshape = create_shape(map, &longaschar, 1);
            output->shapes->next = newshape;
            output->shapes = newshape;
        }
        vector2 empty = {NONE_FILLED, NONE_FILLED};
        NSVGpath* firstpath = create_path(map->input, empty, empty); //lets us wind back the path list
        int code = getLastError();

        if(isBadError()) {
            LOG_ERR("create_path failed with code: %d", code);
            return;
        }
        output->shapes->paths = firstpath; //first shapes path

        svg_hashies_iter shape_data = {
            map, output, firstpath, NULL
        };        

        LOG_INFO("iterating boundaries, count: %d ", map->shape_list->boundaries_length);

        for (pixelchunk_list* iter = map->shape_list->boundaries; iter; iter = iter->next)
        {
            iterate_new_path(iter->chunk_p, &shape_data);
        }
        code = getLastError();

        if(isBadError()) {
            LOG_ERR("iterate_new_path failed with code: %d", code);
            return;
        }

        else if(firstpath->pts[2] == NONE_FILLED) { //didnt form at least one path between two coordinates
            LOG_ERR("NO PATHS FOUND");
            setError(ASSUMPTION_WRONG);
            return;
        }
        LOG_INFO("closing path");
        close_path(map, output, firstpath);
        output->shapes->paths = firstpath; //wind back the paths
        
        //set the colour of the shape while prevent undefined behaviour
        NSVGpaint fillcopy = {
            shape_data.shapescolour->type,
            shape_data.shapescolour->color
        };
        output->shapes->fill = fillcopy;
        free(shape_data.shapescolour); //we held on to the dynamically allocated paint now we free it

        NSVGpaint stroke = {
            NSVG_PAINT_NONE,
            NSVG_RGB(0, 0, 0)
        };
        output->shapes->stroke = stroke;        

        throw_on_max(&i);
        code = getLastError();

        if(isBadError()) {
            LOG_ERR("throw_on_max failed with code: %d", code);
            return;
        }
        ++i;
        map->shape_list = map->shape_list->next; //go to next shape
    }
    map->shape_list = firstchunkshape;
    LOG_INFO("Iterated %d shapes", count_shapes(map->shape_list));

    if(firstshape->paths != NULL) {
        output->shapes = firstshape;
    }

    else {
        LOG_INFO("not giving any paths to nsvgimage since no paths found");
    }    
}
