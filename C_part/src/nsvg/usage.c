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
#include "mapparser.h"
#include "dcdfiller.h"
#include "imagefile/pngfile.h"
#include "bobsweep.h"

//entry point of the file
NSVGimage* dcdfill_for_nsvg(image input, vectorize_options options) {
    DEBUG("generating chunkmap\n");
    chunkmap* map = generate_chunkmap(input, options);
    
    if (isBadError())
    {
        DEBUG("generate_chunkmap failed with code: %d \n", getLastError());
        free_chunkmap(map);
        return NULL;
    }

    DEBUG("filling chunkmap\n");
    fill_chunkmap(map, &options);
    
    if (isBadError())
    {
        DEBUG("fill_chunkmap failed with code %d\n", getLastError());
        free_chunkmap(map);
        return NULL;
    }

    DEBUG("sorting boundaries\n");
    sort_boundary(map);

    if(isBadError()) {
        DEBUG("sort_boundary failed with code %d\n", getLastError());
        return NULL;
    }

    DEBUG("printing chunkmap\n");
    write_chunkmap_to_png(map, "chunkmap.png");
    
    if(isBadError()) {
        DEBUG("write_chunkmap_to_png failed with code: %d\n", getLastError());
        free_chunkmap(map);
        return NULL;
    }

    DEBUG("iterating chunk shapes\n");
    NSVGimage* output = create_nsvgimage(map->map_width, map->map_height);
    iterate_chunk_shapes(map, output);
    
    if (isBadError())
    {
        DEBUG("iterate_chunk_shapes failed with code: %d\n", getLastError());
        free_chunkmap(map);
        free_nsvg(output);
        return NULL;
    }
    free_chunkmap(map);
    return output;
}

NSVGimage* bobsweep_for_nsvg(image input, vectorize_options options)
{
    chunkmap* map = generate_chunkmap(input, options);
    NSVGimage* out = sweepfill_chunkmap(map, options.shape_colour_threshhold);

    if (isBadError())
    {
        DEBUG("Failed to Vectorize Image (v2) error: %d", getLastError());
        free_chunkmap(map);
        free_nsvg(out);        
        return NULL;
    }
    free_chunkmap(map);
    return out;
}


void free_nsvg(NSVGimage* input) {
    if(!input) {
        DEBUG("input is null\n");
        return;
    }

    while(input->shapes != NULL) {
        NSVGpath* currentpath = input->shapes->paths;

        while(currentpath != NULL) {
            if (currentpath->pts){
                free(currentpath->pts);
            }
                
            NSVGpath* nextpath = currentpath->next;
            free(currentpath);
            currentpath = nextpath;
        }        
        NSVGshape* nextshape = input->shapes->next;
        free(input->shapes);
        input->shapes = nextshape;
    }
    free(input);
}
