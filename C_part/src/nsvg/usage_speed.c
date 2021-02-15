#include "usage_speed.h"

#include "utility/logger.h"
#include "utility/vec.h"
#include "imagefile/pngfile.h"
#include "mapping.h"
#include "mapparser.h"
#include "utility/error.h"

#include <stdlib.h>
#include <stdio.h>

typedef struct find_shapes_speed_stuff
{
    int* shape_ints;
    int* shape_counts;
    int* shape_offsets;
    int num_shapes;
    int* chunk_index_of;
    int* border_counts;
    int* border_offsets;
    int* border_chunk_indices;
} find_shapes_speed_stuff;

find_shapes_speed_stuff produce_shape_stuff(chunkmap* map, float threshold)
{
    if (map->map_width < 1 || map->map_height < 1)
    {
        LOG_ERR("Can not process empty chunkmap");
        setError(BAD_ARGUMENT_ERROR);
        return (find_shapes_speed_stuff){ 0 };
    }

    find_shapes_speed_stuff output;

    LOG_INFO("Find Shapes Speedy with threshold: %.1f", threshold);
    output.shape_ints = calloc(map->map_width * map->map_height, sizeof(int));
    int next_shape_int = 0;

    LOG_INFO("Initial Shape Sweep");
    for (int x = 0; x < map->map_width; ++x)
    {
        for (int y = 0; y < map->map_height; ++y)
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
                    int val = (((adj_x != 0) | (adj_y != 0)) * similarity * output.shape_ints[current + adj_x + adj_y * map->map_width]);
                    output.shape_ints[current] = (!val * output.shape_ints[current]) + val;
                }
            }

            next_shape_int += 1 * (!output.shape_ints[current]); // Goes up every time a chunk has no similar neighbours with chunks
            int val = next_shape_int * (!output.shape_ints[current]);
            output.shape_ints[current] = (!val * output.shape_ints[current]) + val; // Assigns the incremented next_shape_int to this pixelchunk if it had no similar neighbours
        }
    }

    output.num_shapes = next_shape_int;
    LOG_INFO("Num Shapes calculated to: %d", output.num_shapes);

    LOG_INFO("Fixing Indices");
    for (int y = 0; y < map->map_height; ++y)
    {
        for (int x = 0; x < map->map_width; ++x)
        {
            int current = x + y * map->map_width;
            --output.shape_ints[current];
        }
    }

    {
        LOG_INFO("Shape Aggregation Sweep");
        int original_count = output.num_shapes;
        int *de_duplicated_indices = calloc(output.num_shapes * 2, sizeof(int));
        int *index_gaps = de_duplicated_indices + output.num_shapes;
        for (int i = 0; i < output.num_shapes; ++i)
            de_duplicated_indices[i] = i;
        for (int x = 0; x < map->map_width; ++x)
        {
            for (int y = 0; y < map->map_height; ++y)
            {
                int current = x + y * map->map_width;
                int x_in_range = (x + 1 < map->map_width);
                int y_in_range = (y + 1 < map->map_height);
                int capped_x = (x_in_range * 2 + (!x_in_range) * 1);
                int capped_y = (y_in_range * 2 + (!y_in_range) * 1);
                int skip_me = de_duplicated_indices[output.shape_ints[current]] == output.shape_ints[current];
                for (int adj_x = 0 - (x > 0) * 1; adj_x < capped_x; ++adj_x)
                {
                    for (int adj_y = 0 - (y > 0) * 1; adj_y < capped_y; ++adj_y)
                    {
                        int adjacent = current + adj_x + adj_y * map->map_width;
                        int similarity = colours_are_similar(map->groups_array_2d[x][y].average_colour, map->groups_array_2d[x + adj_x][y + adj_y].average_colour, threshold);
                        int in_different_shapes = de_duplicated_indices[output.shape_ints[current]] != de_duplicated_indices[output.shape_ints[adjacent]];
                        int val = similarity * in_different_shapes * skip_me;
                        int already_changed = de_duplicated_indices[output.shape_ints[adjacent]] != output.shape_ints[adjacent];
                        de_duplicated_indices[output.shape_ints[adjacent]] = (de_duplicated_indices[output.shape_ints[adjacent]] * !val) + (de_duplicated_indices[output.shape_ints[current]] * val);
                        output.num_shapes += -1 * val * !already_changed;
                    }
                }
            }
        }

        int running_gaps = 0;
        for (int i = 0; i < original_count; ++i)
        {
            index_gaps[i] = running_gaps;
            running_gaps += 1 * (de_duplicated_indices[i] != i);
        }

        int lost_shapes = original_count - output.num_shapes;
        if (lost_shapes)
        {
            LOG_INFO("Found %d duplicate shapes", lost_shapes);
            LOG_INFO("Correcting Indices");
            for (int x = 0; x < map->map_width; ++x)
            {
                for (int y = 0; y < map->map_height; ++y)
                {
                    int current = x + y * map->map_width;
                    int de_dup_shape_index = de_duplicated_indices[output.shape_ints[current]];
                    int val = de_dup_shape_index - index_gaps[de_dup_shape_index];
                    output.shape_ints[current] = val;
                }
            }
        }
        else
        {
            LOG_INFO("No Shapes to De-Duplicate");
        }
        free(de_duplicated_indices);
    }
    
    LOG_INFO("Num Shapes Now calculated to: %d", output.num_shapes);

    {
        int bad_ness = 0;
        for (int x = 0; x < map->map_width; ++x)
        {
            for (int y = 0; y < map->map_height; ++y)
            {
                int current = x + y * map->map_width;
                bad_ness = output.shape_ints[current] < 0 || output.shape_ints[current] >= output.num_shapes;
            }
        }
        if (bad_ness)
            LOG_ERR("Somehow De-Duplication failed to correct Shape Indices");
    }

    LOG_INFO("Counting up Shape numbers");
    output.shape_counts = calloc(output.num_shapes * 3 + map->map_width * map->map_height, sizeof(int));
    int* increasing_shape_indices = output.shape_counts + output.num_shapes;
    output.shape_offsets = increasing_shape_indices + output.num_shapes; // An array containing the index of the first chunk of each shape in the shapes array
    output.chunk_index_of = output.shape_offsets + output.num_shapes; // Stores indices into the chunkmap 

    for (int y = 0; y < map->map_height; ++y)
    {
        for (int x = 0; x < map->map_width; ++x)
        {
            int current = x + y * map->map_width;
            int index = output.shape_ints[current];
            if (index >= output.num_shapes || index < 0)
                LOG_ERR("Shape Int of (%d, %d) was not properly updated in De-Duplication (has value: %d)", x, y, index);
            ++output.shape_counts[index];
        }
    }

    int total_found = 0;
    for (int i = 0; i < output.num_shapes; ++i)
    {
        total_found += output.shape_counts[i];
    }

    LOG_INFO("Total Shape Count: %d", total_found);
    if (total_found != map->map_width * map->map_height)
        LOG_WARN("Total Shape Count was not %u (map w x h)! Something is wrong...", map->map_width * map->map_height);
    
    {
        LOG_INFO("Calculating Shape Offsets");
        int running_index = 0;
        for (int i = 0; i < output.num_shapes; ++i)
        {
            increasing_shape_indices[i] = running_index;
            output.shape_offsets[i] = running_index;
            running_index += output.shape_counts[i];
        }
    }

    LOG_INFO("Moving Chunk Indices into their Shapes");
    for (int y = 0; y < map->map_height; ++y)
    {
        for (int x = 0; x < map->map_width; ++x)
        {
            int current_chunk = x + y * map->map_width;
            int* p = &increasing_shape_indices[output.shape_ints[current_chunk]];
            output.chunk_index_of[*p] = current_chunk;
            *p += 1;
        }
    }

    {
        LOG_INFO("Checking for Duplicates");
        unsigned char* checked_indices = calloc(map->map_width * map->map_height, sizeof(unsigned char));
        int did = 0;
        for (int i = 0; i < output.num_shapes; ++i)
        {
            for (int j = 0; j < output.shape_counts[i]; ++j)
            {
                if (checked_indices[output.chunk_index_of[j + output.shape_offsets[i]]])
                {
                    LOG_WARN("Shape %d has a duplicate index of: %d", i, j + output.shape_offsets[i]);
                    did = 1;
                }
                checked_indices[output.chunk_index_of[j + output.shape_offsets[i]]] = 1;
            }
        }
        if (!did)
            LOG_INFO("No Found Duplicate indices");
        free(checked_indices);
    }
    
    LOG_INFO("Beginning Border Detection");
    LOG_INFO("Border Counting Sweep");
    // Border Detection
    output.border_counts = calloc(output.num_shapes * 3, sizeof(int));
    output.border_offsets = output.border_counts + output.num_shapes;
    int *border_running_indices = output.border_offsets + output.num_shapes;
    char *border_bits = calloc(map->map_width * map->map_height, sizeof(char));
    int border_total = 0;
    for (int i = 0; i < output.num_shapes; ++i)
    {
        for (int j = 0; j < output.shape_counts[i]; ++j)
        {
            // Look through all coordinates
            // See if any adjacents either: don't exist (edge of the image) or are a different colour
            int current = output.chunk_index_of[j + output.shape_offsets[i]];
            int x = current % map->map_width;
            int y = current / map->map_width;
            int edge = (x < 1) | (y < 1) | (x + 1 >= map->map_width) | (y + 1 >= map->map_height);
            int border = edge;
            for (int adj_x = (2 * edge) + (-1 * !edge); adj_x < 2; ++adj_x)
            {
                for (int adj_y = (2 * edge) + (-1 * !edge); adj_y < 2; ++adj_y)
                {
                    border |= output.shape_ints[current] != output.shape_ints[x + adj_x + (y + adj_y) * map->map_width];
                }
            }

            border_total += 1 * border;
            output.border_counts[i] += 1 * border;
            border_bits[current] = (char)border;
        }
    }
    
    LOG_INFO("Border count calculated as: %d", border_total);

    // Initialize Offsets
    {
        LOG_INFO("Calculating Border Offsets");
        int running_offset = 0;
        for (int i = 0; i < output.num_shapes; ++i)
        {
            output.border_offsets[i] = running_offset;
            border_running_indices[i] = running_offset;
            running_offset += output.border_counts[i];
        }
    }

    LOG_INFO("Border Ordering Sweep");
    output.border_chunk_indices = calloc(border_total, sizeof(int));
    int iterations = 0;
    for (int i = 0; i < output.num_shapes; ++i)
    {
        int first_chunk = output.chunk_index_of[output.shape_offsets[i]];
        int current_chunk = first_chunk;
        int x = current_chunk % map->map_width;
        int y = current_chunk / map->map_width;
        vector2 last = (vector2){(float)x, (float)y};
        int pp = border_running_indices[i];
        //if (pp < 0 || pp >= border_total)// DEBUG
            //LOG_ERR("wtf");//DEBUg
        output.border_chunk_indices[pp] = current_chunk;
        for (int j = 0; j < output.border_counts[i]; ++j)
        {
            //if (current_chunk < 0 || current_chunk >= total_found)// DEBUG
            //    LOG_ERR("Wtf");// DEBUG
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
                    is_boundary |= output.shape_ints[corrected_index] != output.shape_ints[current_chunk]; // Different shape borders
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
                    //if (adjacent < 0 || adjacent >= total_found)
                        //LOG_ERR("Border detection Out-Of-Bounds error in adjacent block");//DEBUG
                    vector2 my_vec = vec_normalize((vector2){adj_x, adj_y});
                    float angle = vec_angle_between(sum, my_vec);
                    int angle_is_better = (angle < last_angle);
                    int adjacent_in_same_shape = (output.shape_ints[adjacent] == output.shape_ints[current_chunk]);
                    int is_border = border_bits[adjacent];
                    int not_center = (adj_x || adj_y);

                    int suitable_adjacent = angle_is_better * adjacent_in_same_shape * not_center * is_border;
                    
                    next_boundary_index = (adjacent) * suitable_adjacent + next_boundary_index * (!suitable_adjacent);
                    last_angle = (suitable_adjacent * angle) + last_angle * (!suitable_adjacent);
                }
            }

            // Assign next x, y, current_chunk
            //if (border_running_indices[i] < 0 || border_running_indices[i] >= border_total)// DEBUG
                //LOG_ERR("Border detection Out-Of-Bounds problem");// DEBUG
            output.border_chunk_indices[border_running_indices[i]] = next_boundary_index;
            border_running_indices[i] += 1 * (next_boundary_index != 0);

            //LOG_INFO("Border progress of shape %d at location (%d, %d) (iteration: %d, last: (%.2f, %.2f), sum: (%.2f, %.2f), from_last_to_me: (%.2f, %.2f), away_from_boundary: (%.2f, %.2f), # of boundaries: %d)", i, x, y, iterations, last.x, last.y, sum.x, sum.y, from_last_to_me.x, from_last_to_me.y, away_from_boundary.x, away_from_boundary.y, num_boundaries);
            current_chunk = current_chunk * (!next_boundary_index) + next_boundary_index;
            last.x = x;
            last.y = y;
            x = current_chunk % map->map_width;
            y = current_chunk / map->map_width;
            ++iterations;
        }
    }
    free(border_bits);
    // End Border Detection

    LOG_INFO("Border and Shape Detection Complete");

    return output;
}

void write_shape_struct_to_png(find_shapes_speed_stuff stuff, chunkmap* map, char* fileaddress)
{
    LOG_INFO("Writing found shapes to '%s'", fileaddress);
    colour *colours = calloc(map->map_width * map->map_height, sizeof(colour));
    colour black = {0};

    unsigned char* written_to = calloc(map->map_width * map->map_height, sizeof(unsigned char));

    const colour asshole[] = { { 0x00, 0x00, 0x33 }, { 0x00, 0x00, 0x66 }, { 0x00, 0x00, 0x99 }, { 0x00, 0x00, 0xcc }, { 0x00, 0x00, 0xff }, { 0x00, 0x33, 0x00 }, { 0x00, 0x33, 0x33 }, { 0x00, 0x33, 0x66 }, { 0x00, 0x33, 0x99 }, { 0x00, 0x33, 0xcc }, { 0x00, 0x33, 0xff }, { 0x00, 0x66, 0x00 }, { 0x00, 0x66, 0x33 }, { 0x00, 0x66, 0x66 }, { 0x00, 0x66, 0x99 }, { 0x00, 0x66, 0xcc }, { 0x00, 0x66, 0xff }, { 0x00, 0x99, 0x00 }, { 0x00, 0x99, 0x33 }, { 0x00, 0x99, 0x66 }, { 0x00, 0x99, 0x99 }, { 0x00, 0x99, 0xcc }, { 0x00, 0x99, 0xff }, { 0x00, 0xcc, 0x00 }, { 0x00, 0xcc, 0x33 }, { 0x00, 0xcc, 0x66 }, { 0x00, 0xcc, 0x99 }, { 0x00, 0xcc, 0xcc }, { 0x00, 0xcc, 0xff }, { 0x00, 0xff, 0x00 }, { 0x00, 0xff, 0x33 }, { 0x00, 0xff, 0x66 }, { 0x00, 0xff, 0x99 }, { 0x00, 0xff, 0xcc }, { 0x00, 0xff, 0xff }, { 0x33, 0x00, 0x00 }, { 0x33, 0x00, 0x33 }, { 0x33, 0x00, 0x66 }, { 0x33, 0x00, 0x99 }, { 0x33, 0x00, 0xcc }, { 0x33, 0x00, 0xff }, { 0x33, 0x33, 0x00 }, { 0x33, 0x33, 0x33 }, { 0x33, 0x33, 0x66 }, { 0x33, 0x33, 0x99 }, { 0x33, 0x33, 0xcc }, { 0x33, 0x33, 0xff }, { 0x33, 0x66, 0x00 }, { 0x33, 0x66, 0x33 }, { 0x33, 0x66, 0x66 }, { 0x33, 0x66, 0x99 }, { 0x33, 0x66, 0xcc }, { 0x33, 0x66, 0xff }, { 0x33, 0x99, 0x00 }, { 0x33, 0x99, 0x33 }, { 0x33, 0x99, 0x66 }, { 0x33, 0x99, 0x99 }, { 0x33, 0x99, 0xcc }, { 0x33, 0x99, 0xff }, { 0x33, 0xcc, 0x00 }, { 0x33, 0xcc, 0x33 }, { 0x33, 0xcc, 0x66 }, { 0x33, 0xcc, 0x99 }, { 0x33, 0xcc, 0xcc }, { 0x33, 0xcc, 0xff }, { 0x33, 0xff, 0x00 }, { 0x33, 0xff, 0x33 }, { 0x33, 0xff, 0x66 }, { 0x33, 0xff, 0x99 }, { 0x33, 0xff, 0xcc }, { 0x33, 0xff, 0xff }, { 0x66, 0x00, 0x00 }, { 0x66, 0x00, 0x33 }, { 0x66, 0x00, 0x66 }, { 0x66, 0x00, 0x99 }, { 0x66, 0x00, 0xcc }, { 0x66, 0x00, 0xff }, { 0x66, 0x33, 0x00 }, { 0x66, 0x33, 0x33 }, { 0x66, 0x33, 0x66 }, { 0x66, 0x33, 0x99 }, { 0x66, 0x33, 0xcc }, { 0x66, 0x33, 0xff }, { 0x66, 0x66, 0x00 }, { 0x66, 0x66, 0x33 }, { 0x66, 0x66, 0x66 }, { 0x66, 0x66, 0x99 }, { 0x66, 0x66, 0xcc }, { 0x66, 0x66, 0xff }, { 0x66, 0x99, 0x00 }, { 0x66, 0x99, 0x33 }, { 0x66, 0x99, 0x66 }, { 0x66, 0x99, 0x99 }, { 0x66, 0x99, 0xcc }, { 0x66, 0x99, 0xff }, { 0x66, 0xcc, 0x00 }, { 0x66, 0xcc, 0x33 }, { 0x66, 0xcc, 0x66 }, { 0x66, 0xcc, 0x99 }, { 0x66, 0xcc, 0xcc }, { 0x66, 0xcc, 0xff }, { 0x66, 0xff, 0x00 }, { 0x66, 0xff, 0x33 }, { 0x66, 0xff, 0x66 }, { 0x66, 0xff, 0x99 }, { 0x66, 0xff, 0xcc }, { 0x66, 0xff, 0xff }, { 0x99, 0x00, 0x00 }, { 0x99, 0x00, 0x33 }, { 0x99, 0x00, 0x66 }, { 0x99, 0x00, 0x99 }, { 0x99, 0x00, 0xcc }, { 0x99, 0x00, 0xff }, { 0x99, 0x33, 0x00 }, { 0x99, 0x33, 0x33 }, { 0x99, 0x33, 0x66 }, { 0x99, 0x33, 0x99 }, { 0x99, 0x33, 0xcc }, { 0x99, 0x33, 0xff }, { 0x99, 0x66, 0x00 }, { 0x99, 0x66, 0x33 }, { 0x99, 0x66, 0x66 }, { 0x99, 0x66, 0x99 }, { 0x99, 0x66, 0xcc }, { 0x99, 0x66, 0xff }, { 0x99, 0x99, 0x00 }, { 0x99, 0x99, 0x33 }, { 0x99, 0x99, 0x66 }, { 0x99, 0x99, 0x99 }, { 0x99, 0x99, 0xcc }, { 0x99, 0x99, 0xff }, { 0x99, 0xcc, 0x00 }, { 0x99, 0xcc, 0x33 }, { 0x99, 0xcc, 0x66 }, { 0x99, 0xcc, 0x99 }, { 0x99, 0xcc, 0xcc }, { 0x99, 0xcc, 0xff }, { 0x99, 0xff, 0x00 }, { 0x99, 0xff, 0x33 }, { 0x99, 0xff, 0x66 }, { 0x99, 0xff, 0x99 }, { 0x99, 0xff, 0xcc }, { 0x99, 0xff, 0xff }, { 0xcc, 0x00, 0x00 }, { 0xcc, 0x00, 0x33 }, { 0xcc, 0x00, 0x66 }, { 0xcc, 0x00, 0x99 }, { 0xcc, 0x00, 0xcc }, { 0xcc, 0x00, 0xff }, { 0xcc, 0x33, 0x00 }, { 0xcc, 0x33, 0x33 }, { 0xcc, 0x33, 0x66 }, { 0xcc, 0x33, 0x99 }, { 0xcc, 0x33, 0xcc }, { 0xcc, 0x33, 0xff }, { 0xcc, 0x66, 0x00 }, { 0xcc, 0x66, 0x33 }, { 0xcc, 0x66, 0x66 }, { 0xcc, 0x66, 0x99 }, { 0xcc, 0x66, 0xcc }, { 0xcc, 0x66, 0xff }, { 0xcc, 0x99, 0x00 }, { 0xcc, 0x99, 0x33 }, { 0xcc, 0x99, 0x66 }, { 0xcc, 0x99, 0x99 }, { 0xcc, 0x99, 0xcc }, { 0xcc, 0x99, 0xff }, { 0xcc, 0xcc, 0x00 }, { 0xcc, 0xcc, 0x33 }, { 0xcc, 0xcc, 0x66 }, { 0xcc, 0xcc, 0x99 }, { 0xcc, 0xcc, 0xcc }, { 0xcc, 0xcc, 0xff }, { 0xcc, 0xff, 0x00 }, { 0xcc, 0xff, 0x33 }, { 0xcc, 0xff, 0x66 }, { 0xcc, 0xff, 0x99 }, { 0xcc, 0xff, 0xcc }, { 0xcc, 0xff, 0xff }, { 0xff, 0x00, 0x00 }, { 0xff, 0x00, 0x33 }, { 0xff, 0x00, 0x66 }, { 0xff, 0x00, 0x99 }, { 0xff, 0x00, 0xcc }, { 0xff, 0x00, 0xff }, { 0xff, 0x33, 0x00 }, { 0xff, 0x33, 0x33 }, { 0xff, 0x33, 0x66 }, { 0xff, 0x33, 0x99 }, { 0xff, 0x33, 0xcc }, { 0xff, 0x33, 0xff }, { 0xff, 0x66, 0x00 }, { 0xff, 0x66, 0x33 }, { 0xff, 0x66, 0x66 }, { 0xff, 0x66, 0x99 }, { 0xff, 0x66, 0xcc }, { 0xff, 0x66, 0xff }, { 0xff, 0x99, 0x00 }, { 0xff, 0x99, 0x33 }, { 0xff, 0x99, 0x66 }, { 0xff, 0x99, 0x99 }, { 0xff, 0x99, 0xcc }, { 0xff, 0x99, 0xff }, { 0xff, 0xcc, 0x00 }, { 0xff, 0xcc, 0x33 }, { 0xff, 0xcc, 0x66 }, { 0xff, 0xcc, 0x99 }, { 0xff, 0xcc, 0xcc }, { 0xff, 0xcc, 0xff }, { 0xff, 0xff, 0x00 }, { 0xff, 0xff, 0x33 }, { 0xff, 0xff, 0x66 }, { 0xff, 0xff, 0x99 }, { 0xff, 0xff, 0xcc }, };

    colourmap intermediate = {
        colours,
        map->map_width,
        map->map_height
    };

    for (int i = 0; i < stuff.num_shapes; ++i)
    {
        if (stuff.num_shapes < 50)
        {
            LOG_INFO("Shape %d has %d chunks and %d border chunks", i, stuff.shape_counts[i], stuff.border_counts[i]);
            pixel color = map->groups_array_2d[stuff.chunk_index_of[stuff.shape_offsets[i]] % map->map_width][stuff.chunk_index_of[stuff.shape_offsets[i]] / map->map_width].average_colour;
            LOG_INFO("Shape %d's first chunk has color (%d, %d, %d)", i, color.r, color.g, color.b);
        }

        for (int j = stuff.shape_offsets[i]; j < stuff.shape_offsets[i] + stuff.shape_counts[i]; ++j)
        {
            int index = stuff.chunk_index_of[j];
            pixelchunk *chunga = &map->groups_array_2d[index % map->map_width][index / map->map_width];
            coordinate location = chunga->location;
            colour *ass = &intermediate.colours[location.x + location.y * intermediate.width];
            *ass = asshole[i % 214];
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

    write_image_to_png(output_img, fileaddress);

    free_image_contents(output_img);
    free(intermediate.colours);
    LOG_INFO("Written %d shapes to '%s'", stuff.num_shapes, fileaddress);
}

void write_shape_borders_to_png(find_shapes_speed_stuff stuff, chunkmap* map, char* fileaddress)
{
    LOG_INFO("Writing found shapes' borders to '%s'", fileaddress);
    colour *colours = calloc(map->map_width * map->map_height, sizeof(colour));
    colour black = {0};
    colour white = { 0xff, 0xff, 0xff };

    unsigned char* written_to = calloc(map->map_width * map->map_height, sizeof(unsigned char));

    colourmap intermediate = {
        colours,
        map->map_width,
        map->map_height
    };

    for (int i = 0; i < stuff.num_shapes; ++i)
    {
        if (stuff.num_shapes < 50)
        {
            LOG_INFO("Shape %d has %d chunks and %d border chunks", i, stuff.shape_counts[i], stuff.border_counts[i]);
            pixel color = map->groups_array_2d[stuff.chunk_index_of[stuff.shape_offsets[i]] % map->map_width][stuff.chunk_index_of[stuff.shape_offsets[i]] / map->map_width].average_colour;
            LOG_INFO("Shape %d has color (%d, %d, %d)", i, color.r, color.g, color.b);
        }
        colour border_colour = black;
        for (int j = 0; j < stuff.border_counts[i]; ++j)
        {
            int border_piece = stuff.border_chunk_indices[stuff.border_offsets[i] + j];
            intermediate.colours[border_piece] = border_colour;

            border_colour = lerp_colours(black, white, (float)j / (float)stuff.border_counts[i]);
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

    write_image_to_png(output_img, fileaddress);

    free_image_contents(output_img);
    free(intermediate.colours);
    LOG_INFO("Written %d shapes' borders to '%s'", stuff.num_shapes, fileaddress);
}

void free_shape_stuff(find_shapes_speed_stuff stuff)
{
    if (stuff.shape_ints)
        free(stuff.shape_ints);
    stuff.shape_ints = 0;
    if (stuff.border_counts)
        free(stuff.border_counts);
    stuff.border_counts = 0;
    if (stuff.shape_counts)
        free(stuff.shape_counts);
    stuff.border_counts = 0;
}

NSVGimage* produce_nsvg(chunkmap* map, float threshold, find_shapes_speed_stuff stuff)
{
    map->shape_count = stuff.num_shapes;
    // START CONVERT TO ACTUAL SHAPES
    chunkshape* actual_shapes = calloc(stuff.num_shapes, sizeof(chunkshape));

    for (int i = 0; i < stuff.num_shapes; ++i)
    {
        actual_shapes[i].next = (i + 1 < stuff.num_shapes ? &actual_shapes[i + 1] : NULL);
        actual_shapes[i].boundaries_length = stuff.border_counts[i];
        actual_shapes[i].boundaries = calloc(stuff.border_counts[i], sizeof(pixelchunk_list));
        actual_shapes[i].chunks_amount = stuff.border_counts[i];
        int first_chunk = stuff.chunk_index_of[stuff.shape_offsets[i]];
        actual_shapes[i].colour = map->groups_array_2d[first_chunk % map->map_width][first_chunk / map->map_width].average_colour;
        for (int j = 0; j < actual_shapes[i].boundaries_length; ++j)
        {
            int chunk_index = stuff.chunk_index_of[j + stuff.shape_offsets[i]];
            actual_shapes[i].boundaries[j].chunk_p = &map->groups_array_2d[chunk_index % map->map_width][chunk_index / map->map_width];
            actual_shapes[i].boundaries[j].next = (j + 1 < stuff.border_counts[i] ? &actual_shapes[i].boundaries[j + 1] : NULL);
        }
    }

    // END CONVERT TO ACTUAL SHAPES

    chunkshape* tmp = map->shape_list;
    map->shape_list = actual_shapes;

    NSVGimage* nsvg = create_nsvgimage(map->map_width, map->map_height);
    iterate_chunk_shapes(map, nsvg);

    if (isBadError())
    {
        LOG_ERR("Failed to iterate chunk shapes with error: %d", getLastError());
        map->shape_list = tmp;
        free(actual_shapes);
        return NULL;
    }

    map->shape_list = tmp;
    free(actual_shapes);

    return nsvg;
}


NSVGimage* vectorize_image_speed(image input, vectorize_options options)
{
    open_log("log.txt");
    chunkmap* map = generate_chunkmap(input, options);

    find_shapes_speed_stuff stuff = produce_shape_stuff(map, options.shape_colour_threshhold);

    NSVGimage* out = produce_nsvg(map, options.shape_colour_threshhold, stuff);

    if (isBadError())
    {
        LOG_ERR("Failed to Vectorize Image (v2) error: %d", getLastError());
        free_chunkmap(map);
        free_shape_stuff(stuff);
        close_log();
        return NULL;
    }

    free_chunkmap(map);
    free_shape_stuff(stuff);
    close_log();

    return out;
}

void vectorize_debug_speed(image input, vectorize_options options, char* shapefile, char* borderfile)
{
    open_log("vectorize_debug.txt");
    chunkmap* map = generate_chunkmap(input, options);

    find_shapes_speed_stuff stuff = produce_shape_stuff(map, options.shape_colour_threshhold);

    write_shape_borders_to_png(stuff, map, borderfile);
    write_shape_struct_to_png(stuff, map, shapefile);

    free_chunkmap(map);
    free_shape_stuff(stuff);
    close_log();

}