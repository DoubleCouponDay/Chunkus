#include "usage_speed.h"

#include "hashmap/usage.h"
#include "utility/logger.h"
#include "utility/vec.h"
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
    int* shape_offsets = increasing_shape_indices + num_shapes; // An array containing the index of the first chunk of each shape in the shapes array
    int* chunk_index_of = shape_offsets + num_shapes; // Stores indices into the chunkmap 

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
    if (total_found != map->map_width * map->map_height)
        LOG_WARN("Total Count was not %u (map w x h)! Something is wrong...", map->map_width * map->map_height);
    
    {
        int running_index = 0;
        for (int i = 0; i < num_shapes; ++i)
        {
            increasing_shape_indices[i] = running_index;
            shape_offsets[i] = running_index;
            running_index += shape_counts[i];
        }
    }

    for (int y = 0; y < map->map_height; ++y)
    {
        for (int x = 0; x < map->map_width; ++x)
        {
            int current_chunk = x + y * map->map_width;
            int* p = &increasing_shape_indices[shape_ints[current_chunk]];
            chunk_index_of[*p] = current_chunk;
            *p += 1;
        }
    }

    {
        unsigned char* checked_indices = calloc(map->map_width * map->map_height, sizeof(unsigned char));
        int did = 0;
        for (int i = 0; i < num_shapes; ++i)
        {
            for (int j = 0; j < shape_counts[i]; ++j)
            {
                if (checked_indices[chunk_index_of[j + shape_offsets[i]]])
                {
                    LOG_WARN("Shape %d has a duplicate index of: %d", j + shape_offsets[i]);
                    did = 1;
                }
                checked_indices[chunk_index_of[j + shape_offsets[i]]] = 1;
            }
        }
        if (!did)
            LOG_INFO("No Found Duplicate indices");
        free(checked_indices);
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

    
    LOG_INFO("Beginning Border Detection");
    // Border Detection
    int *border_counts = calloc(num_shapes * 3, sizeof(int));
    int *border_offsets = border_counts + num_shapes;
    int *border_running_indices = border_offsets + num_shapes;
    char *border_bits = calloc(map->map_width * map->map_height, sizeof(char));
    int border_total = 0;
    for (int i = 0; i < num_shapes; ++i)
    {
        for (int j = 0; j < shape_counts[i]; ++j)
        {
            // Look through all coordinates
            // See if any adjacents either: don't exist (edge of the image) or are a different colour
            int current = chunk_index_of[j + shape_offsets[i]];
            int x = current % map->map_width;
            int y = current / map->map_width;
            int edge = (x < 1) | (y < 1) | (x + 1 >= map->map_width) | (y + 1 >= map->map_height);
            int border = edge;
            for (int adj_x = (2 * edge) + (-1 * !edge); adj_x < 2; ++adj_x)
            {
                for (int adj_y = (2 * edge) + (-1 * !edge); adj_y < 2; ++adj_y)
                {
                    border |= shape_ints[current] != shape_ints[x + adj_x + (y + adj_y) * map->map_width];
                }
            }

            border_total += 1 * border;
            border_counts[i] += 1 * border;
            border_bits[current] = (char)border;
        }
    }

    // Initialize Offsets
    {
        int running_offset = 0;
        for (int i = 0; i < num_shapes; ++i)
        {
            border_offsets[i] = running_offset;
            border_running_indices[i] = running_offset;
            running_offset += border_counts[i];
        }
    }

    LOG_INFO("Border count calculated as: %d", border_total);
    int *border_chunk_indices = calloc(border_total, sizeof(int));
    int iterations = 0;
    for (int i = 0; i < num_shapes; ++i)
    {
        int first_chunk = chunk_index_of[shape_offsets[i]];
        int current_chunk = first_chunk;
        int x = current_chunk % map->map_width;
        int y = current_chunk / map->map_width;
        vector2 last = (vector2){(float)x, (float)y};
        int pp = border_running_indices[i]++;
        if (pp < 0 || pp >= border_total)
            LOG_ERR("wtf");
        border_chunk_indices[pp] = current_chunk;
        for (int j = 0; j < border_counts[i]; ++j)
        {
            if (current_chunk < 0 || current_chunk >= total_found)
                LOG_ERR("Wtf");
            float last_angle = 3.1415926535897982384626433f * 2.f;
            vector2 current_vec;
            current_vec.x = x;
            current_vec.y = y;
            vector2 towards_boundary = {0};
            int num_boundaries = 0;
            for (int adj_x = -1; adj_x < 2; ++adj_x)
            {
                for (int adj_y = -1; adj_y < 2; ++adj_y)
                {
                    int is_boundary = (x + adj_x < 0) | (x + adj_x >= map->map_width) | (y + adj_y < 0) | (y + adj_y >= map->map_height);     
                    //int corrected_index = x + adj_x * (!is_boundary) + (y + adj_y * (!is_boundary)) * map->map_width;          // Out of bounds borders
                    int corrected_index = current_chunk + adj_x * !is_boundary + (adj_y * !is_boundary) * map->map_width;
                    is_boundary |= shape_ints[corrected_index] != shape_ints[current_chunk]; // Different shape borders
                    num_boundaries += is_boundary;
                    towards_boundary.x += adj_x * is_boundary;
                    towards_boundary.y += adj_y * is_boundary;
                }
            }
            towards_boundary.x /= (float)(num_boundaries + 1 * !num_boundaries);
            towards_boundary.y /= (float)(num_boundaries + 1 * !num_boundaries);
            vector2 away_from_boundary = vec_negate(towards_boundary);
            vector2 from_last_to_me = vec_sub(current_vec, last);
            vector2 sum = vec_normalize(vec_add(from_last_to_me, away_from_boundary));
            int next_boundary_index = 0;

            
            int x_in_range = (x + 1 < map->map_width);
            int y_in_range = (y + 1 < map->map_height);
            int capped_x = (x_in_range * 2 + (!x_in_range) * 1);
            int capped_y = (y_in_range * 2 + (!y_in_range) * 1);
            for (int adj_x = -1 * (x > 0); adj_x < capped_x; ++adj_x)
            {
                for (int adj_y = -1 * (y > 0); adj_y < capped_y; ++adj_y)
                {
                    int adjacent = x + adj_x + (y + adj_y) * map->map_width;
                    if (adjacent < 0 || adjacent >= total_found)
                        LOG_ERR("Border detection Out-Of-Bounds error in adjacent block");
                    vector2 my_vec = vec_normalize((vector2){adj_x, adj_y});
                    float angle = vec_angle_between(sum, my_vec);
                    int angle_is_better = (angle < last_angle);
                    int adjacent_in_same_shape = (shape_ints[adjacent] == shape_ints[current_chunk]);
                    int is_border = border_bits[adjacent];
                    int not_center = (adj_x || adj_y);

                    int suitable_adjacent = angle_is_better * adjacent_in_same_shape * not_center * is_border;
                    
                    next_boundary_index = (adjacent) * suitable_adjacent + next_boundary_index * (!suitable_adjacent);
                    last_angle = (suitable_adjacent * angle) + last_angle * (!suitable_adjacent);
                }
            }

            // Assign next x, y, current_chunk
            if (border_running_indices[i] < 0 || border_running_indices[i] >= border_total)
                LOG_ERR("Border detection Out-Of-Bounds problem");
            border_chunk_indices[border_running_indices[i]] = next_boundary_index;
            border_running_indices[i] += 1 * (next_boundary_index != 0);

            //LOG_INFO("Border progress of shape %d at location (%d, %d) (iteration: %d, last: (%.2f, %.2f), sum: (%.2f, %.2f), from_last_to_me: (%.2f, %.2f), away_from_boundary: (%.2f, %.2f), # of boundaries: %d)", i, x, y, iterations, last.x, last.y, sum.x, sum.y, from_last_to_me.x, from_last_to_me.y, away_from_boundary.x, away_from_boundary.y, num_boundaries);
            current_chunk = current_chunk * (!next_boundary_index) + next_boundary_index;
            last.x = x;
            last.y = y;
            x = current_chunk % map->map_width;
            y = current_chunk / map->map_width;
            ++iterations;
            free(malloc(1));
            free(malloc(10));
            free(malloc(100));
            free(malloc(100000));
        }
    }
    free(malloc(1));
    free(malloc(10));
    free(malloc(100));
    free(malloc(100000));
    free(border_bits);
    free(malloc(1));
    free(malloc(10));
    free(malloc(100));
    free(malloc(100000));
    // End Border Detection

    /// WRITE PNG SHIT DOWN HERE
    {
        LOG_INFO("Writing found shapes to yo mama.png");
        colour *colours = calloc(map->map_width * map->map_height, sizeof(colour));
        colour black = {0};

        unsigned char* written_to = calloc(map->map_width * map->map_height, sizeof(unsigned char));

        const colour asshole[] = { { 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x33 }, { 0x00, 0x00, 0x66 }, { 0x00, 0x00, 0x99 }, { 0x00, 0x00, 0xcc }, { 0x00, 0x00, 0xff }, { 0x00, 0x33, 0x00 }, { 0x00, 0x33, 0x33 }, { 0x00, 0x33, 0x66 }, { 0x00, 0x33, 0x99 }, { 0x00, 0x33, 0xcc }, { 0x00, 0x33, 0xff }, { 0x00, 0x66, 0x00 }, { 0x00, 0x66, 0x33 }, { 0x00, 0x66, 0x66 }, { 0x00, 0x66, 0x99 }, { 0x00, 0x66, 0xcc }, { 0x00, 0x66, 0xff }, { 0x00, 0x99, 0x00 }, { 0x00, 0x99, 0x33 }, { 0x00, 0x99, 0x66 }, { 0x00, 0x99, 0x99 }, { 0x00, 0x99, 0xcc }, { 0x00, 0x99, 0xff }, { 0x00, 0xcc, 0x00 }, { 0x00, 0xcc, 0x33 }, { 0x00, 0xcc, 0x66 }, { 0x00, 0xcc, 0x99 }, { 0x00, 0xcc, 0xcc }, { 0x00, 0xcc, 0xff }, { 0x00, 0xff, 0x00 }, { 0x00, 0xff, 0x33 }, { 0x00, 0xff, 0x66 }, { 0x00, 0xff, 0x99 }, { 0x00, 0xff, 0xcc }, { 0x00, 0xff, 0xff }, { 0x33, 0x00, 0x00 }, { 0x33, 0x00, 0x33 }, { 0x33, 0x00, 0x66 }, { 0x33, 0x00, 0x99 }, { 0x33, 0x00, 0xcc }, { 0x33, 0x00, 0xff }, { 0x33, 0x33, 0x00 }, { 0x33, 0x33, 0x33 }, { 0x33, 0x33, 0x66 }, { 0x33, 0x33, 0x99 }, { 0x33, 0x33, 0xcc }, { 0x33, 0x33, 0xff }, { 0x33, 0x66, 0x00 }, { 0x33, 0x66, 0x33 }, { 0x33, 0x66, 0x66 }, { 0x33, 0x66, 0x99 }, { 0x33, 0x66, 0xcc }, { 0x33, 0x66, 0xff }, { 0x33, 0x99, 0x00 }, { 0x33, 0x99, 0x33 }, { 0x33, 0x99, 0x66 }, { 0x33, 0x99, 0x99 }, { 0x33, 0x99, 0xcc }, { 0x33, 0x99, 0xff }, { 0x33, 0xcc, 0x00 }, { 0x33, 0xcc, 0x33 }, { 0x33, 0xcc, 0x66 }, { 0x33, 0xcc, 0x99 }, { 0x33, 0xcc, 0xcc }, { 0x33, 0xcc, 0xff }, { 0x33, 0xff, 0x00 }, { 0x33, 0xff, 0x33 }, { 0x33, 0xff, 0x66 }, { 0x33, 0xff, 0x99 }, { 0x33, 0xff, 0xcc }, { 0x33, 0xff, 0xff }, { 0x66, 0x00, 0x00 }, { 0x66, 0x00, 0x33 }, { 0x66, 0x00, 0x66 }, { 0x66, 0x00, 0x99 }, { 0x66, 0x00, 0xcc }, { 0x66, 0x00, 0xff }, { 0x66, 0x33, 0x00 }, { 0x66, 0x33, 0x33 }, { 0x66, 0x33, 0x66 }, { 0x66, 0x33, 0x99 }, { 0x66, 0x33, 0xcc }, { 0x66, 0x33, 0xff }, { 0x66, 0x66, 0x00 }, { 0x66, 0x66, 0x33 }, { 0x66, 0x66, 0x66 }, { 0x66, 0x66, 0x99 }, { 0x66, 0x66, 0xcc }, { 0x66, 0x66, 0xff }, { 0x66, 0x99, 0x00 }, { 0x66, 0x99, 0x33 }, { 0x66, 0x99, 0x66 }, { 0x66, 0x99, 0x99 }, { 0x66, 0x99, 0xcc }, { 0x66, 0x99, 0xff }, { 0x66, 0xcc, 0x00 }, { 0x66, 0xcc, 0x33 }, { 0x66, 0xcc, 0x66 }, { 0x66, 0xcc, 0x99 }, { 0x66, 0xcc, 0xcc }, { 0x66, 0xcc, 0xff }, { 0x66, 0xff, 0x00 }, { 0x66, 0xff, 0x33 }, { 0x66, 0xff, 0x66 }, { 0x66, 0xff, 0x99 }, { 0x66, 0xff, 0xcc }, { 0x66, 0xff, 0xff }, { 0x99, 0x00, 0x00 }, { 0x99, 0x00, 0x33 }, { 0x99, 0x00, 0x66 }, { 0x99, 0x00, 0x99 }, { 0x99, 0x00, 0xcc }, { 0x99, 0x00, 0xff }, { 0x99, 0x33, 0x00 }, { 0x99, 0x33, 0x33 }, { 0x99, 0x33, 0x66 }, { 0x99, 0x33, 0x99 }, { 0x99, 0x33, 0xcc }, { 0x99, 0x33, 0xff }, { 0x99, 0x66, 0x00 }, { 0x99, 0x66, 0x33 }, { 0x99, 0x66, 0x66 }, { 0x99, 0x66, 0x99 }, { 0x99, 0x66, 0xcc }, { 0x99, 0x66, 0xff }, { 0x99, 0x99, 0x00 }, { 0x99, 0x99, 0x33 }, { 0x99, 0x99, 0x66 }, { 0x99, 0x99, 0x99 }, { 0x99, 0x99, 0xcc }, { 0x99, 0x99, 0xff }, { 0x99, 0xcc, 0x00 }, { 0x99, 0xcc, 0x33 }, { 0x99, 0xcc, 0x66 }, { 0x99, 0xcc, 0x99 }, { 0x99, 0xcc, 0xcc }, { 0x99, 0xcc, 0xff }, { 0x99, 0xff, 0x00 }, { 0x99, 0xff, 0x33 }, { 0x99, 0xff, 0x66 }, { 0x99, 0xff, 0x99 }, { 0x99, 0xff, 0xcc }, { 0x99, 0xff, 0xff }, { 0xcc, 0x00, 0x00 }, { 0xcc, 0x00, 0x33 }, { 0xcc, 0x00, 0x66 }, { 0xcc, 0x00, 0x99 }, { 0xcc, 0x00, 0xcc }, { 0xcc, 0x00, 0xff }, { 0xcc, 0x33, 0x00 }, { 0xcc, 0x33, 0x33 }, { 0xcc, 0x33, 0x66 }, { 0xcc, 0x33, 0x99 }, { 0xcc, 0x33, 0xcc }, { 0xcc, 0x33, 0xff }, { 0xcc, 0x66, 0x00 }, { 0xcc, 0x66, 0x33 }, { 0xcc, 0x66, 0x66 }, { 0xcc, 0x66, 0x99 }, { 0xcc, 0x66, 0xcc }, { 0xcc, 0x66, 0xff }, { 0xcc, 0x99, 0x00 }, { 0xcc, 0x99, 0x33 }, { 0xcc, 0x99, 0x66 }, { 0xcc, 0x99, 0x99 }, { 0xcc, 0x99, 0xcc }, { 0xcc, 0x99, 0xff }, { 0xcc, 0xcc, 0x00 }, { 0xcc, 0xcc, 0x33 }, { 0xcc, 0xcc, 0x66 }, { 0xcc, 0xcc, 0x99 }, { 0xcc, 0xcc, 0xcc }, { 0xcc, 0xcc, 0xff }, { 0xcc, 0xff, 0x00 }, { 0xcc, 0xff, 0x33 }, { 0xcc, 0xff, 0x66 }, { 0xcc, 0xff, 0x99 }, { 0xcc, 0xff, 0xcc }, { 0xcc, 0xff, 0xff }, { 0xff, 0x00, 0x00 }, { 0xff, 0x00, 0x33 }, { 0xff, 0x00, 0x66 }, { 0xff, 0x00, 0x99 }, { 0xff, 0x00, 0xcc }, { 0xff, 0x00, 0xff }, { 0xff, 0x33, 0x00 }, { 0xff, 0x33, 0x33 }, { 0xff, 0x33, 0x66 }, { 0xff, 0x33, 0x99 }, { 0xff, 0x33, 0xcc }, { 0xff, 0x33, 0xff }, { 0xff, 0x66, 0x00 }, { 0xff, 0x66, 0x33 }, { 0xff, 0x66, 0x66 }, { 0xff, 0x66, 0x99 }, { 0xff, 0x66, 0xcc }, { 0xff, 0x66, 0xff }, { 0xff, 0x99, 0x00 }, { 0xff, 0x99, 0x33 }, { 0xff, 0x99, 0x66 }, { 0xff, 0x99, 0x99 }, { 0xff, 0x99, 0xcc }, { 0xff, 0x99, 0xff }, { 0xff, 0xcc, 0x00 }, { 0xff, 0xcc, 0x33 }, { 0xff, 0xcc, 0x66 }, { 0xff, 0xcc, 0x99 }, { 0xff, 0xcc, 0xcc }, { 0xff, 0xcc, 0xff }, { 0xff, 0xff, 0x00 }, { 0xff, 0xff, 0x33 }, { 0xff, 0xff, 0x66 }, { 0xff, 0xff, 0x99 }, { 0xff, 0xff, 0xcc }, };

        colourmap intermediate = {
            colours,
            map->map_width,
            map->map_height};

        pixel color_check = {255};
        for (int i = 0; i < num_shapes; ++i)
        {
            if (num_shapes < 50)
            {
                LOG_INFO("Shape %d has %d chunks and %d border chunks", i, shape_counts[i], border_counts[i]);
                pixel color = map->groups_array_2d[chunk_index_of[shape_offsets[i]] % map->map_width][chunk_index_of[shape_offsets[i]] / map->map_width].average_colour;
                LOG_INFO("Shape %d has color (%d, %d, %d)", i, color.r, color.g, color.b);
            }
            colour border_colour = { 0xaa, 0x14, 0xaa };
            char increment = 20;
            char sign = 1;
            for (int j = 0; j < border_counts[i]; ++j)
            {
                int border_piece = border_chunk_indices[border_offsets[i] + j];
                intermediate.colours[border_piece] = border_colour;
            }
        }
        free(written_to);

        int x_scale = 1, y_scale = 1;
        image output_img = create_image(intermediate.width * x_scale, intermediate.height * y_scale);

        for (int x = 0; x < intermediate.width; ++x)
        {
            for (int y = 0; y < intermediate.height; ++y)
            {
                colour *bob = &intermediate.colours[x + intermediate.width * y];
                for (int xx = 0; xx < x_scale; ++xx)
                {
                    for (int yy = 0; yy < y_scale; ++yy)
                    {
                        pixel *img_pix = &(output_img.pixels_array_2d[x * x_scale + xx][y * y_scale + yy]);
                        img_pix->r = bob->r;
                        img_pix->g = bob->g;
                        img_pix->b = bob->b;
                    }
                }
            }
        }

        write_image_to_png(output_img, "yo mama.png");

        free_image_contents(output_img);
        free(intermediate.colours);
    }
    // WRITE PNG SHIT STOPPED

    free(border_counts);

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