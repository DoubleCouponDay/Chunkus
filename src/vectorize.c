#include "vectorize.h"

#include <math.h>
#include <stdlib.h>

#include "tools.h"

#ifndef NULL
#define NULL 0
#endif


node_map generate_node_map(image from, node_map_options options)
{
    if (from.width < 1 || from.height < 1 || !from.pixels)
        return (node_map){ NULL, 0, 0 };

    printf("Generating Node Map\n");
    if (options.chunk_size < 3)
        options.chunk_size = 3;
    
    node_map output;
    output.width = ceilf(from.width / (float)options.chunk_size);
    output.height = ceilf(from.height / (float)options.chunk_size);

    printf("Allocating Node map with %dx%d\n", output.width, output.height);
    output.nodes = malloc(sizeof(node) * output.width * output.height);

    printf("Iterating through the nodes\n");
    for (int x = 0; x < output.width; ++x)
    {
        int x_offset = x * options.chunk_size;
        for (int y = 0; y < output.height; ++y)
        {
            //printf("Begin iteration\n");
            int y_offset = y * options.chunk_size;

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

            // // All the colors that are in this node
            // color **node_data = malloc(sizeof(color) * node_width * node_height);
            // for (int i = 0; i < node_width; ++i)
            // {
            //     for (int j = 0; j < node_height; ++i)
            //     {
            //         node_data[i + node_width * j].r = from.pixels[x * options.chunk_size + i + (y + j) * options.chunk_size * from.width].r;
            //         node_data[i + node_width * j].g = from.pixels[x * options.chunk_size + i + (y + j) * options.chunk_size * from.width].g;
            //         node_data[i + node_width * j].b = from.pixels[x * options.chunk_size + i + (y + j) * options.chunk_size * from.width].b;
            //     }
            // }

            color average;
            average.r = average.g = average.b = 0.f;
            // Calculate the average
            // for (int i = 0; i < node_width * node_height; ++i)
            // {
            //     average.r += node_data[i].r;
            //     average.g += node_data[i].g;
            //     average.b += node_data[i].b;
            // }

            //printf("Summing pixels\n");
            for (int x = 0; x < node_width; ++x)
            {
                for (int y = 0; y < node_height; ++y)
                {
                    average.r += from.pixels[y_offset + y][x_offset + x].r;
                    average.g += from.pixels[y_offset + y][x_offset + x].g;
                    average.b += from.pixels[y_offset + y][x_offset + x].b;
                }
            }

            //printf("Finishing Averages\n");
            average.r /= (float)(node_width * node_height);
            average.g /= (float)(node_width * node_height);
            average.b /= (float)(node_width * node_height);

            //printf("Assigning average\n");
            output.nodes[x + y * output.width].color = average;

            //printf("Gathering Relevant colors to calculate Variance\n");
            // Gather the relevant colors
            color *node_colors = malloc(sizeof(color) * node_width * node_height);
            for (int x = 0; x < node_width; ++x)
            {
                for (int y = 0; y < node_height; ++y)
                {
                    node_colors[x + y * node_width] = from.pixels[y_offset + y][x_offset + x];
                }
            }
            // Calculate the variance

            //printf("End iteration: assigning variance\n");
            output.nodes[x + y * output.width].variance = calculate_color_variance(node_colors, node_width * node_height);
        }
    }

    return output;
}


node_variance calculate_color_variance(color *colors, int num_colors)
{
    if (num_colors < 2)
        return (color){0.f, 0.f, 0.f};

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

float shifted_data_variance(float *data, int data_len)
{
    if (data_len < 2)
        return 0.f;
    
    float K = data[0];
    int n = 0;
    float Ex = 0.f, Ex2 = 0.f;

    for (int i = 0; i < data_len; ++i)
    {
        ++n;
        Ex += data[i] - K;
        Ex2 += (data[i] - K) * (data[i] - K);
    }
    float variance = (Ex2 - (Ex * Ex) / (float)n) / (float)(n - 1);

    return variance;
}