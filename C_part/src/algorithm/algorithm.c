#include "algorithm.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <nanosvg.h>

#include "../image.h"
#include "../chunkmap.h"
#include "../utility/logger.h"
#include "../utility/error.h"
#include "copy.h"
#include "../sort.h"
#include "../utility/vec.h"

const float ZIP_DISTANCE = 0.5;

float get_border_zip_offset(float dimension) {
    if (dimension > 0.f)
        return ZIP_DISTANCE;
        
    else if (dimension < 0.f)
        return -ZIP_DISTANCE;
    
    return 0.f;
}

void zip_seam(pixelchunk* chunk_to_zip, pixelchunk* adjacent) {
    if(adjacent != NULL) { //chunk could be on the edge of the image
        vector2 diff = create_vector_between_chunks(chunk_to_zip, adjacent);
        float offset_x = get_border_zip_offset(diff.x);
        float offset_y = get_border_zip_offset(diff.y);
        chunk_to_zip->border_location.x = chunk_to_zip->border_location.x + offset_x;
        chunk_to_zip->border_location.y = chunk_to_zip->border_location.y + offset_y;
    }
}

void process_layer(Layer* layer) {
    LOG_INFO("Filling layer: %d", layer->layer_index);
    long count = 0;
    int tenth_count = 0;
    int tenth_of_map = (int)floor(layer->map->map_width * layer->map->map_height / 10.f);
     
    for(int map_y = 0; map_y < layer->map->map_height; ++map_y)
    {
        for(int map_x = 0; map_x < layer->map->map_width; ++map_x)
        {
            ++count;

            if (tenth_of_map > 0 && count % tenth_of_map == 0)
            {
                ++tenth_count;
                LOG_INFO("layer: %d progress: %d0%%", layer->layer_index, tenth_count);
            }
            pixelchunk* currentchunk_p = &(layer->map->groups_array_2d[map_x][map_y]);

            if(layer->options->step_index > 0 && count >= layer->options->step_index) {
                LOG_INFO("layer: %d, step_index reached: %d\n", layer->layer_index, count);
                return;
            }
        }
    }
}
