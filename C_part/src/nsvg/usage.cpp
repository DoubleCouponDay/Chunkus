#include "usage.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <nanosvg.h>

#include "../image.h"
#include "../chunkmap.h"
#include "../utility/error.h"
#include "copy.h"
#include "mapping.h"
#include "../sort.h"
#include "mapparser.h"
#include "dcdfiller.h"
#include "imagefile/pngfile.h"
#include "bobsweep.h"
#include "utility/logger.h"

//entry point of the file
nsvg_ptr dcdfill_for_nsvg(const image& input, vectorize_options options) {
    LOG_INFO("DCDFill with threshold: %f, chunk_size: %d", options.shape_colour_threshhold, options.chunk_size);
    chunkmap map{input, options.chunk_size};
    
    fill_chunkmap(map, options);
    LOG_INFO("Chunkmap found %u shapes", map.shape_list.size());
    
    LOG_INFO("Sorting Boundaries...");
    sort_boundary(map);

    if(isBadError()) {
        LOG_ERR("sort_boundary failed with code %d", getLastError());
        return nsvg_ptr(nullptr, free_nsvg);
    }

    map.chunks_to_file("chunkmap chunks.png");
    map.shapes_to_file("chunkmap shapes.png");
    
    if(isBadError()) {
        LOG_ERR("write_chunkmap_to_png failed with code: %d", getLastError());
        return nsvg_ptr(nullptr, free_nsvg);
    }

    LOG_INFO("Now Parsing");
    nsvg_ptr nsvg{ create_nsvgimage(map.width(), map.height()), free_nsvg };
    parse_map_into_nsvgimage(map, nsvg);
    
    if (isBadError())
    {
        LOG_ERR("mapparser failed with code: %d", getLastError());
        return nsvg_ptr(nullptr, free_nsvg);
    }

    return nsvg;
}

nsvg_ptr bobsweep_for_nsvg(const image& input, vectorize_options options) {
    
    chunkmap map{input, options.chunk_size};

    if (isBadError()) {
        LOG_INFO("generate_chunkmap failed with code: %d ", getLastError());
        return nsvg_ptr(nullptr, free_nsvg);
    }
    sweepfill_chunkmap(map, options.shape_colour_threshhold);

    if (isBadError())
    {
        LOG_ERR("bobsweep failed with error: %d", getLastError());
        return nsvg_ptr(nullptr, free_nsvg);
    }
    //sort_boundary(map);

    map.borders_to_file("bobsweep borders.png");

    if(isBadError()) {
        LOG_INFO("sort_boundary failed with code %d", getLastError());
        return nsvg_ptr(nullptr, free_nsvg);
    }
    
    nsvg_ptr nsvg{ create_nsvgimage(map.width(), map.height()), free_nsvg };
    parse_map_into_nsvgimage(map, nsvg);

    if (isBadError())
    {
        LOG_ERR("mapparser failed with error: %d", getLastError());
        return nsvg_ptr(nullptr, free_nsvg);
    }

    return nsvg;
}


void free_nsvg(NSVGimage* input) {
    if(!input) {
        LOG_INFO("input is null");
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
