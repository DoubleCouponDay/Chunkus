#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <nanosvg.h>

#ifdef _WIN32
#include <windows.h>
#include <process.h>
#include <Handleapi.h>
#include <vadefs.h>
#elif __linux__
#include <pthread.h>
#endif

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

void free_layeroperations(LayerOperation* operations_p, int length) {
    LOG_INFO("freeing layeroperations array");
    LayerOperation* next;
    
    for(int i = 0; i < length; ++i) {
        next = &operations_p[i];
        #ifdef _WIN32
        CloseHandle(next->thread);
        #elif __linux__
        free(next->thread);
        #endif
        free(next->layer);
        free(next);
    }
    free(operations_p);
}

void windback_lists(chunkmap* map) {
    map->shape_list = map->first_shape;
    chunkshape* current = map->first_shape;

    while(current != NULL) {
        current->boundaries = current->first_boundary;
        current = current->next;
    }
}

void* process_in_thread(void* arg) {
    Layer* layer = (Layer*)arg;
    process_layer(layer);
    
    if (isBadError())
    {
        LOG_ERR("fill_chunkmap failed with code %d", getLastError());
        #ifdef _WIN32
        _endthreadex(NULL);
        #elif __linux__
        pthread_exit(NULL);
        #endif
    }
    #ifdef _WIN32
    _endthreadex(NULL);
    #elif __linux__
    pthread_exit(NULL);
    #endif
}

LayerOperation* create_layeroperation(chunkmap* map, vectorize_options options) {
    LOG_INFO("creating layeroperation");
    LayerOperation* output = calloc(1, sizeof(LayerOperation));
    #ifdef _WIN32
    output->thread = NULL;
    #elif __linux__
    output->thread = calloc(1, sizeof(pthread_t));
    #endif
    output->layer = calloc(1, sizeof(Layer));
    output->layer->layer_index = options.threshold;
    output->layer->map = map;
    output->layer->options = &options;
    output->layer->boundaries = NULL;
    output->layer->non_boundaries = NULL;
    return output;
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
    FILE* output_file = start_svg_file(map_width, map_height, OUTPUT_PATH);

    if(isBadError()) {
        LOG_ERR("start_svg_file failed with code: %d", getLastError());
        free_thresholds_array(thresholds);
        return;
    }

    quantize_image(&input, options.num_colours);

    if(isBadError()) {
        LOG_ERR("quantize_image failed with %d", getLastError());
        finish_svg_file(output_file);
        free_thresholds_array(thresholds);
        return;
    }
    LOG_INFO("creating layeroperation array");
    LayerOperation* operations_p = calloc(1, sizeof(LayerOperation) * options.thresholds);
    int index = 0;

    for(int i = options.thresholds - 1; i >= 0; --i) { //put larger shapes first so that they appear underneath
        options.threshold = thresholds[i];
        
        LOG_INFO("generating chunkmap");
        chunkmap* map = generate_chunkmap(input, options);
        
        if (isBadError())
        {
            LOG_ERR("generate_chunkmap failed with code: %d", getLastError());
            finish_svg_file(output_file);
            free_chunkmap(map);
            free_thresholds_array(thresholds);
            return;
        }        
        LayerOperation* currentoperation = create_layeroperation(map, options);
        operations_p[index] = *currentoperation;
        ++index;

        LOG_INFO("filling chunkmap");

        #ifdef _WIN32
        currentoperation->thread = _beginthreadex(
            NULL, //handle cannot be inherited by child processes
            0, //stack size
            process_in_thread, //threads start address
            currentoperation->layer, //thread argument
            0, //default init flags; start immediately
            NULL //thread identifier not used
        );
        #elif __linux__
        pthread_create(currentoperation->thread, NULL, process_in_thread, currentoperation->layer);
        #endif

        if(isBadError()) {
            LOG_ERR("write_svg_file failed with code: %d", getLastError());
            finish_svg_file(output_file);
            free_chunkmap(map);
            free_thresholds_array(thresholds);
            return;
        }
    }

    for(int i = 0; i < options.thresholds; ++i) {
        LayerOperation current = operations_p[i];
        #ifdef _WIN32
        WaitForSingleObjectEx(current.thread, INFINITE, false);
        #elif __linux__
        pthread_join(*current.thread, NULL);
        #endif
        windback_lists(current.layer->map);
        write_svg_file(output_file, current.layer->map, options);

        if(isBadError()) {
            LOG_ERR("write_svg_file failed with code: %d", getLastError());
            finish_svg_file(output_file);
            free_chunkmap(current.layer->map);
            free_thresholds_array(thresholds);
            return;
        }
        free_chunkmap(current.layer->map);
        current.layer->map->shape_list = NULL;
    }

    finish_svg_file(output_file);
    free_thresholds_array(thresholds);
    free_layeroperations(operations_p, options.thresholds);
    LOG_INFO("vectorization complete");
}
