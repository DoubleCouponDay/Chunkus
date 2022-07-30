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
#include "../utility/error.h"
#include "copy.h"
#include "mapping.h"
#include "../sort.h"
#include "mapparser.h"
#include "algorithm.h"
#include "../imagefile/pngfile.h"
#include "../utility/logger.h"


NSVGimage* dcdfill_for_nsvg(image input, vectorize_options options) {
    LOG_INFO("quantizing image to %d colours", options.num_colours);
	quantize_image(&input, options.num_colours);

	if(isBadError()) {
		LOG_ERR("quantize_image failed with %d", getLastError());
		return NULL;
	}

    LOG_INFO("generating chunkmap");
    chunkmap* map = generate_chunkmap(input, options);
    
    if (isBadError())
    {
        LOG_ERR("generate_chunkmap failed with code: %d", getLastError());
        free_chunkmap(map);
        return NULL;
    }

    LOG_INFO("filling chunkmap");
    fill_chunkmap(map, &options);
    
    if (isBadError())
    {
        LOG_ERR("fill_chunkmap failed with code %d", getLastError());
        free_chunkmap(map);
        return NULL;
    }

    LOG_INFO("sorting boundaries");
    sort_boundary(map);

    if(isBadError()) {
        LOG_ERR("sort_boundary failed with code %d", getLastError());
        free_chunkmap(map);
        return NULL;
    }

    LOG_INFO("printing chunkmap");
    write_chunkmap_to_png(map, "chunkmap.png");
    
    if(isBadError()) {
        LOG_INFO("write_chunkmap_to_png failed with code: %d", getLastError());
        free_chunkmap(map);
        return NULL;
    }

    LOG_INFO("iterating chunk shapes");
    NSVGimage* output = create_nsvgimage(map->map_width, map->map_height);
    parse_map_into_nsvgimage(map, output);
    
    if (isBadError())
    {
        LOG_ERR("mapparser failed with code: %d", getLastError());
        free_chunkmap(map);
        free_nsvg(output);
        return NULL;
    }
    free_chunkmap(map);
    return output;
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
