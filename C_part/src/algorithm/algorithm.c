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

float get_quadrant_zip_Xoffset(Quadrant* quadrant) {
    return (float)quadrant->X_seam * ZIP_DISTANCE * 2;
}

float get_quadrant_zip_Yoffset(Quadrant* quadrant) {
    return (float)quadrant->Y_seam * ZIP_DISTANCE * 2;
}

bool zip_quadrant(Quadrant* quadrant, pixelchunk* chunk_to_zip) {
    float offset_x = get_quadrant_zip_Xoffset(quadrant);
    float offset_y = get_quadrant_zip_Yoffset(quadrant);
    chunk_to_zip->border_location.x = chunk_to_zip->border_location.x + offset_x;
    chunk_to_zip->border_location.y = chunk_to_zip->border_location.y + offset_y;
}

bool zip_seam(Quadrant* quadrant, pixelchunk* chunk_to_zip, pixelchunk* adjacent) {
    if(adjacent != NULL) { //chunk could be on the edge of the image
        vector2 diff = create_vector_between_chunks(chunk_to_zip, adjacent);
        float offset_x = get_border_zip_offset(diff.x);
        float offset_y = get_border_zip_offset(diff.y);
        chunk_to_zip->border_location.x = chunk_to_zip->border_location.x + offset_x;
        chunk_to_zip->border_location.y = chunk_to_zip->border_location.y + offset_y;
    }
    zip_quadrant(quadrant, chunk_to_zip);
}

void windback_lists(chunkmap* map) {
    chunkshape* current = map->first_shape;

    while(current != NULL) {
        current->chunks = current->chunks->first;
        current->boundaries = current->boundaries->first;
        current = current->next;
    }
}

void add_chunk_to_shape(Quadrant* quadrant, chunkshape* shape, pixelchunk* chunk) {
    if(shape == NULL || chunk == NULL) {
        LOG_ERR("%s: add_chunk_to_shape given null pointer!", quadrant->name);
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
        new->first = new;
    }

    else {
        new->first = shape->chunks->first; //must set this before adding to chunkslist
        shape->chunks->next = new;
        shape->chunks = new;
    }
    chunk->shape_chunk_in = shape;
    ++shape->chunks_amount;
    shape->chunks = new;
}

void add_chunk_to_boundary(Quadrant* quadrant, chunkshape* shape, pixelchunk* chunk) {
    if(shape == NULL || chunk == NULL) { //sanity check
        LOG_ERR("%s: add_chunk_to_boundary given null pointer!", quadrant->name);
        setError(ASSUMPTION_WRONG);
        return;
    }
    
    else if(chunk->boundary_chunk_in != NULL || shape == chunk->shape_chunk_in) { //chunk already in boundary or in another shapes boundaries
        return;
    }
    pixelchunk_list* new = calloc(1, sizeof(pixelchunk_list));
    new->chunk_p = chunk;
    new->next = NULL;
     
    if(shape->boundaries == NULL) {
        shape->boundaries = new;
        new->first = new;
    }

    else {
        new->first = shape->boundaries->first;
        shape->boundaries->next = new;
        shape->boundaries = new;
    }
    ++shape->boundaries_length;
    chunk->boundary_chunk_in = new;
    chunk->shape_chunk_in = NULL; //just paranoid that add_chunk_to_shape will fail
    add_chunk_to_shape(quadrant, shape, chunk); //boundaries are part of the shape too
}

chunkshape* add_new_shape(Quadrant* quadrant, pixel colour) {
    chunkshape* new = calloc(1, sizeof(chunkshape));

    if (!new) {
        LOG_ERR("%s: allocation failed", quadrant->name);        
        setError(ASSUMPTION_WRONG);
        return NULL;
    } 

    new->previous = quadrant->map->shape_list;
    new->next = NULL;
    new->chunks = NULL;
    new->boundaries = NULL;
    new->boundaries_length = 0;
    new->chunks_amount = 0;

    if(quadrant->map->shape_list != NULL) 
        quadrant->map->shape_list->next = new; //links to the previous last item
    else
        quadrant->map->first_shape = new; //if there was no previous item, this is the first item
        
    quadrant->map->shape_list = new; //sets the linked list to last item

    new->colour = colour;
    ++quadrant->map->shape_count;
    return new;
}

chunkshape* merge_shapes(
    Quadrant* quadrant,
    chunkshape* shape1, 
    chunkshape* shape2) {
    if(shape1 == NULL || shape2 == NULL) {
        LOG_ERR("%s: null shape passed to merge shapes!", quadrant->name);
        setError(ASSUMPTION_WRONG);
        return NULL;
    }

    // Find smallest shape
    chunkshape* smaller = (shape1->chunks_amount < shape2->chunks_amount ? shape1 : shape2);
    chunkshape* larger = (smaller == shape1 ? shape2 : shape1);

    // in the smaller shape replace every chunk's shape
    pixelchunk_list* chunk_first = smaller->chunks->first;
    
    for (pixelchunk_list* current = smaller->chunks->first; current != NULL; current = current->next) {
        current->chunk_p->shape_chunk_in = larger;
        current->first = larger->chunks->first;
    }

    pixelchunk_list* boundary_first = smaller->boundaries->first;

    for(pixelchunk_list* current = smaller->boundaries->first; current != NULL; current = current->next) {
        current->chunk_p->shape_chunk_in = larger; //not all chunks are boundaries. merge all chunks twice
        current->first = larger->boundaries->first;
    }

    //append the chunk lists
    larger->chunks->next = chunk_first;
    larger->chunks = smaller->chunks;
    larger->chunks_amount += smaller->chunks_amount;

    larger->boundaries->next = boundary_first;
    larger->boundaries = smaller->boundaries;
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
    return larger;
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
        chosenshape = add_new_shape(quadrant, chunk_to_add->average_colour);
    }

    else { //use chunks shape
        chosenshape = chunk_to_add->shape_chunk_in;
    }
    
    add_chunk_to_boundary(quadrant, chosenshape, chunk_to_add); //add to boundary

    if(isBadError()) {
        LOG_ERR("%s: add_chunk_to_boundary failed with code: %d", quadrant->name, getLastError());
        return;
    }
    zip_seam(quadrant, chunk_to_add, adjacent);
}

void enlarge_shape(
    Quadrant* quadrant,
    pixelchunk* current,
    pixelchunk* adjacent) {
    chunkshape* chosenshape;

    //both chunks go in fresh shape
    if(current->shape_chunk_in == NULL && adjacent->shape_chunk_in == NULL) {
        chosenshape = add_new_shape(quadrant, current->average_colour);
        add_chunk_to_shape(quadrant, chosenshape, current);
        add_chunk_to_shape(quadrant, chosenshape, adjacent);
        chosenshape->colour = current->average_colour;
    }

    //adjacent goes in currents shape
    else if (current->shape_chunk_in && adjacent->shape_chunk_in == NULL)
    {
        chosenshape = current->shape_chunk_in;
        add_chunk_to_shape(quadrant, chosenshape, adjacent);
    }

    //current goes in adjacents shape
    else if(current->shape_chunk_in == NULL && adjacent->shape_chunk_in)
    {
        chosenshape = adjacent->shape_chunk_in;
        add_chunk_to_shape(quadrant, chosenshape, current);
    }
    
    //merge the two shapes
    else if (current->shape_chunk_in != adjacent->shape_chunk_in) {
        chosenshape = merge_shapes(quadrant, current->shape_chunk_in, adjacent->shape_chunk_in);
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
            }
        }
    }
}

void make_triangle(Quadrant* quadrant, pixelchunk* currentchunk_p) {  
    float Xoffset = get_quadrant_zip_Xoffset(quadrant);
    float Yoffset = get_quadrant_zip_Yoffset(quadrant);

    int top_location_x = (float)currentchunk_p->location.x + Xoffset;
    int top_location_y = (float)currentchunk_p->location.y - 1 + Yoffset;

    int right_location_x = (float)currentchunk_p->location.x + 1 + Xoffset;
    int right_location_y = (float)currentchunk_p->location.y + Yoffset;

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
        chunkshape* new_shape = add_new_shape(quadrant, currentchunk_p->average_colour);
        quadrant->map->shape_list = new_shape;
        add_chunk_to_boundary(quadrant, new_shape, currentchunk_p);
        zip_quadrant(quadrant, currentchunk_p);
    }

    else if(currentchunk_p->shape_chunk_in->chunks_amount != 1) //only form triangle on isolated chunk
    {
        return; //only allow single pixel shapes through
    }

    else if(currentchunk_p->boundary_chunk_in == NULL) {
        add_chunk_to_boundary(quadrant, currentchunk_p->shape_chunk_in, currentchunk_p);
        zip_quadrant(quadrant, currentchunk_p);
    }

    if(top_vertex->shape_chunk_in != NULL || right_vertex->shape_chunk_in != NULL) { //dont put chunks in multiple shapes
        return;
    }
    chunkshape* triangle = currentchunk_p->shape_chunk_in;
    add_chunk_to_boundary(quadrant, triangle, top_vertex);
    zip_quadrant(quadrant, top_vertex);
    add_chunk_to_boundary(quadrant, triangle, right_vertex);
    zip_quadrant(quadrant, right_vertex);
    
    if(isBadError()) {
        LOG_ERR("%s: add_chunk_to_boundary failed with code: %d", quadrant->name, getLastError());
        return;
    }
}

void remove_loner(Quadrant* quadrant, pixelchunk* chunk) {
    if(chunk->shape_chunk_in->chunks_amount != 1) { //remove single pixel shape
        return;
    }
    chunkshape* shape = chunk->shape_chunk_in;

    if(shape == NULL) {
        LOG_ERR("%s: chunk did not have a shape! %.2x, %.2y", quadrant->name, chunk->location.x, chunk->location.y);
        setError(ASSUMPTION_WRONG);
        return;
    }
    chunk->shape_chunk_in = NULL;
    chunk->boundary_chunk_in = NULL;

    if(shape->previous != NULL)
        shape->previous->next = shape->next;

    shape->previous = NULL;

    if(shape->next != NULL)
        shape->next->previous = shape->previous;

    shape->next = NULL;

    if(quadrant->map->first_shape == shape) {
        quadrant->map->first_shape = shape->next;
    }

    if(shape->chunks && shape->chunks->first) {
        free_pixelchunklist(shape->chunks->first);
        shape->chunks = NULL;
    }

    if(shape->boundaries && shape->boundaries->first) {
        free_pixelchunklist(shape->boundaries->first);
        shape->boundaries = NULL;
    }
    free(shape);
}

///A multithreaded function
void* fill_quadrant(void* arg) {
    Quadrant* quadrant = (Quadrant*)arg;
    int count = 0;
    int tenth_count = 0;
    int tenth_of_map = (int)floor(quadrant->map->map_width * quadrant->map->map_height / 10.f);

    // check if quadrant is actually big enough
    if (quadrant->bounds.endingX - quadrant->bounds.startingX <= 1 || quadrant->bounds.endingY - quadrant->bounds.startingY <= 1)
    {
        LOG_ERR("quadrant '%s' is not big enough!", quadrant->name);
        setError(ASSUMPTION_WRONG);
        pthread_exit(NULL);
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
                pthread_exit(NULL);
            }

            make_triangle(quadrant, currentchunk_p);

            if (isBadError())
            {
                LOG_ERR("%s make_triangle failed with code: %d", quadrant->name, code);
                pthread_exit(NULL);
            }

            remove_loner(quadrant, currentchunk_p);

            if (isBadError())
            {
                LOG_ERR("%s remove_loner failed with code: %d", quadrant->name, code);
                pthread_exit(NULL);
            }

            if(quadrant->options->step_index > 0 && count >= quadrant->options->step_index) {
                LOG_INFO("%s: step_index reached: %d\n", quadrant->name, count);
                pthread_exit(NULL);
            }
        }
    }
    pthread_exit(NULL);
}

void fill_chunkmap(chunkmap* map, vectorize_options* options) {
    //create set of shapes
    LOG_INFO("Filling chunkmap");

    int middle_width = (int)floor((float)map->map_width / (float)2); //int divisions return 0 by default. use float division
    int middle_height = (int)floor((float)map->map_height / (float)2);

    LOG_INFO("creating quadrants");
    Quadrant quadrant1 = {"top-left", map, options, POSITIVE, POSITIVE};
    quadrant1.bounds.startingX = 0;
    quadrant1.bounds.startingY = 0;
    quadrant1.bounds.endingX = middle_width;
    quadrant1.bounds.endingY = middle_height;

    chunkmap* map2 = generate_chunkmap(map->input, *options);
    Quadrant quadrant2 = {"top-right", map2, options, NEGATIVE, POSITIVE};
    quadrant2.bounds.startingX = middle_width;
    quadrant2.bounds.startingY = 0;
    quadrant2.bounds.endingX = map->map_width;
    quadrant2.bounds.endingY = middle_height; 

    chunkmap* map3 = generate_chunkmap(map->input, *options);
    Quadrant quadrant3 = {"bottom-left", map3, options, POSITIVE, NEGATIVE};
    quadrant3.bounds.startingX = 0;
    quadrant3.bounds.startingY = middle_height;
    quadrant3.bounds.endingX = middle_width;
    quadrant3.bounds.endingY = map->map_height;

    chunkmap* map4 = generate_chunkmap(map->input, *options);
    Quadrant quadrant4 = {"bottom-right", map4, options, NEGATIVE, NEGATIVE};
    quadrant4.bounds.startingX = middle_width;
    quadrant4.bounds.startingY = middle_height;
    quadrant4.bounds.endingX = map->map_width;
    quadrant4.bounds.endingY = map->map_height;

    LOG_INFO("creating threads");
    pthread_t thread1;
    pthread_t thread2;
    pthread_t thread3;
    pthread_t thread4;
    pthread_create(&thread1, NULL, fill_quadrant, &quadrant1);
    pthread_create(&thread2, NULL, fill_quadrant, &quadrant2);
    pthread_create(&thread3, NULL, fill_quadrant, &quadrant3);
    pthread_create(&thread4, NULL, fill_quadrant, &quadrant4);
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
        return;
    }
    LOG_INFO("appending shapes from threads");
    
    map3->shape_list->next = map4->first_shape;
    map4->first_shape->previous = map3->shape_list;
    map3->shape_count += map4->shape_count;

    map2->shape_list->next = map3->first_shape;
    map3->first_shape->previous = map2->shape_list;
    map2->shape_count += map3->shape_count;

    map->shape_list->next = map2->first_shape;
    map2->first_shape->previous = map->shape_list;
    map->shape_count += map2->shape_count;

    LOG_INFO("winding back list");
    map->shape_list = map->first_shape;
    windback_lists(map);
}
