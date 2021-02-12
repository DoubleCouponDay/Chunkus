#include "usage_speed.h"
#include "utility/logger.h"
#include "imagefile/pngfile.h"

#include <stdlib.h>
#include <stdio.h>

void find_shapes_speed(chunkmap* map, float threshold)
{
    if (map->map_width < 1 || map->map_height < 1)
    {
        LOG_ERR("Can not process empty chunkmap");
        return;
    }

    LOG_INFO("Find Shapes Speedy with threshold: %.1f", threshold);
    int* shape_ints = calloc(map->map_width * map->map_height, sizeof(int));
    int next_shape_int = 0;

    for (int y = 0; y < map->map_height; ++y)
    {
        for (int x = 0; x < map->map_width; ++x)
        {
            int current = x + y * map->map_width;
            int x_in_range = (x + 1 < map->map_width);
            int y_in_range = (y + 1 < map->map_height);
            int capped_x = (x_in_range * 2 + (!x_in_range) * 1);
            int capped_y = (y_in_range * 2 + (!y_in_range) * 1);
            for (int adj_x = 0 - (x > 0) * 1; adj_x < capped_x; ++adj_x)
            {
                for (int adj_y = 0 - (y > 0) * 1; adj_y < capped_y; ++adj_y)
                {
                    int similarity = colours_are_similar(map->groups_array_2d[x][y].average_colour, map->groups_array_2d[x+adj_x][y+adj_y].average_colour, threshold);
                    int val = (((adj_x != 0) | (adj_y != 0)) * similarity * shape_ints[current + adj_x + adj_y * map->map_width]);
                    shape_ints[current] = (!val * shape_ints[current]) + val;
                }
            }

            if (!shape_ints[current])
                LOG_INFO("Creating new Shape" );

            next_shape_int += 1 * (!shape_ints[current]); // Goes up every time a chunk has no similar neighbours with chunks
            int val = next_shape_int * (!shape_ints[current]);
            shape_ints[current] = (!val * shape_ints[current]) + val; // Assigns the incremented next_shape_int to this pixelchunk if it had no similar neighbours
        }
    }

    for (int y = 0; y < map->map_height; ++y)
    {
        for (int x = 0; x < map->map_width; ++x)
        {
            int current = x + y * map->map_width;
            --shape_ints[current];
        }
    }

    int num_shapes = next_shape_int;
    LOG_INFO("Num Shapes calculated to: %d", num_shapes);

    int* shape_counts = calloc(num_shapes * 3 + map->map_width * map->map_height, sizeof(int));
    int* increasing_shape_indices = shape_counts + num_shapes;
    int* shape_indices = increasing_shape_indices + num_shapes;
    int* shapes = shape_indices + num_shapes;

    for (int y = 0; y < map->map_height; ++y)
    {
        for (int x = 0; x < map->map_width; ++x)
        {
            int current = x + y * map->map_width;
            int index = shape_ints[current];
            ++shape_counts[index];
        }
    }

    int total_found = 0;
    for (int i = 0; i < num_shapes; ++i)
    {
        total_found += shape_counts[i];
    }

    LOG_INFO("Total Count: %d", total_found);

    memcpy(increasing_shape_indices + 1, shape_counts, (num_shapes - 1) * sizeof(int));
    memcpy(shape_indices + 1, shape_counts, (num_shapes - 1) * sizeof(int));

    for (int y = 0; y < map->map_height; ++y)
    {
        for (int x = 0; x < map->map_width; ++x)
        {
            int current = x + y * map->map_width;
            shapes[increasing_shape_indices[shape_ints[current]]++] = current;
        }
    }

    // Now convert this ridiculousness into actual shapes
    chunkshape* actual_shapes = calloc(num_shapes, sizeof(chunkshape));

    // for (int i = 0; i < num_shapes; ++i)
    // {
    //     actual_shapes[i].chunks = hashmap_new(sizeof(pixelchunk), 0, 0, 0, chunk_hash, chunk_compare, 0);
    //     LOG_INFO("Shape %d has %d chunks", i, shape_counts[i]);
    //     for (int j = shape_pps_unmodified[i]; j < shape_pps_unmodified[i] + shape_counts[i]; ++j)
    //     {
    //         int index = shapes[j];
    //         hashmap_set(actual_shapes[i].chunks, &map->groups_array_2d[index % map->map_width][index / map->map_width]);
    //     }
    // }

    /// WRITE PNG SHIT DOWN HERE
    {
        colour* colours = calloc(map->map_width * map->map_height, sizeof(colour));
        colour black = {0};

        const colour stack_colours[] = {{0xaa, 0x14, 0x14}, {0x14, 0xaa, 0x14}, {0x14, 0x14, 0xaa}, {0xaa, 0xaa, 0x14}, {0xaa, 0x14, 0xaa}, {0x14, 0xaa, 0xaa}, {0xaa, 0xaa, 0xaa}};

        colourmap intermediate = {
            colours,
            map->map_width,
            map->map_height};

        LOG_INFO("First colour in colourmap %s black", (memcmp(colours, &black, sizeof(colour)) ? "is not" : "is"));

        pixel color_check = {255};
        for (int i = 0; i < num_shapes; ++i)
        {
            LOG_INFO("Shape %d has %d chunks", i, shape_counts[i]);
            pixel color = map->groups_array_2d[shapes[shape_indices[i]] % map->map_width][shapes[shape_indices[i]] / map->map_width].average_colour;
            LOG_INFO("Shape %d has color (%d, %d, %d)", i, color.r, color.g, color.b);
            for (int j = shape_indices[i]; j < shape_indices[i] + shape_counts[i]; ++j)
            {
                int index = shapes[j];
                pixelchunk *chunga = &map->groups_array_2d[index % map->map_width][index / map->map_width];
                coordinate location = chunga->location;
                colour *currentcolour = &intermediate.colours[location.x + location.y * intermediate.width];
                //currentcolour->r = chunga->average_colour.r;
                //currentcolour->g = chunga->average_colour.g;
                //currentcolour->b = chunga->average_colour.b;
                *currentcolour = stack_colours[i % 7];
            }
        }

        image output_img = create_image(intermediate.width * 3, intermediate.height * 3);

        for (int x = 0; x < intermediate.width; ++x)
        {
            for (int y = 0; y < intermediate.height; ++y)
            {
                colour *bob = &intermediate.colours[x + intermediate.width * y];
                for (int xx = 0; xx < 3; ++xx)
                {
                    for (int yy = 0; yy < 3; ++yy)
                    {
                        pixel *img_pix = &(output_img.pixels_array_2d[x * 3 + xx][y * 3 + yy]);
                        img_pix->r = bob->r;
                        img_pix->g = bob->g;
                        img_pix->b = bob->b;
                    }
                }
            }
        }

        write_image_to_png(output_img, "output.png");

        free_image_contents(output_img);
        free(intermediate.colours);
    }
    // WRITE PNG SHIT STOPPED
    

    free(shape_ints);
    free(shape_counts);

    chunkshape* tmp = map->shape_list;
    map->shape_list = actual_shapes;

    map->shape_list = tmp;
    free(actual_shapes);
}


NSVGimage* vectorize_image_speed(image input, vectorize_options options)
{
    open_log("joe mama.txt");
    chunkmap* map = generate_chunkmap(input, options);

    find_shapes_speed(map, options.shape_colour_threshhold);

    close_log();

    return NULL;
}
