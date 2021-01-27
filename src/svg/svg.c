#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
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
#include "nsvgcopy.h"
#include "tidwallcopy.h"
#include "nsvgmapping.h"
#include "../imagefile.h"

const int NONE_SIMILAR = 8;
const int NONE_FILLED = -1;
const char* TEMPLATE_PATH = "template.svg";
const char* OOM_MESSAGE = "hashmap out of mana\n";

chunkshape* add_new_shape(chunkshape* shape_list) {
    chunkshape* new = calloc(1, sizeof(chunkshape));

    if (!new) {
        DEBUG("Uh oh! Your allocation failed! You should really account for this...\n");
        exit(NULL_ARGUMENT_ERROR);
    }
    hashmap* newhashy = hashmap_new(sizeof(chunkshape), 16, 0, 0, chunk_hash, chunk_compare, NULL);

    if(newhashy == NULL) {
        DEBUG("big problem\n");
        exit(ASSUMPTION_WRONG);
    }  
    new->chunks = newhashy;
    new->next = NULL;
    new->previous = shape_list;
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
                DEBUG("border found\n");
                chunkshape* isinshape = big_chungus_already_in_shape(map, adjacent);
                
                if (isinshape) {
                    DEBUG("adjacent is in shape\n");
                    add_chunk_to_shape(isinshape, current);
                }

                else if(hashmap_oom(map.shape_list->chunks) == false){
                    DEBUG("adjacent is not in shape\n");
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
bool iterate_new_path(const void* item, void* udata) {
    DEBUG("creating new path\n");
    pixelchunk* chunk = item;
    iter_struct* shape_data = udata;

    if(chunk->is_boundary == false) {
        DEBUG("chunk is not boundary\n");
        return true;
    }
    NSVGshape* current = shape_data->output->shapes;
    NSVGpath* currentpath = current->paths;
    NSVGpath* nextsegment;

    //add chunk to path if its a boundary
    DEBUG("adding chunk to boundary\n");

    if(currentpath->pts[0] == NONE_FILLED) { //first point not supplied
        DEBUG("creating first point\n");
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
        return true; //dont use nextsegment before its defined
    }

    else if(currentpath->pts[2] == NONE_FILLED) { //first point supplied but not first path
        DEBUG("connecting first path\n");
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
        DEBUG("creating new path\n");
        
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

void throw_on_max(unsigned long* subject) {
    if(subject == ULONG_MAX) {
        DEBUG("long is way too big!\n");
        exit(OVERFLOW_ERROR);
    }
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

    DEBUG("iterating shapes list\n");
    char* firstid = "firstshape";
    long firstidlength = 10;
    NSVGshape* firstshape = create_shape(&map, firstid, firstidlength);
    output->shapes = NULL; //get rid of fluff in the template
    unsigned long i = 0;
    bool firstrun = true;

    //iterate shapes
    while(map.shape_list != NULL) {        
        DEBUG("iteration: %d \n", i);
        if(output->shapes == NULL) {
            DEBUG("using first shape\n");
            output->shapes = firstshape;
        }

        else {
            DEBUG("creating new shape\n");
            char longaschar = i;
            NSVGshape* newshape = create_shape(&map, &longaschar, 1);
            output->shapes->next = newshape;
            output->shapes = newshape;
        }
        coordinate empty = {NONE_FILLED, NONE_FILLED};
        NSVGpath* firstpath = create_path(map.input, empty, empty);
        output->shapes->paths = firstpath; //first shapes path
        DEBUG("creating iter struct\n");

        iter_struct shape_data = {
            map, output, firstpath, NULL
        };

        if(firstrun == false && 
            hashmap_count(map.shape_list->chunks) == 0) {
            DEBUG("no chunks found in hashmap\n");
            continue;    
        }
        DEBUG("iterating hashmap, count: %d \n", hashmap_count(map.shape_list->chunks));
        hashmap_scan(map.shape_list->chunks, iterate_new_path, &shape_data);

        if(firstpath->pts[2] == NONE_FILLED) {
            DEBUG("NO PATHS FOUND\n");
            exit(ASSUMPTION_WRONG);
        }
        DEBUG("closing path\n");
        close_path(&map, output, firstpath);
        output->shapes->paths = firstpath; //wind back the paths
        
        //set the colour of the shape while prevent undefined behaviour
        NSVGpaint fillcopy = {
            shape_data.shapescolour->type,
            shape_data.shapescolour->color
        };
        output->shapes->fill = fillcopy;
        free(shape_data.shapescolour); //we held on to the dynamically allocated paint now we free it

        NSVGpaint stroke = {
            NSVG_PAINT_NONE,
            NSVG_RGB(0, 0, 0)
        };
        output->shapes->stroke = stroke;
        map.shape_list = map.shape_list->next; //go to next shape

        throw_on_max(&i);
        ++i;
        firstrun = false;
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
    NSVGimage* output = create_nsvgimage(input.width, input.height);

    DEBUG("generating chunkmap\n");
    chunkmap map = generate_chunkmap(input, options);

    //create set of shapes
    for(int map_x = 0; map_x < map.map_width; ++map_x)
    {
        for(int map_y = 0; map_y < map.map_height; ++map_y)
        {
            pixelchunk* currentchunk_p = &map.groups_array_2d[map_x][map_y];
            find_shapes(map, currentchunk_p, map_x, map_y, options.shape_colour_threshhold);
        }
    }
    DEBUG("chunk shapes found: %d\n", );
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
