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
#include "algorithm.h"
#include "../imagefile/pngfile.h"
#include "../utility/logger.h"
#include "../simplify.h"
#include "../imagefile/svg.h"
#include "thresholds.h"

const char* OUTPUT_PNG_PATH = "output.png";

///warning: winding back lists before threads have finished is bad
void windback_lists(chunkmap* map) {
    map->shape_list = map->first_shape;
    chunkshape* current = map->first_shape;

    while(current != NULL) {
        current->chunks = current->first_chunk;
        current->boundaries = current->first_boundary;
        current = current->next;
    }
}

void* process_in_thread(void* arg) {
    Quadrant* quadrant = (Quadrant*)arg;
    fill_quadrant(quadrant);
    
    if (isBadError())
    {
        LOG_ERR("fill_chunkmap failed with code %d", getLastError());
        pthread_exit(NULL);
    }
    pthread_exit(NULL);
}

void vectorize(image input, vectorize_options options) {
    LOG_INFO("quantizing image to %d colours", options.num_colours);
    LOG_INFO("thresholds: %d", options.thresholds);
    float* thresholds = get_thresholds(options.thresholds);

    if(isBadError()) {
        LOG_ERR("get_thresholds failed with code: %d", getLastError());
        free_thresholds_array(thresholds);
        return;
    }
    int map_width = get_map_width(input, options);
    int map_height = get_map_height(input, options);
    FILE* output = start_svg_file(map_width, map_height, OUTPUT_PATH);

    if(isBadError()) {
        LOG_ERR("start_svg_file failed with code: %d", getLastError());
        free_thresholds_array(thresholds);
        return;
    }

    quantize_image(&input, options.num_colours);

    if(isBadError()) {
        LOG_ERR("quantize_image failed with %d", getLastError());
        finish_svg_file(output);
        free_thresholds_array(thresholds);
        return;
    }
    LayerOperation operations[options.thresholds] = {};
    int index = 0;

    for(int i = options.thresholds - 1; i >= 0; --i) { //put larger shapes first so that they appear underneath
        options.threshold = thresholds[i];
        
        LOG_INFO("generating chunkmap");
        chunkmap* map = generate_chunkmap(input, options);
        
        if (isBadError())
        {
            LOG_ERR("generate_chunkmap failed with code: %d", getLastError());
            finish_svg_file(output);
            free_chunkmap(map);
            free_thresholds_array(thresholds);
            return;
        }

        LOG_INFO("creating map");
        chunkmap* mapCopy = generate_chunkmap(map->input, options);
        Layer layer = {options.thresold, mapCopy, &options};
        LOG_INFO("creating thread");
        pthread_t currentThread;
        
        operations[index] = {
            currentThread, layer
        };
        LOG_INFO("filling chunkmap");
        pthread_create(&currentThread, NULL, process_in_thread, &quadrant1);

        if(isBadError()) {
            LOG_ERR("write_svg_file failed with code: %d", getLastError());
            finish_svg_file(output);
            free_chunkmap(map);
            free_thresholds_array(thresholds);
            return;
        }
    }

    int code = getLastError();

    if (isBadError())
    {
        LOG_ERR("a thread encountered an error.");
        finish_svg_file(output);
        free_chunkmap(map);
        free_thresholds_array(thresholds);
        return;
    }

    for(int i = 0; i < options.thresholds; ++i) {
        LayerOperation current = operations[i];
        pthread_join(current->thread, NULL);
        windback_lists(current.layer->map);
        write_svg_file(output, map, options);

        if(isBadError()) {
            LOG_ERR("write_svg_file failed with code: %d", getLastError());
            finish_svg_file(output);
            free_chunkmap(map);
            free_thresholds_array(thresholds);
            return;
        }
        free_chunkmap(current->layer->map);
        current->layer->map->shape_list = NULL;
    }

    finish_svg_file(output);
    free_thresholds_array(thresholds);
    LOG_INFO("vectorization complete");
}
