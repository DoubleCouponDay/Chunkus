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

    printf("Begin Generation of Node Map\n");
    if (options.chunk_size < 2)
        options.chunk_size = 2;
    
    node_map output;
    output.width = (int)ceilf((float)from.width / (float)options.chunk_size);
    output.height = (int)ceilf((float)from.height / (float)options.chunk_size);

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

            // Grab the node
            node *ptr = &output.nodes[x + y * output.width];

            // Assigned the edge case node dimensions
            int node_width = from.width - x * options.chunk_size;
            int node_height = from.height - y * options.chunk_size;
            
            // Check if not actually on the edge
            if (node_width > options.chunk_size)
                node_width = options.chunk_size;
            if (node_height > options.chunk_size)
                node_height = options.chunk_size;

            int count = node_width * node_height;

            // Full Node (Node does not go beyond the image)

            // // All the pixels that are in this node
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

            colorf average = { 0.f, 0.f, 0.f };
            int average_r = 0, average_g = 0, average_b = 0;
            // Calculate the average
            // for (int i = 0; i < node_width * node_height; ++i)
            // {
            //     average.r += node_data[i].r;
            //     average.g += node_data[i].g;
            //     average.b += node_data[i].b;
            // }

            //printf("Summing pixels\n");
            pixel min = { 255, 255, 255 }, max = { 0, 0, 0 };
            for (int x = 0; x < node_width; ++x)
            {
                for (int y = 0; y < node_height; ++y)
                {
                    pixel *p = &from.pixels[y_offset + y][x_offset + x];
                    //colorf as_color = convert_pixel_to_colorf(from.pixels[y_offset + y][x_offset + x]);
                    //average.r += as_color.r;
                    //average.g += as_color.g;
                    //average.b += as_color.b;
                    average_r += p->r;
                    average_g += p->g;
                    average_b += p->b;
                    if (p->r < min.r)
                        min.r = p->r;
                    if (p->g < min.g)
                        min.g = p->g;
                    if (p->b < min.b)
                        min.b = p->b;
                    if (p->r > max.r)
                        max.r = p->r;
                    if (p->g > max.g)
                        max.g = p->g;
                    if (p->b > max.b)
                        max.b = p->b;
                }
            }

            //printf("Finishing Averages\n");
            //average.r /= (float)count;
            //average.g /= (float)count;
            //average.b /= (float)count;
            pixel average_p = { (byte)((float)average_r / (float)count), (byte)((float)average_g / (float)count), (byte)((float)average_b / (float)count) };

            //printf("Assigning average\n");
            //ptr->color = convert_colorf_to_pixel(average);
            ptr->color = average_p;

            //printf("Gathering Relevant pixels to calculate Variance\n");
            // Gather the relevant pixels
            pixel *node_pixels = malloc(sizeof(pixel) * node_width * node_height);
            for (int x = 0; x < node_width; ++x)
            {
                for (int y = 0; y < node_height; ++y)
                {
                    node_pixels[x + y * node_width] = from.pixels[y_offset + y][x_offset + x];
                }
            }
            // Calculate the variance

            //printf("End iteration: assigning variance\n");
            ptr->variance = calculate_pixel_variance(node_pixels, node_width * node_height);

            if ((x == y && x % 20 == 0) || (x == 0 && y == 0) || (x == (output.width - 1) && y == (output.height - 1)))
            {
                printf("Node (%d, %d) variance: (%g, %g, %g), average: (%d, %d, %d), node_width: %d, node_height %d, min: %d, %d, %d, max: %d, %d, %d\n", x, y, ptr->variance.r, ptr->variance.g, ptr->variance.b, ptr->color.r, ptr->color.g, ptr->color.b, node_width, node_height, min.r, min.g, min.b, max.r, max.g, max.b);
            }
        }
    }

    return output;
}


node_variance calculate_pixel_variance(pixel *pixels, int num_pixels)
{
    if (num_pixels < 2)
        return (node_variance){0.f, 0.f, 0.f};

    double Kr = pixels[0].r;
    double Kg = pixels[0].g;
    double Kb = pixels[0].b;
    int n = 0;
    double Exr = 0.f, Ex2r = 0.f;
    double Exg = 0.f, Ex2g = 0.f;
    double Exb = 0.f, Ex2b = 0.f;

    for (int i = 0; i < num_pixels; ++i)
    {
        ++n;
        colorf col = convert_pixel_to_colorf(pixels[i]);
        Exr += (double)col.r - Kr;
        Exg += (double)col.g - Kg;
        Exb += (double)col.b - Kb;
        Ex2r += ((double)col.r - Kr) * ((double)col.r - Kr);
        Ex2g += ((double)col.g - Kg) * ((double)col.g - Kg);
        Ex2b += ((double)col.b - Kb) * ((double)col.b - Kb);
    }
    node_variance variance;
    variance.r = (float)((Ex2r - (Exr * Exr) / (double)n) / (double)(n - 1));
    variance.g = (float)((Ex2g - (Exg * Exg) / (double)n) / (double)(n - 1));
    variance.b = (float)((Ex2b - (Exb * Exb) / (double)n) / (double)(n - 1));
    if (fabsf(variance.r) < 0.000000001)
        variance.r = 0.f;
    if (fabsf(variance.g) < 0.000000001)
        variance.g = 0.f;
    if (fabsf(variance.b) < 0.000000001)
        variance.b = 0.f;
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