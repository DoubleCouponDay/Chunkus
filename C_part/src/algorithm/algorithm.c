#include "algorithm.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <nanosvg.h>
#include <pthread.h>

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

void make_triangle(Layer* layer, pixelchunk* currentchunk_p) {  
    int top_location_x = currentchunk_p->location.x;
    int top_location_y = currentchunk_p->location.y + 1;

    int right_location_x = currentchunk_p->location.x + 1;
    int right_location_y = currentchunk_p->location.y;

    pixelchunk* top_vertex = &(layer->map->groups_array_2d[top_location_x][top_location_y]);
    pixelchunk* right_vertex = &(layer->map->groups_array_2d[right_location_x][right_location_y]);
    
    if(currentchunk_p->shape_chunk_in->boundaries_length != 1 &&
        currentchunk_p->shape_chunk_in->boundaries_length != 2)  //sadly this can add the same chunk to its own boundary twice
    {
        //only form triangle on unformed chunks        
        return;
    }

    else if(currentchunk_p->boundary_chunk_in == NULL) {
        add_chunk_to_boundary(layer, currentchunk_p->shape_chunk_in, currentchunk_p, true);
    }
    chunkshape* triangle = currentchunk_p->shape_chunk_in;

    if(top_vertex->shape_chunk_in == NULL) {
        top_vertex->shape_chunk_in = triangle;
        add_chunk_to_boundary(layer, triangle, top_vertex, true);
    }

    else {
        add_chunk_to_boundary(layer, triangle, top_vertex, true);
    }

    if(right_vertex->shape_chunk_in == NULL) {
        right_vertex->shape_chunk_in = triangle;
        add_chunk_to_boundary(layer, triangle, right_vertex, true);
    }

    else {
        add_chunk_to_boundary(layer, triangle, right_vertex, true);
    }
    
    if(isBadError()) {
        LOG_ERR("add_chunk_to_boundary failed with code: %d", getLastError());
        return;
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

    if(layer->options->threshold != 0) { //only draw triangles on the top layer
        return;
    }
    LOG_INFO("layer: %d, making triangles", layer->layer_index);

    return;
}
