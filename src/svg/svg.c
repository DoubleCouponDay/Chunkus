
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdarg.h>
#include <nanosvg.h>
#include <stdbool.h>

#include "../types/colour.h"
#include "../types/image.h"
#include "../mapping.h"
#include "../../test/tools.h"
#include "nanocopy.h"

const int POINTS_LENGTH = 8;
const int BEZIER_POINTS = 2;
const int BOUNDS_LENGTH = 4;
const char* TEMPLATE_PATH = "../template.svg";

void fill_char_array(char* input, char* output) {
    strcpy(output, input);
}

void fill_float_array(float* input, int input_length, float* output, int output_length) {
    if(input_length > output_length) {
        DEBUG("output array is too small for your input.");
        int crash[1];
        crash[1];
    }

    for(int i = 0; i < input_length; ++i) {
        output[i] = input[i];
    }
}

void fill_pts_array(float* array,
                    int array_length,
                    float x1, float y1, 
                    float x2, float y2, 
                    float control_x1, float control_y1, 
                    float control_x2, float control_y2)
{
    if(array_length != POINTS_LENGTH) {
        DEBUG("points array must be 8 long.");
        int crash[1];
        crash[1];
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
    fill_pts_array(output->pts, POINTS_LENGTH, start.x, start.y, end.x, end.y, 0, 0, 1, 1); //draw the top side of a box
    float boundingbox[4] = { 0, 0, input.width, input.height };
    fill_float_array(boundingbox, BOUNDS_LENGTH, output->bounds, BOUNDS_LENGTH);
    return output;
}

/// can be null
NSVGshape* draw_corners(image input) {
    int r = input.topleftcorner_p->r;
    int g = input.topleftcorner_p->g;
    int b = input.topleftcorner_p->b;

    char* topleft_type = rgb_to_string(input.topleftcorner_p);
    char* topright_type = rgb_to_string(input.toprightcorner_p);
    char* bottomleft_type = rgb_to_string(input.bottomleftcorner_p);
    char* bottomright_type = rgb_to_string(input.bottomrightcorner_p);

    if(topleft_type != topright_type ||
        topleft_type != bottomleft_type ||
        topleft_type != bottomright_type) {
        return NULL;
    }

    NSVGpaint newpaint = {
        NSVG_PAINT_COLOR,
        NSVG_RGB(r, g, b)
	};

    NSVGpaint newstroke = {
        NSVG_PAINT_NONE,
        NSVG_RGB(0, 0, 0)
	};

    NSVGshape* output = calloc(1, sizeof(NSVGshape));
    fill_char_array("border", output->id); //id of element
    output->fill = newpaint; // Fill paint
    output->stroke = newstroke; // Stroke paint

    // Top Path
    coordinate topstart = {0, 0};
    coordinate topend = {input.width, 0};
    NSVGpath* toppath = create_path(input, topstart, topend);

    // right Path
    coordinate rightstart = {input.width, 0};
    coordinate rightend = {input.width, input.height};
    NSVGpath* rightpath = create_path(input, rightstart, rightend);
    toppath->next = rightpath;

    // bottom Path
    coordinate bottomstart = {input.width, input.height};
    coordinate bottomend = {0, input.height};
    NSVGpath* bottompath = create_path(input, bottomstart, bottomend);
    rightpath->next = bottompath;
    
    // Left Path
    coordinate leftstart = {0, 0};
    coordinate leftend = {input.width, 0};
    NSVGpath* leftpath = create_path(input, topstart, topend);
    bottompath->next = leftpath;

    leftpath->next = toppath;
    output->paths = toppath;

    float* bounds[4] = { 0, 0, input.width, input.height };
    fill_float_array(bounds, BOUNDS_LENGTH, output->bounds, BOUNDS_LENGTH);

    return output;
}

void* add_boundary_path() {

}

bool colours_are_similar(pixel color_a, pixel color_b, float max_distance)
{
    pixel diff;
    diff.r = color_a.r - color_b.r;
    diff.g = color_a.g - color_b.g;
    diff.b = color_a.b - color_b.b;

    float mag = sqrt(diff.r * diff.r + diff.g * diff.g + diff.b * diff.b);

    return mag <= max_distance;
}

void scan_neighbours(chunkmap map, pixelchunk* current, int map_x, int map_y, float shape_colour_threshold)
{
    for (int adjacent_x = -1; adjacent_x < 2; ++adjacent_x)
    {
        for (int adjacent_y = -1; adjacent_y < 2; ++adjacent_y)
        {
            if (adjacent_x == 0 && adjacent_y == 0)
                continue;
            
            int adjacent_index_x = map_x + adjacent_x;
            int adjacent_index_y = map_y + adjacent_y;

            if (adjacent_index_x < 0 || adjacent_index_x >= map.map_width
                || adjacent_index_y < 0 || adjacent_index_y >= map.map_height)
                continue;

            pixelchunk* adjacent = &map.groups_array_2d[map_x + adjacent_x][map_y + adjacent_y];

            if (colours_are_similar(current->average_colour, adjacent->average_colour, shape_colour_threshold))
            {
                add_boundary_path();
            }
        }
    }
}

points* add_boundary_point(points* points_list, pixelchunk* currentgroup_p) {
    points_list->current = currentgroup_p->location;
    points_list->next = calloc(1, sizeof(points));
    return points_list->next;
}

NSVGimage* vectorize_image(image input, chunkmap map, float variance_threshhold, float shape_colour_threshhold) {
    NSVGimage* output = parsetemplate(TEMPLATE_PATH);
    output->width = input.width;
    output->height = input.height;
    NSVGshape* border = draw_corners(input);

    points* points_list = calloc(1, sizeof(points));

    if(border) { //border has a consistent colour so place it as first object
        output->shapes = border;
    }

    for (int map_x = 0; map_x < map.map_width; ++map_x)
    {
        for (int map_y = 0; map_y < map.map_height; ++map_y)
        {
            pixelchunk* currentgroup_p = &map.groups_array_2d[map_x][map_y];

            //add a boundary point
            if(currentgroup_p->variance.r > variance_threshhold ||
                currentgroup_p->variance.g > variance_threshhold ||
                currentgroup_p->variance.b > variance_threshhold) {
                points_list = add_boundary_point(points_list, currentgroup_p);
            }

            // Scan neighbours that look similar and add to the list
            scan_neighbours();
        }
    }
    
    //wind back the linked list to the start
    while(points_list->previous != NULL) {
        points_list = points_list->previous;
    }

    //free all points
    while(points_list->next != NULL) {
        points_list = points_list->next;
        free(points_list->previous);        
    }
    return output;
}

void free_image(NSVGimage* input) {
    if(!input) {
        DEBUG("input is null");
        int crash[1];
        crash[1];
    }

    NSVGshape* currentlink = input->shapes->next;

    while(currentlink) {
        NSVGpath* currentpath = currentlink->paths;

        while(currentpath) {
            free(currentpath);
            currentpath = currentlink->paths;
        }        
        
        free(currentlink);
        currentlink = input->shapes->next;
    }
    free(input);
}
