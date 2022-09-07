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
#include "../simplify.h"
#include "../imagefile/svg.h"

const char* OUTPUT_PNG_PATH = "output.png";

void vectorize(image input, vectorize_options options) {
    LOG_INFO("quantizing image to %d colours", options.num_colours);
	quantize_image(&input, options.num_colours);

	if(isBadError()) {
		LOG_ERR("quantize_image failed with %d", getLastError());
		return;
	}

    LOG_INFO("generating chunkmap");
    chunkmap* map = generate_chunkmap(input, options);
    
    if (isBadError())
    {
        LOG_ERR("generate_chunkmap failed with code: %d", getLastError());
        free_chunkmap(map);
        return;
    }

    LOG_INFO("filling chunkmap");
    fill_chunkmap(map, &options);
    
    if (isBadError())
    {
        LOG_ERR("fill_chunkmap failed with code %d", getLastError());
        free_chunkmap(map);
        return;
    }

    LOG_INFO("sorting boundaries");
    sort_boundary(map);

    if(isBadError()) {
        LOG_ERR("sort_boundary failed with code %d", getLastError());
        free_chunkmap(map);
        return;
    }

    LOG_INFO("printing chunkmap");
    write_chunkmap_to_png(map, "output.png");
    
    if(isBadError()) {
        LOG_INFO("write_chunkmap_to_png failed with code: %d", getLastError());
        free_chunkmap(map);
        return;
    }
     
	write_svg_file(map, OUTPUT_PATH);

	if(isBadError()) {
		free_chunkmap(map);
		LOG_ERR("write_svg_file failed with code: %d", getLastError());
		return;
	}
    free_chunkmap(map);
}
