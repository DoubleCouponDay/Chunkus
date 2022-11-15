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

float get_quadrant_zip_offset(int seam) {
    return (float)seam * ZIP_DISTANCE * 2;
}

void zip_quadrant(Quadrant* quadrant, pixelchunk* chunk_to_zip) {
    float offset_x = get_quadrant_zip_offset(quadrant->X_seam);
    float offset_y = get_quadrant_zip_offset(quadrant->Y_seam);
    chunk_to_zip->border_location.x = chunk_to_zip->border_location.x + offset_x;
    chunk_to_zip->border_location.y = chunk_to_zip->border_location.y + offset_y;
}

void zip_seam(Quadrant* quadrant, pixelchunk* chunk_to_zip, pixelchunk* adjacent) {
    if(adjacent != NULL) { //chunk could be on the edge of the image
        vector2 diff = create_vector_between_chunks(chunk_to_zip, adjacent);
        float offset_x = get_border_zip_offset(diff.x);
        float offset_y = get_border_zip_offset(diff.y);
        chunk_to_zip->border_location.x = chunk_to_zip->border_location.x + offset_x;
        chunk_to_zip->border_location.y = chunk_to_zip->border_location.y + offset_y;
    }
    zip_quadrant(quadrant, chunk_to_zip);
}

void add_chunk_to_shape(chunkmap* map, chunkshape* shape, pixelchunk* chunk) {
    if(shape == NULL || chunk == NULL) {
        LOG_ERR("add_chunk_to_shape given null pointer!");
        setError(ASSUMPTION_WRONG);
        return;
    }

    else if(chunk->shape_chunk_in != NULL || shape == chunk->shape_chunk_in) {
        return;
    }
    pixelchunk_list* new = calloc(1, sizeof(pixelchunk_list));
    new->chunk_p = chunk;
    new->next = NULL;
     
    if(shape->chunks == NULL) {
        shape->chunks = new;
        shape->first_chunk = new;
    }

    else {
        shape->chunks->next = new;
        shape->chunks = new;
    }
    chunk->shape_chunk_in = shape;
    ++shape->chunks_amount;
    shape->chunks = new;
}

/// @brief returns whether the current chunk was added to the boundary or not
/// @param quadrant 
/// @param shape 
/// @param chunk 
/// @param allow_multiple_shapes 
/// @return 
bool add_chunk_to_boundary(Quadrant* quadrant, chunkshape* shape, pixelchunk* chunk, bool allow_multiple_shapes) {
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

    else {
        current_sorted = sort_boundary_chunk(quadrant, shape, chunk); //dont allocate a boundary list item until circumstances are clear
    }
    ++shape->boundaries_length;

    if(isBadError()) {
        LOG_ERR("%s: sort_boundary_chunk failed with code: %d", quadrant->name, getLastError());
        return current_sorted;
    }
    chunk->shape_chunk_in = NULL; //just paranoid that add_chunk_to_shape will fail
    add_chunk_to_shape(quadrant->map, shape, chunk); //boundaries are part of the shape too
    return current_sorted;
}

chunkshape* add_new_shape(chunkmap* map, pixel colour) {
    chunkshape* new = calloc(1, sizeof(chunkshape));

    if (!new) {
        LOG_ERR("allocation failed");        
        setError(ASSUMPTION_WRONG);
        return NULL;
    } 

    new->previous = map->shape_list;
    new->next = NULL;
    new->chunks = NULL;
    new->boundaries = NULL;
    new->boundaries_length = 0;
    new->chunks_amount = 0;

    if(map->shape_list != NULL) 
        map->shape_list->next = new; //links to the previous last item
    else
        map->first_shape = new; //if there was no previous item, this is the first item
        
    map->shape_list = new; //sets the linked list to last item

    new->colour = colour;
    ++map->shape_count;
    return new;
}

void merge_shapes(
    Quadrant* quadrant,
    chunkshape* shape1, 
    chunkshape* shape2) {
    if(shape1 == NULL || shape2 == NULL) {
        LOG_ERR("%s: null shape passed to merge shapes!", quadrant->name);
        setError(ASSUMPTION_WRONG);
        return;
    }

    // Find smallest shape
    chunkshape* smaller = (shape1->chunks_amount < shape2->chunks_amount ? shape1 : shape2);
    chunkshape* larger = (smaller == shape1 ? shape2 : shape1);

    if(smaller->first_boundary == NULL || larger->first_boundary == NULL) {
        LOG_ERR("sorting didn't add a chunk to this new shape!");
        setError(ASSUMPTION_WRONG);
        return;
    }

    // in the smaller shape replace every chunk's shape
    for (pixelchunk_list* current = smaller->first_chunk; current != NULL; current = current->next) {
        current->chunk_p->shape_chunk_in = larger;
    }

    //append the chunk lists
    larger->chunks->next = smaller->first_chunk;
    larger->chunks = smaller->chunks;
    larger->chunks_amount += smaller->chunks_amount;

    //connect first and last if they are adjacent
    if(smaller->boundaries_length != 0) {
        sort_boundary_chunk(quadrant, larger, smaller->first_boundary->chunk_p);
        getAndResetErrorCode();
        sort_boundary_chunk(quadrant, larger, smaller->boundaries->chunk_p);
        getAndResetErrorCode();
    }
    
    larger->boundaries_length += smaller->boundaries_length;

    //get rid of smaller shape by cutting it out of the linked list
    if (smaller->previous) {
        smaller->previous->next = smaller->next;
    }

    if(smaller->next) {
        smaller->next->previous = smaller->previous;
    }

    if(quadrant->map->first_shape == smaller) { //smaller is first item
        quadrant->map->first_shape = quadrant->map->first_shape->next;
    }
    if (quadrant->map->shape_list == smaller) { //smaller is last item
        quadrant->map->shape_list = quadrant->map->shape_list->previous;
    }

    smaller->chunks = 0;
    smaller->boundaries = 0;
    smaller->chunks_amount = -1;
    smaller->boundaries_length = -1;
    free(smaller);

    --quadrant->map->shape_count;
}

void enlarge_border(
    Quadrant* quadrant,
    pixelchunk* chunk_to_add,
    pixelchunk* adjacent) {
    chunkshape* chosenshape;

    if(chunk_to_add->boundary_chunk_in != NULL) {
        return; //chunk is already a boundary
    }

    if(quadrant->map->shape_list == NULL || chunk_to_add->shape_chunk_in == NULL) { //make first shape
        chosenshape = add_new_shape(quadrant->map, chunk_to_add->average_colour);
    }

    else { //use chunks shape
        chosenshape = chunk_to_add->shape_chunk_in;
    }
    
    bool current_sorted = add_chunk_to_boundary(quadrant, chosenshape, chunk_to_add, false); //add to boundary

    if(isBadError()) {
        LOG_ERR("%s: add_chunk_to_boundary failed with code: %d", quadrant->name, getLastError());
        return;
    }

    if(current_sorted == false) //dont zip if current not sorted
        return;

    zip_seam(quadrant, chunk_to_add, adjacent);
}

void enlarge_shape(
    Quadrant* quadrant,
    pixelchunk* current,
    pixelchunk* adjacent) {
    chunkshape* chosenshape;

    //both chunks go in fresh shape
    if(current->shape_chunk_in == NULL && adjacent->shape_chunk_in == NULL) {
        chosenshape = add_new_shape(quadrant->map, current->average_colour);
        add_chunk_to_shape(quadrant->map, chosenshape, current);
        add_chunk_to_shape(quadrant->map, chosenshape, adjacent);
        chosenshape->colour = current->average_colour;
    }

    //adjacent goes in currents shape
    else if (current->shape_chunk_in && adjacent->shape_chunk_in == NULL)
    {
        chosenshape = current->shape_chunk_in;
        add_chunk_to_shape(quadrant->map, chosenshape, adjacent);
    }

    //current goes in adjacents shape
    else if(current->shape_chunk_in == NULL && adjacent->shape_chunk_in)
    {
        chosenshape = adjacent->shape_chunk_in;
        add_chunk_to_shape(quadrant->map, chosenshape, current);
    }
    
    //merge the two shapes
    else if (current->shape_chunk_in != adjacent->shape_chunk_in) {
        merge_shapes(quadrant, current->shape_chunk_in, adjacent->shape_chunk_in);
    }

    if(isBadError()) {
        LOG_ERR("%s: enlarge_shape failed with code: %d", quadrant->name, getLastError());
        return;
    }
}

void find_shapes(
    Quadrant* quadrant, 
    pixelchunk* current,
    int map_x, int map_y, 
    float threshold) {

    if(map_x == quadrant->bounds.startingX || map_x == (quadrant->bounds.endingX - 1) ||
        map_y == quadrant->bounds.startingY || map_y == (quadrant->bounds.endingY - 1)) 
    {
        enlarge_border(quadrant, current, NULL); //add pixel on the edge of the image to a border

        if(isBadError()) {
            LOG_ERR("%s: enlarge_border failed with code: %d", quadrant->name, getLastError());
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
            if (adjacent_index_x < quadrant->bounds.startingX || 
                adjacent_index_y < quadrant->bounds.startingY ||
                adjacent_index_x >= quadrant->bounds.endingX ||  
                adjacent_index_y >= quadrant->bounds.endingY)
                continue;

            pixelchunk* adjacent = &(quadrant->map->groups_array_2d[adjacent_index_x][adjacent_index_y]);

            //make a shape out of two adjacent chunks
            if (colours_are_similar(current->average_colour, adjacent->average_colour, threshold)) {
                enlarge_shape(quadrant, current, adjacent);

                if(isBadError()) {
                    LOG_ERR("%s: enlarge_shape failed with code: %d", quadrant->name, getLastError());
                    return;
                }
            }

            //define a boundary between two chunks
            else {
                enlarge_border(quadrant, current, adjacent);
                
                if(isBadError()) {
                    LOG_ERR("%s: enlarge_border failed with code: %d", quadrant->name, getLastError());
                    return;
                }
            }
        }
    }
}

void make_triangle(Quadrant* quadrant, pixelchunk* currentchunk_p) {  
    int top_location_x = currentchunk_p->location.x;
    int top_location_y = currentchunk_p->location.y + 1;

    int right_location_x = currentchunk_p->location.x + 1;
    int right_location_y = currentchunk_p->location.y;

    if(top_location_x < quadrant->bounds.startingX || top_location_x >= quadrant->bounds.endingX || 
        top_location_y < quadrant->bounds.startingY || top_location_y >= quadrant->bounds.endingY ||
        right_location_x < quadrant->bounds.startingX || right_location_x >= quadrant->bounds.endingX ||
        right_location_y < quadrant->bounds.startingY || right_location_y >= quadrant->bounds.endingY) 
    {
        return;
    }
    pixelchunk* top_vertex = &(quadrant->map->groups_array_2d[top_location_x][top_location_y]);
    pixelchunk* right_vertex = &(quadrant->map->groups_array_2d[right_location_x][right_location_y]);

    if(currentchunk_p->shape_chunk_in == NULL) { //sanity check isolated chunk
        chunkshape* new_shape = add_new_shape(quadrant->map, currentchunk_p->average_colour);
        quadrant->map->shape_list = new_shape;
        add_chunk_to_boundary(quadrant, new_shape, currentchunk_p, true);
    }

    else if(currentchunk_p->shape_chunk_in->boundaries_length != 1 &&
            currentchunk_p->shape_chunk_in->boundaries_length != 2)  //sadly this can add the same chunk to its own boundary twice
    {
        //only form triangle on unformed chunks        
        return;
    }

    else if(currentchunk_p->boundary_chunk_in == NULL) {
        add_chunk_to_boundary(quadrant, currentchunk_p->shape_chunk_in, currentchunk_p, true);
    }
    chunkshape* triangle = currentchunk_p->shape_chunk_in;

    if(top_vertex->shape_chunk_in == NULL) {
        top_vertex->shape_chunk_in = triangle;
        add_chunk_to_boundary(quadrant, triangle, top_vertex, true);
    }

    else {
        add_chunk_to_boundary(quadrant, triangle, top_vertex, true);
    }
    

    if(right_vertex->shape_chunk_in == NULL) {
        right_vertex->shape_chunk_in = triangle;
        add_chunk_to_boundary(quadrant, triangle, right_vertex, true);
    }

    else {
        add_chunk_to_boundary(quadrant, triangle, right_vertex, true);
    }
    
    if(isBadError()) {
        LOG_ERR("add_chunk_to_boundary failed with code: %d", getLastError());
        return;
    }
}

void fill_quadrant(Quadrant* quadrant) {
    LOG_INFO("Filling quadrant: %s", quadrant->name);
    long count = 0;
    int tenth_count = 0;
    int tenth_of_map = (int)floor(quadrant->map->map_width * quadrant->map->map_height / 10.f);

    // check if quadrant is actually big enough
    if (quadrant->bounds.endingX - quadrant->bounds.startingX <= 1 || quadrant->bounds.endingY - quadrant->bounds.startingY <= 1)
    {
        LOG_ERR("quadrant '%s' is not big enough!", quadrant->name);
        setError(ASSUMPTION_WRONG);
        return;
    }
    
    for(int map_y = quadrant->bounds.startingY; map_y < quadrant->bounds.endingY; ++map_y)
    {
        for(int map_x = quadrant->bounds.startingX; map_x < quadrant->bounds.endingX; ++map_x)
        {
            ++count;
            if (tenth_of_map > 0 && count % tenth_of_map == 0)
            {
                ++tenth_count;
                LOG_INFO("%s progress: %d0%%", quadrant->name, tenth_count);
            }
            pixelchunk* currentchunk_p = &(quadrant->map->groups_array_2d[map_x][map_y]);
            
            find_shapes(
                quadrant, 
                currentchunk_p,
                map_x, map_y, 
                quadrant->options->threshold);            
            
            int code = getLastError();

            if (isBadError())
            {
                LOG_ERR("%s find_shapes failed with code: %d", quadrant->name, code);
                return;
            }

            else if(quadrant->options->step_index > 0 && count >= quadrant->options->step_index) {
                LOG_INFO("%s: step_index reached: %d\n", quadrant->name, count);
                return;
            }
        }
    }

    if(quadrant->options->threshold != 0) {
        return;
    }
    LOG_INFO("%s: making triangles", quadrant->name);

    for(int map_y = quadrant->bounds.startingY; map_y < quadrant->bounds.endingY; ++map_y)
    {
        for(int map_x = quadrant->bounds.startingX; map_x < quadrant->bounds.endingX; ++map_x)
        {
            ++count;
            pixelchunk* currentchunk_p = &(quadrant->map->groups_array_2d[map_x][map_y]);
            make_triangle(quadrant, currentchunk_p);

            int code = getLastError();

            if (isBadError())
            {
                LOG_ERR("%s find_shapes failed with code: %d", quadrant->name, code);
                return;
            }

            else if(quadrant->options->step_index > 0 && count >= quadrant->options->step_index) {
                LOG_INFO("%s: step_index reached: %d\n", quadrant->name, count);
                return;
            }
        }
    }
    return;
}
