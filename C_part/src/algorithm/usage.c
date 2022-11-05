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

    for(int i = options.thresholds - 1; i >= 0; --i) { //put larger shapes at the top so that they appear underneath
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

        int middle_width = (int)floor((float)map->map_width / (float)2); //int divisions return 0 by default. use float division
        int middle_height = (int)floor((float)map->map_height / (float)2);

        LOG_INFO("creating quadrants");
        Quadrant quadrant1 = {"top-left", map, &options, POSITIVE, POSITIVE};
        quadrant1.bounds.startingX = 0;
        quadrant1.bounds.startingY = 0;
        quadrant1.bounds.endingX = middle_width;
        quadrant1.bounds.endingY = middle_height;

        chunkmap* map2 = generate_chunkmap(map->input, options);
        Quadrant quadrant2 = {"top-right", map2, &options, NEGATIVE, POSITIVE};
        quadrant2.bounds.startingX = middle_width;
        quadrant2.bounds.startingY = 0;
        quadrant2.bounds.endingX = map->map_width;
        quadrant2.bounds.endingY = middle_height; 

        chunkmap* map3 = generate_chunkmap(map->input, options);
        Quadrant quadrant3 = {"bottom-left", map3, &options, POSITIVE, NEGATIVE};
        quadrant3.bounds.startingX = 0;
        quadrant3.bounds.startingY = middle_height;
        quadrant3.bounds.endingX = middle_width;
        quadrant3.bounds.endingY = map->map_height;

        chunkmap* map4 = generate_chunkmap(map->input, options);
        Quadrant quadrant4 = {"bottom-right", map4, &options, NEGATIVE, NEGATIVE};
        quadrant4.bounds.startingX = middle_width;
        quadrant4.bounds.startingY = middle_height;
        quadrant4.bounds.endingX = map->map_width;
        quadrant4.bounds.endingY = map->map_height;

        LOG_INFO("creating threads");
        pthread_t thread1;
        pthread_t thread2;
        pthread_t thread3;
        pthread_t thread4;
        LOG_INFO("filling chunkmap");
        
        pthread_create(&thread1, NULL, process_in_thread, &quadrant1);
        pthread_create(&thread2, NULL, process_in_thread, &quadrant2);
        pthread_create(&thread3, NULL, process_in_thread, &quadrant3);
        pthread_create(&thread4, NULL, process_in_thread, &quadrant4);
        LOG_INFO("waiting for thread1");
        pthread_join(thread1, NULL);
        LOG_INFO("waiting for thread2");
        pthread_join(thread2, NULL);
        LOG_INFO("waiting for thread3");
        pthread_join(thread3, NULL);
        LOG_INFO("waiting for thread4");
        pthread_join(thread4, NULL);

        int code = getLastError();

        if (isBadError())
        {
            LOG_ERR("a thread encountered an error.");
            finish_svg_file(output);
            free_chunkmap(map);
            free_thresholds_array(thresholds);
            return;
        }
        
        map3->shape_list->next = map4->first_shape;
        map4->first_shape->previous = map3->shape_list;
        map3->shape_count += map4->shape_count;

        map2->shape_list->next = map3->first_shape;
        map3->first_shape->previous = map2->shape_list;
        map2->shape_count += map3->shape_count;

        map->shape_list->next = map2->first_shape;
        map2->first_shape->previous = map->shape_list;
        map->shape_count += map2->shape_count;

        windback_lists(map);
        
        write_svg_file(output, map, options);

        if(isBadError()) {
            LOG_ERR("write_svg_file failed with code: %d", getLastError());
            finish_svg_file(output);
            free_chunkmap(map);
            free_thresholds_array(thresholds);
            return;
        }
        free_chunkmap(map);
        map2->shape_list = NULL;
        map3->shape_list = NULL;
        map4->shape_list = NULL;
        free_chunkmap(map2);
        free_chunkmap(map3);
        free_chunkmap(map4);
    }
    finish_svg_file(output);
    free_thresholds_array(thresholds);
    LOG_INFO("vectorization complete");
}
