
#include <stdio.h>
#include <math.h>
#include "types/colour.h"
#include <nanosvg.h>
#include "types/image.h"
#include "mapping.h"
#include "types/map.h"

void draw_corners(image input, NSVGimage* output) {

}

NSVGimage vectorize_image(image input, groupmap output, float variance_threshold) {
    NSVGimage output = {
        input.width,
        input.height,
        NULL
    };

    for (int map_x = 0; map_x < output.map_width; ++map_x)
    {
        for (int map_y = 0; map_y < output.map_height; ++map_y)
        {
            pixelgroup* group_p = &output.groups_array_2d[map_x][map_y];

            
        }
    }
}









