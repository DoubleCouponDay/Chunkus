#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <nanosvg.h>

#include "svg.h"
#include "../types/colour.h"
#include "../types/image.h"
#include "../mapping.h"
#include "../../test/tools.h"
#include "../tidwall.h"
#include "../error.h"
#include "tidwallcopy.h"

const int BEZIERCURVE_LENGTH = 8;
const int BEZIER_POINTS = 2;
const int BOUNDS_LENGTH = 4;
const int NONE_SIMILAR = 8;
const int NONE_FILLED = -1;
const char* TEMPLATE_PATH = "template.svg";
const char* OOM_MESSAGE = "hashmap out of mana\n";

void fill_float_array(float* input, int input_length, float* output, int output_length) {
    if(input_length > output_length) {
        DEBUG("output array is too small for your input.");
        exit(ARRAY_DIFF_SIZE_ERROR);
    }

    for(int i = 0; i < input_length; ++i) {
        output[i] = input[i];
    }
}

void fill_beziercurve_array(float* array,
                    int array_length,
                    float x1, float y1, 
                    float x2, float y2, 
                    float control_x1, float control_y1, 
                    float control_x2, float control_y2)
{
    if(array_length != BEZIERCURVE_LENGTH) {
        DEBUG("beziercurve array must be 8 long.");
        exit(ARRAY_DIFF_SIZE_ERROR);
    }
    array[0] = x1;
    array[1] = y1;
    array[2] = x2;
    array[3] = y2;
    array[4] = control_x1;
    array[5] = control_y1;
    array[6] = control_x2;
    array[7] = control_y2;
}

NSVGpath* create_path(image input, coordinate start, coordinate end) {
    NSVGpath* output = calloc(1, sizeof(NSVGpath));
    output->npts = 2;
    fill_beziercurve_array(output->pts, BEZIERCURVE_LENGTH, start.x, start.y, end.x, end.y, 0, 0, 1, 1); //draw the top side of a box
    float boundingbox[4] = { 0, 0, input.width, input.height };
    fill_float_array(boundingbox, BOUNDS_LENGTH, output->bounds, BOUNDS_LENGTH);
    output->closed = 1;
    return output;
}

bool colours_are_similar(pixel color_a, pixel color_b, float max_distance)
{
    pixel diff;
    diff.r = color_a.r - color_b.r;
    diff.g = color_a.g - color_b.g;
    diff.b = color_a.b - color_b.b;

    float mag = sqrt(pow(diff.r, 2) + pow(diff.g, 2) + pow(diff.b, 2)); //pythagorean theorem

    return mag <= max_distance;
}

chunkshape* add_new_shape(chunkshape* shape_list) {
    chunkshape* new = calloc(1, sizeof(chunkshape));
    if (!new) {
        // Uh oh! Your allocation failed! You should really account for this...
        return NULL;
    }
    DEBUG("allocating new hashmap\n");
    hashmap* newhashy = hashmap_new(sizeof(chunkshape), 16, 0, 0, chunk_hash, chunk_compare, NULL);    
    DEBUG("Prepare new chunkshape to be spliced in\n");
    new->chunks = newhashy;
    new->next = NULL;
    new->previous = shape_list;
    DEBUG("Put the new node into the list");
    shape_list->next = new;
    return new;
}

void add_chunk_to_shape(chunkshape* shape_list, pixelchunk* item) {
    if(hashmap_oom(shape_list->chunks)){
        DEBUG("hashmap out of mana\n");
        exit(OOM_MESSAGE);
    }
    hashmap_set(shape_list->chunks, item);
}

//returns the shape its in. else, NULL
chunkshape* big_chungus_already_in_shape(chunkmap map, pixelchunk* chungus) {
    chunkshape* current = map.shape_list;

    wind_back_chunkshapes(&current);

    while(current->next != NULL) {
        if (!hashmap_get(current->chunks, chungus))
            return current;

        current = current->next;
    }
    return NULL;
}

void find_shapes(chunkmap map, pixelchunk* current, int map_x, int map_y, float shape_colour_threshold)
{
    int num_not_similar = 0;

    for (int adjacent_x = -1; adjacent_x < 2; ++adjacent_x)
    {
        for (int adjacent_y = -1; adjacent_y < 2; ++adjacent_y)
        {
            if (adjacent_x == 0 && adjacent_y == 0)
                continue; //skip center pixel
            
            int adjacent_index_x = map_x + adjacent_x;
            int adjacent_index_y = map_y + adjacent_y;

            //prevent out of bounds index
            if (adjacent_index_x < 0 || 
                adjacent_index_y < 0 ||
                adjacent_index_x >= map.map_width ||  
                adjacent_index_y >= map.map_height)
                continue;

            pixelchunk* adjacent = &map.groups_array_2d[adjacent_index_x][adjacent_index_y];

            if (colours_are_similar(current->average_colour, adjacent->average_colour, shape_colour_threshold))
            {
                chunkshape* isinshape = big_chungus_already_in_shape(map, adjacent);
                
                if (isinshape) {
                    add_chunk_to_shape(isinshape, current);
                }

                else if(hashmap_oom(map.shape_list->chunks) == false){
                    map.shape_list = add_new_shape(map.shape_list);                    
                    hashmap_set(map.shape_list->chunks, current);
                    hashmap_set(map.shape_list->chunks, adjacent);
                }

                else { //probably never happen
                    DEBUG("hashmap out of mana\n");
                    exit(HASHMAP_OOM);
                }
            }

            else {
                ++num_not_similar;
                current->is_boundary = true;
            }
        }
    }

    if(num_not_similar == NONE_SIMILAR) {
        if(hashmap_oom(map.shape_list->chunks)) {
            DEBUG("hashmap out of mana\n");
            exit(HASHMAP_OOM);
        }
        map.shape_list = add_new_shape(map.shape_list);
        hashmap_set(map.shape_list->chunks, current);
    }
}

//assumes first path and first shape are given
bool iterate_new_path(void* item, void* udata) {
    pixelchunk* chunk = item;
    iter_struct* shape_data = udata;

    if(chunk->is_boundary == false) {
        return true;
    }
    NSVGshape* current = shape_data->output->shapes;
    NSVGpath* currentpath = current->paths;
    NSVGpath* nextsegment;

    //add chunk to path if its a boundary

    if(currentpath->pts[0] == NONE_FILLED) { //first point not supplied
        coordinate empty = {0, 0};
        currentpath->pts[0] = chunk->location.x; //x1
        currentpath->pts[1] = chunk->location.y; //y1

        shape_data->shapescolour = calloc(1, sizeof(NSVGpaint));    
        NSVGpaint* fill = shape_data->shapescolour;
        fill->type = NSVG_PAINT_COLOR;

        fill->color = NSVG_RGB(
            chunk->average_colour.r, 
            chunk->average_colour.g, 
            chunk->average_colour.b
        );
        return true;
    }

    else if(currentpath->pts[2] == NONE_FILLED) { //first point supplied but not first path
        currentpath->pts[2] = chunk->location.x; //x2
        currentpath->pts[3] = chunk->location.y; //y2

        coordinate previous_coord = {
            currentpath->pts[0],
            currentpath->pts[1]
        };

        nextsegment = create_path(
            shape_data->map.input, 
            previous_coord,
            chunk->location
        );
    }

    else { //first path supplied
        coordinate previous_coord = {
            currentpath->pts[2],
            currentpath->pts[3]
        };

        nextsegment = create_path(
            shape_data->map.input, 
            previous_coord,
            chunk->location
        );
    }
    currentpath->next = nextsegment;
    return true;
}

void close_path(chunkmap* map, NSVGimage* output, NSVGpath* firstpath) {
    coordinate realstart = {
        output->shapes->paths->pts[2],
        output->shapes->paths->pts[3]
    };

    coordinate realend = {
        firstpath->pts[0],
        firstpath->pts[1]
    };        
    output->shapes->paths->next = create_path(map->input, realstart, realend);
}

void iterate_chunk_shapes(chunkmap map, NSVGimage* output)
{
    DEBUG("checking if shapelist is null\n");
    //create the svg
    if(map.shape_list == NULL) {
        DEBUG("NO SHAPES FOUND\n");
        exit(ASSUMPTION_WRONG);
    }
    DEBUG("creating first shape\n");
    NSVGshape* firstshape;

    //iterate shapes
    while(map.shape_list != NULL) {        
        if(output->shapes == NULL) {
            firstshape = calloc(1, sizeof(NSVGshape));
            output->shapes = firstshape;
        }

        else {
            NSVGshape* newshape = calloc(1, sizeof(NSVGshape));
            output->shapes->next = newshape;
            output->shapes = newshape;
        }
        
        coordinate empty = {NONE_FILLED, NONE_FILLED};
        NSVGpath* firstpath = create_path(map.input, empty, empty);

        iter_struct shape_data = {
            map, output, firstpath, NULL
        };
        output->shapes->paths = firstpath; //first shapes path

        if(hashmap_count(map.shape_list->chunks) == 0) {
            continue;    
        }
        hashmap_scan(map.shape_list->chunks, iterate_new_path, &shape_data);

        if(firstpath->pts[2] == NONE_FILLED) {
            DEBUG("NO PATHS FOUND\n");
            exit(ASSUMPTION_WRONG);
        }

        close_path(&map, output, firstpath);
        output->shapes->paths = firstpath; //wind back the paths
        
        //set the colour of the shape
        output->shapes->fill = *shape_data.shapescolour;
        free(shape_data.shapescolour);

        NSVGpaint stroke = {
            NSVG_PAINT_NONE,
            NSVG_RGB(0, 0, 0)
        };
        output->shapes->stroke = stroke;
        map.shape_list = map.shape_list->next; //go to next shape
                
    }
    output->shapes = firstshape;
}

//entry point of the file
NSVGimage* vectorize_image(image input, vectorize_options options) {
    DEBUG("Beginning vectorize_image\n");
    DEBUG("checking template file exists\n");
    FILE* filefound = fopen(TEMPLATE_PATH, "rb");

    if(filefound == NULL) {
        DEBUG("could not find template file: %s", TEMPLATE_PATH);
    }
    fclose(filefound);
    NSVGimage* output = nsvgParseFromFile(TEMPLATE_PATH, "px", 0);
    output->width = input.width;
    output->height = input.height;

    DEBUG("generating chunkmap\n");
    chunkmap map = generate_chunkmap(input, options);

    //create set of shapes
    for(int map_x = 0; map_x < map.map_width; ++map_x)
    {
        for(int map_y = 0; map_y < map.map_height; ++map_y)
        {
            DEBUG("finding shape for chunk\n");
            pixelchunk* currentchunk_p = &map.groups_array_2d[map_x][map_y];
            find_shapes(map, currentchunk_p, map_x, map_y, options.shape_colour_threshhold);
        }
    }

    DEBUG("Now winding back chunk_shapes\n");
    wind_back_chunkshapes(&map.shape_list);
    
    DEBUG("iterating chunk shapes\n");
    iterate_chunk_shapes(map, output);
    
    DEBUG("freeing group map\n");
    free_group_map(&map);
    return output;
}

void free_image(NSVGimage* input) {
    if(!input) {
        DEBUG("input is null");
        exit(NULL_ARGUMENT_ERROR);
    }

    while(input->shapes != NULL) {
        NSVGpath* currentpath = input->shapes->paths;

        while(currentpath != NULL) {
            NSVGpath* nextpath = currentpath->next;
            free(currentpath);
            currentpath = nextpath;
        }        
        NSVGshape* nextshape = input->shapes->next;
        free(input->shapes);
        input->shapes = nextshape;
    }
    free(input);
}
