#include "chunkmap.h"

#include <stdlib.h>
#include <math.h>
#include <errno.h>
#include <string.h>

#include "image.h"
#include "utility/error.h"
#include "utility/one_d.h"
#include "imagefile/pngfile.h"
#include "utility/logger.h"

void chunkmap::iterate_pixels(const image& input, size_t chunk_size) {
    for (int x = 0; x < (int)width(); ++x)
    {
        for (int y = 0; y < (int)height(); ++y)
        {
            int x_offset = x * chunk_size;
            int y_offset = y * chunk_size;

            // Grab the pixelchunk
            pixelchunk& chunk = *(groups[x][y] = std::make_shared<pixelchunk>(coordinate{ x, y }));

            // Assigned the edge case pixelchunk dimensions
            size_t node_width = input.width() - (size_t)x * chunk_size;
            size_t node_height = input.height() - (size_t)y * chunk_size;

            // Check if not actually on the edge
            if (node_width > chunk_size)
                node_width = chunk_size;

            if (node_height > chunk_size)
                node_height = chunk_size;
                
            int count = node_width * node_height;

            // Calculate the average of all these pixels
            pixelInt average_i{};

            for (int x = 0; x < node_width; ++x)
            {
                for (int y = 0; y < node_height; ++y)
                {
                    const pixel& current_pixel = input.get(x_offset + x, y_offset + y);

                    average_i += current_pixel;
                }
            }

            pixel average_p = {
                (byte)((float)average_i.R / (float)count),
                (byte)((float)average_i.G / (float)count),
                (byte)((float)average_i.B / (float)count)
                };
            chunk.average_colour = average_p;
        }
    }
}

chunkmap::chunkmap() : groups(), shape_list(0)
{
}

chunkmap::chunkmap(const image& image, int chunk_size) : groups(), shape_list()
{
    int width = (int)ceilf((float)image.width() / (float)chunk_size);
    int height = (int)ceilf((float)image.height() / (float)chunk_size);

    groups.resize(width, std::vector<std::shared_ptr<pixelchunk>>(height));

    iterate_pixels(image, chunk_size);
}

chunkmap& chunkmap::operator=(chunkmap &&other)
{ 
    groups = std::move(other.groups); 
    shape_list.clear(); 
    shape_list = other.shape_list; 
    other.groups.clear(); 
    other.shape_list.clear(); 
    return *this; 
}


bool chunkmap::chunks_to_file(char *file) const
{
    LOG_INFO("Writing Chunkmap (%d x %d) to '%s'", width(), height(), file);
    image im{width(), height()};

    for (int x = 0; x < width(); ++x)
    {
        for (int y = 0; y < height(); ++y)
        {
            im.set(x, y, get(x, y)->average_colour);
        }
    }

    return im.to_png(file);
}

bool chunkmap::shapes_to_file(char *file) const
{
    write_chunkmap_to_png(*this, file);

    return getAndResetErrorCode() != SUCCESS_CODE;
}

bool chunkmap::borders_to_file(char *file) const
{
    LOG_INFO("Writing Chunkmap (%d x %d) borders to '%s'", width(), height(), file);
    image im{width(), height()};

    for (auto& s : shape_list)
    {
        for (auto& border : s->boundaries)
        {
            im.set(border->location.x, border->location.y, border->average_colour);
        }
    }
    
    return im.to_png(file);
}