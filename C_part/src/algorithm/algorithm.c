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

/// @brief returns whether the current chunk was added to the boundary or not
/// @param layer 
/// @param shape 
/// @param chunk 
/// @param allow_multiple_shapes 
/// @return 
bool add_chunk_to_boundary(Layer* layer, chunkshape* shape, pixelchunk* chunk, bool allow_multiple_shapes) {
    bool current_sorted = false;
    if(shape == NULL || chunk == NULL) { //sanity check
        LOG_ERR("add_chunk_to_boundary given null pointer!");
        setError(ASSUMPTION_WRONG);
        return current_sorted;
    }
    
    else if(chunk->boundary_chunk_in != NULL && allow_multiple_shapes == false) { //chunk already in boundary or in another shapes boundaries
        return current_sorted;
    }
    if(shape->boundaries == NULL) {
        pixelchunk_list* new = create_boundaryitem(chunk);
        shape->first_boundary = new;
        shape->boundaries = new;
    }
    
    ++shape->boundaries_length;
    if(isBadError()) {
        LOG_ERR("layer: %d, sort_boundary_chunk failed with code: %d", layer->layer_index, getLastError());
        return current_sorted;
    }
    chunk->shape_chunk_in = NULL; //just paranoid that add_chunk_to_shape will fail
    return current_sorted;
}


void find_shapes(
    Layer* layer, 
    pixelchunk* current,
    int map_x, int map_y, 
    float threshold) {

    if(map_x == 0 || map_x == (layer->map->map_width - 1) ||
        map_y == 0 || map_y == (layer->map->map_height - 1)) 
    {

        if(isBadError()) {
            LOG_ERR("layer: %d, enlarge_border failed with code: %d", layer->layer_index, getLastError());
            return;
        }
    }

    for (int adjacent_y = -1; adjacent_y < 2; ++adjacent_y)
    {
        for (int adjacent_x = -1; adjacent_x < 2; ++adjacent_x)
        {
            if (adjacent_x == 0 && adjacent_y == 0)
                continue; //skip center pixel
            
            int adjacent_index_x = map_x + adjacent_x;
            int adjacent_index_y = map_y + adjacent_y;

            //prevent out of bounds index
            if (adjacent_index_x < 0 || 
                adjacent_index_y < 0 ||
                adjacent_index_x >= layer->map->map_width ||  
                adjacent_index_y >= layer->map->map_height)
                continue;

            pixelchunk* adjacent = &(layer->map->groups_array_2d[adjacent_index_x][adjacent_index_y]);

            //make a shape out of two adjacent chunks
            if (colours_are_similar(current->average_colour, adjacent->average_colour, threshold)) {

                if(isBadError()) {
                    LOG_ERR("layer: %d, enlarge_shape failed with code: %d", layer->layer_index, getLastError());
                    return;
                }
            }

            //define a boundary between two chunks
            else {
                
                if(isBadError()) {
                    LOG_ERR("layer: %d, enlarge_border failed with code: %d", layer->layer_index, getLastError());
                    return;
                }
            }
        }
    }
}

void make_triangle(Layer* layer, pixelchunk* currentchunk_p) {  
    int top_location_x = currentchunk_p->location.x;
    int top_location_y = currentchunk_p->location.y + 1;

    int right_location_x = currentchunk_p->location.x + 1;
    int right_location_y = currentchunk_p->location.y;

    pixelchunk* top_vertex = &(layer->map->groups_array_2d[top_location_x][top_location_y]);
    pixelchunk* right_vertex = &(layer->map->groups_array_2d[right_location_x][right_location_y]);

    if(currentchunk_p->shape_chunk_in == NULL) { //sanity check isolated chunk
        chunkshape* new_shape = add_new_shape(layer->map, currentchunk_p->average_colour);
        layer->map->shape_list = new_shape;

    }

    else if(currentchunk_p->shape_chunk_in->boundaries_length != 1 &&
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
    
    for(int map_y = 0; map_y < layer->map.map_height; ++map_y)
    {
        for(int map_x = 0; map_x < layer->bounds.map_width; ++map_x)
        {
            ++count;

            if (tenth_of_map > 0 && count % tenth_of_map == 0)
            {
                ++tenth_count;
                LOG_INFO("layer: %d progress: %d0%%", layer->layer_index, tenth_count);
            }
            pixelchunk* currentchunk_p = &(layer->map->groups_array_2d[map_x][map_y]);

            find_shapes(
                layer,
                currentchunk_p,
                map_x, map_y,
                layer->options->threshold);

            int code = getLastError();

            if (isBadError())
            {
                LOG_ERR("layer: %d, find_shapes failed with code: %d", layer->layer_index, code);
                return;
            }

            else if(layer->options->step_index > 0 && count >= layer->options->step_index) {
                LOG_INFO("layer: %d, step_index reached: %d\n", layer->layer_index, count);
                return;
            }
        }
    }

    if(layer->options->threshold != 0) { //only draw triangles on the top layer
        return;
    }
    LOG_INFO("layer: %d, making triangles", layer->layer_index);

    for(int map_y = 0; map_y < layer->map.map_height; ++map_y)
    {
        for(int map_x = 0; map_x < layer->bounds.map_width; ++map_x)
        {
            ++count;
            pixelchunk* currentchunk_p = &(layer->map->groups_array_2d[map_x][map_y]);
            make_triangle(layer, currentchunk_p);

            int code = getLastError();

            if (isBadError())
            {
                LOG_ERR("layer: %d, find_shapes failed with code: %d", layer->layer_index, code);
                return;
            }

            else if(layer->options->step_index > 0 && count >= layer->options->step_index) {
                LOG_INFO("layer: %d, step_index reached: %d\n", layer->layer_index, count);
                return;
            }
        }
    }
    return;
}
