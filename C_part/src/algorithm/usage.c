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
#include "../sort.h"
#include "algorithm.h"
#include "../imagefile/pngfile.h"
#include "../utility/logger.h"
#include "../simplify.h"
#include "../imagefile/svg.h"
#include "thresholds.h"

const char* OUTPUT_PNG_PATH = "output.png";

void vectorize(image input, vectorize_options options) {
    LOG_INFO("quantizing image to %d colours", options.num_colours);
    LOG_INFO("thresholds: %d", options.thresholds);
    vectorize_options* options_ref = &(options);
    float* thresholds = get_thresholds(options.thresholds);
    int map_width = get_map_width(input, options);
    int map_height = get_map_height(input, options);
    FILE* output = start_svg_file(map_width, map_height, OUTPUT_PATH);

    if(isBadError()) {
        LOG_ERR("start_svg_file failed with code: %d", getLastError());
        return;
    }

    quantize_image(&input, options.num_colours);

    if(isBadError()) {
        LOG_ERR("quantize_image failed with %d", getLastError());
        finish_svg_file(output);
        return;
    }

    for(int i = options.thresholds; i >= 0; --i) { //put larger shapes at the top so that they appear underneath
        options_ref->threshold = thresholds[i];
        
        LOG_INFO("generating chunkmap");
        chunkmap* map = generate_chunkmap(input, options);
        
        if (isBadError())
        {
            LOG_ERR("generate_chunkmap failed with code: %d", getLastError());
            finish_svg_file(output);
            free_chunkmap(map);
            return;
        }

        LOG_INFO("filling chunkmap");
        fill_chunkmap(map, &options);
        
        if (isBadError())
        {
            LOG_ERR("fill_chunkmap failed with code %d", getLastError());
            finish_svg_file(output);
            free_chunkmap(map);
            return;
        }

        LOG_INFO("sorting boundaries");
        sort_boundary(map);

        if(isBadError()) {
            LOG_ERR("sort_boundary failed with code %d", getLastError());
            finish_svg_file(output);
            free_chunkmap(map);
            return;
        }
        
        if(isBadError()) {
            LOG_INFO("write_chunkmap_to_png failed with code: %d", getLastError());
            finish_svg_file(output);
            free_chunkmap(map);
            return;
        }
        
        write_svg_file(output, map, options);

        if(isBadError()) {
            free_chunkmap(map);
            LOG_ERR("write_svg_file failed with code: %d", getLastError());
            finish_svg_file(output);
            return;
        }
        free_chunkmap(map);
    }
    finish_svg_file(output);
    free_thresholds_array(thresholds);
}
