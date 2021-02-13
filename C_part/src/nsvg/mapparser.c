#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <nanosvg.h>

#include "usage.h"
#include "../image.h"
#include "../chunkmap.h"
#include "../../test/debug.h"
#include "../utility/error.h"
#include "copy.h"
#include "mapping.h"
#include "../sort.h"

const int NONE_FILLED = -1;


//assumes first path and first shape are given
bool iterate_new_path(pixelchunk* chunk, svg_hashies_iter* udata) {
    NSVGshape* current = udata->output->shapes;
    NSVGpath* currentpath = current->paths;
    NSVGpath* nextsegment;

    //add chunk to path if its a boundary
    if(currentpath->pts[0] == NONE_FILLED) { //first point not supplied
        currentpath->pts[0] = chunk->location.x; //x1
        currentpath->pts[1] = chunk->location.y; //y1

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
        currentpath->pts[2] = chunk->location.x; //x2
        currentpath->pts[3] = chunk->location.y; //y2

        coordinate previous_coord = {
            currentpath->pts[0],
            currentpath->pts[1],
            1, 1
        };
        
        nextsegment = create_path(
            udata->map->input, 
            previous_coord,
            chunk->location
        );
    }

    else { //first path supplied
        int x = chunk->location.x;
        int y = chunk->location.y;
        
        coordinate previous_coord = {
            currentpath->pts[2],
            currentpath->pts[3],
            1, 1
        };

        nextsegment = create_path(
            udata->map->input, 
            previous_coord,
            chunk->location
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
    coordinate realstart = {
        output->shapes->paths->pts[2],
        output->shapes->paths->pts[3],
        1, 1
    };

    coordinate realend = {
        firstpath->pts[0],
        firstpath->pts[1],
        1, 1
    };        
    NSVGpath* path = create_path(map->input, realstart, realend);
    int code = getLastError();
    
    if(isBadError()) {
        DEBUG("create_path failed with code: %d\n", code);
        return;
    }
    output->shapes->paths->next = path;
}

void throw_on_max(unsigned long* subject) {
    if(subject == (unsigned long*)0xffffffff) {
        DEBUG("long is way too big!\n");
        setError(OVERFLOW_ERROR);
    }
}

void iterate_chunk_shapes(chunkmap* map, NSVGimage* output)
{
    DEBUG("checking if shapelist is null\n");
    //create the svg
    if(map->shape_list == NULL) {
        DEBUG("no shapes given to mapparser\n");
        setError(ASSUMPTION_WRONG);
        return;
    }
    int low_boundary_shapes = 0;
    DEBUG("creating first shape\n");
    char* firstid = "firstshape";
    long firstidlength = 10;
    chunkshape* firstchunkshape = map->shape_list;
    NSVGshape* firstshape = create_shape(map, firstid, firstidlength);
    unsigned long i = 0;
    DEBUG("iterating shapes list\n");

    //iterate shapes
    while(map->shape_list != NULL) {        
        DEBUG("iteration: %d \n", i);
        int chunkcount = map->shape_list->chunks_amount;

        if(low_boundary_shapes == map->shape_count - 1) {
            DEBUG("MOST BOUNDARIES NOT BIG ENOUGH\n");
            setError(LOW_BOUNDARIES_CREATED);
            return;
        }

        else if(map->shape_list->boundaries_length < 2) {
            DEBUG("skipping shape with too small boundary\n");
            ++i;
            ++low_boundary_shapes;
            map->shape_list = map->shape_list->next;
            continue;
        }

        else if(map->shape_list->boundaries->chunk_p == NULL) {
            DEBUG("boundary creation broken!\n");
            setError(LOW_BOUNDARIES_CREATED);
            return;
        }

        if(output->shapes == NULL) {
            DEBUG("using first shape\n");
            output->shapes = firstshape;
        }

        else if(chunkcount > 1) { 
            DEBUG("creating new shape\n");
            char longaschar = i;
            NSVGshape* newshape = create_shape(map, &longaschar, 1);
            output->shapes->next = newshape;
            output->shapes = newshape;
        }

        else {
            DEBUG("not enough chunks found in shape!\n");
            ++i;
            map->shape_list = map->shape_list->next;
            continue;
        }
        coordinate empty = {NONE_FILLED, NONE_FILLED, 1, 1};
        NSVGpath* firstpath = create_path(map->input, empty, empty); //lets us wind back the path list
        int code = getLastError();

        if(isBadError()) {
            DEBUG("create_path failed with code: %d\n", code);
            return;
        }
        output->shapes->paths = firstpath; //first shapes path
        DEBUG("creating iter struct\n");

        svg_hashies_iter shape_data = {
            map, output, firstpath, NULL
        };        

        DEBUG("iterating boundaries, count: %d \n", map->shape_list->boundaries_length);

        for (pixelchunk_list* iter = map->shape_list->boundaries; iter; iter = iter->next)
        {
            iterate_new_path(iter->chunk_p, &shape_data);
        }
        code = getLastError();

        if(isBadError()) {
            DEBUG("iterate_new_path failed with code: %d\n", code);
            return;
        }

        else if(firstpath->pts[2] == NONE_FILLED) { //didnt form at least one path between two coordinates
            DEBUG("NO PATHS FOUND\n");
            setError(ASSUMPTION_WRONG);
            return;
        }
        DEBUG("closing path\n");
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
            DEBUG("throw_on_max failed with code: %d\n", code);
            return;
        }
        ++i;
        map->shape_list = map->shape_list->next; //go to next shape
    }
    map->shape_list = firstchunkshape;

    if(firstshape->paths != NULL) {
        output->shapes = firstshape;
    }

    else {
        DEBUG("not giving any paths to nsvgimage since no paths found\n");
    }    
}
