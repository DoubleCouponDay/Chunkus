#include "vectorize.h"

#include <math.h>
#include <our_math.h>

#ifndef NULL
#define NULL 0
#endif


node_map generate_node_map(image from, node_map_options options)
{
    if (from.width < 1 || from.height < 1)
        return (node_map){ NULL, 0, 0 };

    if (options.chunk_size < 3)
        options.chunk_size = 3;
    
    node_map output;
    output.width = ceilf(from.width / (float)options.chunk_size);
    output.height = ceilf(from.height / (float)options.chunk_size);

    output.nodes = malloc(sizeof(node) * output.width * output.height);

    for (int x = 0; x < output.width; ++x)
    {
        for (int y = 0; y < output.height; ++y)
        {
            int node_width = options.chunk_size;
            int node_height = options.chunk_size;
            if (fmodf(from.width, options.chunk_size) != 0.f)
            {
                // Potentially empty Node (Node area extends beyond the image)
                node_width = from.width - x * options.chunk_size;
                node_height = from.height - y * options.chunk_size;
                if (node_width < 1 || node_height < 1)
                    continue;
            }
            // Full Node (Node does not go beyond the image)

            Color *node_data = malloc(sizeof(Color) * node_width * node_height);
            for (int i = 0; i < node_width; ++i)
            {
                for (int j = 0; j < node_height; ++i)
                {
                    node_data[i + node_width * j].r = from.pixels[x * options.chunk_size + i + (y + j) * options.chunk_size * from.width].r;
                    node_data[i + node_width * j].g = from.pixels[x * options.chunk_size + i + (y + j) * options.chunk_size * from.width].g;
                    node_data[i + node_width * j].b = from.pixels[x * options.chunk_size + i + (y + j) * options.chunk_size * from.width].b;
                }
            }

            Color average;
            average.r = average.g = average.b = 0.f;
            // Calculate the average
            for (int i = 0; i < node_width * node_height; ++i)
            {
                average.r += node_data[i].r;
                average.g += node_data[i].g;
                average.b += node_data[i].b;
            }
            average.r /= (float)(node_width * node_height);
            average.g /= (float)(node_width * node_height);
            average.b /= (float)(node_width * node_height);

            output.nodes[x + y * output.width].color = average;

            // Calculate the variance
            output.nodes[x + y * output.width].variance = calculate_color_variance(node_data, node_width * node_height);
        }
    }

    return output;
}


node_variance calculate_color_variance(Color *colors, int num_colors)
{
    if (num_colors < 2)
        return (Color){0.f, 0.f, 0.f};

    float Kr = colors[0].r;
    float Kg = colors[0].g;
    float Kb = colors[0].b;
    int n = 0;
    float Exr = 0.f, Ex2r = 0.f;
    float Exg = 0.f, Ex2g = 0.f;
    float Exb = 0.f, Ex2b = 0.f;

    for (int i = 0; i < num_colors; ++i)
    {
        ++n;
        Exr += colors[i].r - Kr;
        Exg += colors[i].g - Kg;
        Exb += colors[i].b - Kb;
        Ex2r += (colors[i].r - Kr) * (colors[i].r - Kr);
        Ex2g += (colors[i].g - Kg) * (colors[i].g - Kg);
        Ex2b += (colors[i].b - Kb) * (colors[i].b - Kb);
    }
    node_variance variance;
    variance.r = (Ex2r - (Exr * Exr) / (float)n) / (float)(n - 1);
    variance.g = (Ex2g - (Exg * Exg) / (float)n) / (float)(n - 1);
    variance.b = (Ex2b - (Exb * Exb) / (float)n) / (float)(n - 1);
    return variance;
}