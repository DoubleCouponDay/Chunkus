
#include <stdio.h>
#include <math.h>
#include "types/colour.h"
#include <nanosvg.h>
#include "types/image.h"
#include "mapping.h"

void iterate_neighbours(int map_x, int map_y, group_map group_input, pixelgroup* group) {
    for (int neighbour_x = -1; neighbour_x < 2; ++neighbour_x)
    {
        // Out of bounds check
        if (map_x + neighbour_x >= group_input.width)
            continue;
        for (int neighbour_y = -1; neighbour_y < 2; ++neighbour_y)
        {
            // Out of bounds check
            if (map_y + neighbour_y >= group_input.height || (map_x == 0 && map_y == 0))
                continue;
            
            int neighbourindex = map_x + neighbour_x + group_input.width * (map_y + neighbour_y);
            pixelgroup* neighbour = &group_input.nodes[neighbourindex];

            pixelF neighbourF = {
                neighbour->color.r,
                neighbour->color.g,
                neighbour->color.b
            };

            pixelF groupF = {
                group->color.r,
                group->color.g,
                group->color.b
            };

            if (pixelf_equal(neighbourF, groupF)) {
                
            }
        }
    }
}

void vectorize_Image_Group(image input, group_map group_input, float variance_threshold)
{
    // Make 'background' path

    // TODO: make background path

    for (int map_x = 0; map_x < group_input.width; ++map_x)
    {
        for (int map_y = 0; map_y < group_input.height; ++map_y)
        {
            // Find high variance groups
            pixelgroup *group = &group_input.nodes[map_x + group_input.width * map_y];
            
            float summed_variance = group->variance.r + group->variance.g + group->variance.b;

            if (summed_variance > variance_threshold)
            {
                // Eligible group
                // Search neighbours
                
                
            }
        }
    }
}









