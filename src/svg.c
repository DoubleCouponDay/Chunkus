
#include <stdio.h>
#include <math.h>
#include "types/colour.h"
#include <nanosvg.h>
#include "types/image.h"
#include "mapping.h"
#include "types/map.h"

void iterate_neighbours(int map_x, int map_y, groupmap input, pixelgroup* group) {
    for (int neighbour_x = -1; neighbour_x < 2; ++neighbour_x)
    {
        // Out of bounds check
        if (map_x + neighbour_x >= input.map_width)
            continue;
        for (int neighbour_y = -1; neighbour_y < 2; ++neighbour_y)
        {
            // Out of bounds check
            if (map_y + neighbour_y >= input.map_height || (map_x == 0 && map_y == 0))
                continue;
            
            int neighbourindex = map_x + neighbour_x + input.map_width * (map_y + neighbour_y);
            pixelgroup* neighbour = &input.nodes[neighbourindex];

            pixelF neighbourF = {
                neighbour->average_colour.r,
                neighbour->average_colour.g,
                neighbour->average_colour.b
            };

            pixelF groupF = {
                group->average_colour.r,
                group->average_colour.g,
                group->average_colour.b
            };

            if (pixelf_equal(neighbourF, groupF)) {
                
            }
        }
    }
}

void vectorize_Image_Group(image input, groupmap output, float variance_threshold)
{
    // Make 'background' path
    // TODO: make background path

    for (int map_x = 0; map_x < output.map_width; ++map_x)
    {
        for (int map_y = 0; map_y < output.map_height; ++map_y)
        {
            // Find high variance groups
            pixelgroup *group = &output.nodes[map_x + output.map_width * map_y];
            
        }
    }
}









