#include "dcdfiller.h"

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

void add_chunk_to_shape(std::shared_ptr<chunkshape>& shape, std::shared_ptr<pixelchunk>& item)
{
    if (!item->shape_in)
    {
        shape->chunks.emplace_back(item);
        item->shape_in = shape;
    }
}

void add_border_to_shape(std::shared_ptr<chunkshape>& shape, std::shared_ptr<pixelchunk>& item)
{
    if (!item->boundary_in)
    {
        shape->boundaries.emplace_back(item);
        item->boundary_in = shape;
    }
}

std::shared_ptr<chunkshape>& merge_shapes(chunkmap& map, std::shared_ptr<chunkshape>& first, std::shared_ptr<chunkshape>& second) {
    // Find smallest shape
    LOG_INFO("Merging Shapes");
    std::shared_ptr<chunkshape>& smaller = (first->chunks.size() < second->chunks.size() ? first : second);
    std::shared_ptr<chunkshape>& larger = (smaller == first ? second : first);

    // Replace every chunk's shape_chunk_in in second's shape holder to point to first
    for (auto& thing : smaller->chunks)
    {
        thing->shape_in = larger;
    }

    for (auto& thing : smaller->boundaries)
    {
        thing->boundary_in = larger;
    }

    // Append smaller shape's chunks and borders onto larger's
    
    larger->chunks.splice(larger->chunks.end(), std::move(smaller->chunks));
    
    larger->boundaries.splice(larger->boundaries.end(), std::move(smaller->boundaries));

    // Remove smaller from the chunkshape list
    map.shape_list.remove(smaller);
    return larger;
}

void enlarge_border(chunkmap& map, std::shared_ptr<pixelchunk> current, std::shared_ptr<chunkshape>& currentinshape, std::shared_ptr<chunkshape>& adjacentinshape) {
    std::shared_ptr<chunkshape> chosenshape;

    if(currentinshape) { //set shape for boundary manipulation
        chosenshape = currentinshape;
    }

    else { //current is not in a shape
        chosenshape = std::make_shared<chunkshape>();
        map.shape_list.emplace_back(chosenshape);
    }
    
    
    add_border_to_shape(chosenshape, current);
    add_chunk_to_shape(chosenshape, current);
    chosenshape->colour = current->average_colour;
}

void enlarge_shape(chunkmap& map, std::shared_ptr<pixelchunk>& current, std::shared_ptr<chunkshape>& currentinshape, std::shared_ptr<chunkshape>& adjacentinshape, std::shared_ptr<pixelchunk>& adjacent) {
    std::shared_ptr<chunkshape> chosenshape;

    if(!currentinshape && !adjacentinshape) {
        chosenshape = std::make_shared<chunkshape>();
        map.shape_list.emplace_back(chosenshape);

        add_chunk_to_shape(chosenshape, current);
        add_chunk_to_shape(chosenshape, adjacent);
    }

    else if (currentinshape && !adjacentinshape)
    {
        chosenshape = currentinshape;
        add_chunk_to_shape(currentinshape, adjacent);
    }

    else if(!currentinshape && adjacentinshape)
    {
        chosenshape = adjacentinshape;
        add_chunk_to_shape(adjacentinshape, current);
    }

    else if(currentinshape == adjacentinshape) {
        chosenshape = currentinshape;
    }

    else { // Merge the two shapes        
        chosenshape = merge_shapes(map, currentinshape, adjacentinshape);
    }
    chosenshape->colour = current->average_colour;
}

//welcome to the meat and potatoes of the program!
void find_shapes(chunkmap& map, int map_x, int map_y, float shape_colour_threshold) {
    std::shared_ptr<pixelchunk>& current = map.get(map_x, map_y);
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
                adjacent_index_x >= map.width() ||  
                adjacent_index_y >= map.height())
                continue;

            auto adjacent = map.get(adjacent_index_x, adjacent_index_y);
            auto current_shape = current->shape_in;
            auto adjacent_shape = adjacent->shape_in;

            if (current->average_colour.is_similar_to(adjacent->average_colour, shape_colour_threshold)) {
                if(map_x == 0 || map_x == (map.width() - 1) ||
                    map_y == 0 || map_y == (map.height() - 1)) 
                {
                    enlarge_border(map, current, current_shape, adjacent_shape);
                }
                enlarge_shape(map, current, current_shape, adjacent_shape, adjacent);
            }

            else {
                enlarge_border(map, current, current_shape, adjacent_shape);
            }
        }
    }
}

void fill_chunkmap(chunkmap& map, const vectorize_options& options) {
    //create set of shapes
    LOG_INFO("Fill chunkmap with threshold: %f", options.shape_colour_threshhold);
    int tenth_of_map = (int)floorf(map.width() * map.height() / 10.f);
    int count = 0;
    int tenth_count = 0;

    for(int map_x = 0; map_x < map.width(); ++map_x)
    {
        for(int map_y = 0; map_y < map.height(); ++map_y)
        {
            ++count;
            if (tenth_of_map > 0 && count % tenth_of_map == 0)
            {
                ++tenth_count;
                LOG_INFO("Progress: %d0%%", tenth_count);
            }
            find_shapes(map, map_x, map_y, options.shape_colour_threshhold);

            if (isBadError())
            {
                LOG_INFO("find_shapes failed with code: %d", getLastError());
                return;
            }
        }
    }
}
