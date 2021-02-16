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
#include "mapfiller.h"
#include "imagefile/pngfile.h"

//entry point of the file
NSVGimage* vectorize_image(image input, vectorize_options options) {
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

        if (output)
            free_nsvg(output);

        return NULL;
    }
    free_chunkmap(map);
    return output;
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
