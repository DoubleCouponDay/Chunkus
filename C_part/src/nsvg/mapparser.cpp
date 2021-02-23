#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <nanosvg.h>

#include "usage.h"
#include "../image.h"
#include "../chunkmap.h"
#include "utility/logger.h"
#include "../utility/error.h"
#include "copy.h"
#include "mapping.h"
#include "../sort.h"



void parse_map_into_nsvgimage(const chunkmap& map, nsvg_ptr& output)
{
    //create the svg
    if(map.shape_list.empty()) {
        LOG_ERR("no shapes given to mapparser");
        setError(ASSUMPTION_WRONG);
        return;
    }
    
    LOG_INFO("Parsing %u Shapes", map.shape_list.size());

    NSVGshape* end = nullptr;

    for (auto& shape : map.shape_list)
    {
        if (shape->boundaries.size() < 2)
            continue;
        
        auto new_piece = create_shape(output->width, output->height, shape->colour);
        if (end)
        {
            end->next = new_piece;
            end = new_piece;
        }
        else
        {
            output->shapes = end = new_piece;
        }

        NSVGpath *end_path = nullptr;
        coordinate previous_loc;
        for (auto iter = shape->boundaries.begin(); iter != shape->boundaries.end(); ++iter)
        {
            if (iter == shape->boundaries.begin())
            {
                previous_loc = (*iter)->location;
                continue;
            }
            
            auto new_path = create_path(output->width, output->height, previous_loc, (*iter)->location);

            if (end_path)
            {
                end_path->next = new_path;
                end_path = new_path;
            }
            else
            {
                end_path = end->paths = new_path;
            }
            previous_loc = (*iter)->location;
        }

        auto closing_path = create_path(output->width, output->height, shape->boundaries.back()->location, shape->boundaries.front()->location);
        end_path->next = closing_path;
    }
}
