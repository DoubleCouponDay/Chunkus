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
#include "./splitting.h"
#include "./aggregating.h"
#include "./pathfinding.h"

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
    splits* splits = split_chunks(layer->map, layer->options->threshold);
    bool** aggregate = OR_64(splits, layer->map->map_width, layer->map->map_height);
    pathfind_shapes(layer, layer->map, aggregate);
    free_splits(splits);
    free_aggregate(aggregate, layer->map->map_width);
}
