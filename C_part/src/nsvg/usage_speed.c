#include "usage_speed.h"

#include "utility/logger.h"
#include "utility/vec.h"
#include "imagefile/pngfile.h"
#include "mapping.h"
#include "mapparser.h"
#include "utility/error.h"
#include "sort.h"

#include <stdlib.h>
#include <stdio.h>

find_shapes_speed_stuff* produce_shape_stuff(chunkmap* map, float threshold)
{
    if (isBadError())
    {
        LOG_WARN("A Previous error occured: %d, you should bubble these errors up", getLastError());
        return NULL;
    }
    
    if (map->map_width < 1 || map->map_height < 1)
    {
        LOG_ERR("Can not process empty chunkmap");
        setError(BAD_ARGUMENT_ERROR);
        return NULL;
    }

    find_shapes_speed_stuff* output = calloc(1, sizeof(find_shapes_speed_stuff));

    LOG_INFO("Find Shapes Speedy with threshold: %.1f", threshold);
    shape_initial_sweep(map, output, threshold);

    shape_aggregate_sweep(map, output, threshold);

    if (isBadError())
    {
        LOG_ERR("Shape Aggregate Sweep failed with: %d", getLastError());
        return NULL;
    }
    
    LOG_INFO("Num Shapes Now calculated to: %d", output->num_shapes);

    move_shape_indices(map, output);
    if (isBadError())
    {
        LOG_ERR("Moving Shape Indices failed with: %d");
        free_shape_stuff(output);
        return NULL;
    }

    {
        LOG_INFO("Checking for Duplicates");
        unsigned char* checked_indices = calloc(map->map_width * map->map_height, sizeof(unsigned char));
        int did = 0;
        
        for (int i = 0; i < output->num_shapes; ++i)
        {
            for (int j = 0; j < output->shape_counts[i]; ++j)
            {
                if (checked_indices[output->chunk_index_of[j + output->shape_offsets[i]]])
                {
                    LOG_WARN("Shape %d has a duplicate index of: %d", i, j + output->shape_offsets[i]);
                    did = 1;
                }
                checked_indices[output->chunk_index_of[j + output->shape_offsets[i]]] = 1;
            }
        }
        
        if (!did)
            LOG_INFO("No Duplicate indices found");
            
        else
        {
            LOG_ERR("Duplicates were found");
            setError(ASSUMPTION_WRONG);
            free(checked_indices);
            free_shape_stuff(output);
            return NULL;
        }
        free(checked_indices);
    }
    
    LOG_INFO("Beginning Border Detection");
    LOG_INFO("Border Counting Sweep");
    // Border Detection
    output->border_counts = calloc(output->num_shapes * 3, sizeof(int));
    output->border_offsets = output->border_counts + output->num_shapes;
    int *border_running_indices = output->border_offsets + output->num_shapes;
    char *border_bits = calloc(map->map_width * map->map_height, sizeof(char));
    int border_total = 0;

    for (int i = 0; i < output->num_shapes; ++i)
    {
        for (int j = 0; j < output->shape_counts[i]; ++j)
        {
            // Look through all coordinates
            // See if any adjacents either: don't exist (edge of the image) or are a different colour
            int current = output->chunk_index_of[j + output->shape_offsets[i]];
            int x = current % map->map_width;
            int y = current / map->map_width;
            int edge = (x < 1) | (y < 1) | (x + 1 >= map->map_width) | (y + 1 >= map->map_height);
            int border = edge;

            for (int adj_x = (2 * edge) + (-1 * !edge); adj_x < 2; ++adj_x)
            {
                for (int adj_y = (2 * edge) + (-1 * !edge); adj_y < 2; ++adj_y)
                {
                    border |= output->shape_ints[current] != output->shape_ints[x + adj_x + (y + adj_y) * map->map_width];
                }
            }

            border_total += 1 * border;
            output->border_counts[i] += 1 * border;
            border_bits[current] = (char)border;
        }
    }
    
    LOG_INFO("Border count calculated as: %d", border_total);

    // Initialize Offsets
    {
        LOG_INFO("Calculating Border Offsets");
        int running_offset = 0;

        for (int i = 0; i < output->num_shapes; ++i)
        {
            output->border_offsets[i] = running_offset;
            border_running_indices[i] = running_offset;
            running_offset += output->border_counts[i];
        }
    }

    return output;
}

void shape_initial_sweep(chunkmap* map, find_shapes_speed_stuff* stuff, float threshold)
{
    stuff->shape_ints = calloc(map->map_width * map->map_height, sizeof(int));
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
                    int val = (((adj_x != 0) | (adj_y != 0)) * similarity * stuff->shape_ints[current + adj_x + adj_y * map->map_width]);
                    stuff->shape_ints[current] = (!val * stuff->shape_ints[current]) + val;
                }
            }

            next_shape_int += 1 * (!stuff->shape_ints[current]); // Goes up every time a chunk has no similar neighbours with chunks
            int val = next_shape_int * (!stuff->shape_ints[current]);
            stuff->shape_ints[current] = (!val * stuff->shape_ints[current]) + val; // Assigns the incremented next_shape_int to this pixelchunk if it had no similar neighbours
        }
    }

    stuff->num_shapes = next_shape_int;
    LOG_INFO("Num Shapes calculated to: %d", stuff->num_shapes);
    LOG_INFO("Fixing Indices");
    
    for (int y = 0; y < map->map_height; ++y)
    {
        for (int x = 0; x < map->map_width; ++x)
        {
            int current = x + y * map->map_width;
            --stuff->shape_ints[current];
        }
    }
}

void shape_aggregate_sweep(chunkmap* map, find_shapes_speed_stuff* stuff, float threshold)
{
    LOG_INFO("Shape Aggregation Sweep");
    int original_count = stuff->num_shapes;
    int *de_duplicated_indices = calloc(stuff->num_shapes * 2, sizeof(int));
    int *index_gaps = de_duplicated_indices + stuff->num_shapes;

    for (int i = 0; i < stuff->num_shapes; ++i)
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
            int skip_me = de_duplicated_indices[stuff->shape_ints[current]] == stuff->shape_ints[current];

            for (int adj_x = 0 - (x > 0) * 1; adj_x < capped_x; ++adj_x)
            {
                for (int adj_y = 0 - (y > 0) * 1; adj_y < capped_y; ++adj_y)
                {
                    int adjacent = current + adj_x + adj_y * map->map_width;
                    int similarity = colours_are_similar(map->groups_array_2d[x][y].average_colour, map->groups_array_2d[x + adj_x][y + adj_y].average_colour, threshold);
                    int in_different_shapes = de_duplicated_indices[stuff->shape_ints[current]] != de_duplicated_indices[stuff->shape_ints[adjacent]];
                    int val = similarity * in_different_shapes * skip_me;
                    int already_changed = de_duplicated_indices[stuff->shape_ints[adjacent]] != stuff->shape_ints[adjacent];
                    de_duplicated_indices[stuff->shape_ints[adjacent]] = (de_duplicated_indices[stuff->shape_ints[adjacent]] * !val) + (de_duplicated_indices[stuff->shape_ints[current]] * val);
                    stuff->num_shapes += -1 * val * !already_changed;
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
    int lost_shapes = original_count - stuff->num_shapes;
    
    if (lost_shapes)
    {
        LOG_INFO("Found %d duplicate shapes", lost_shapes);
        LOG_INFO("Correcting Indices");
        
        for (int x = 0; x < map->map_width; ++x)
        {
            for (int y = 0; y < map->map_height; ++y)
            {
                int current = x + y * map->map_width;
                int de_dup_shape_index = de_duplicated_indices[stuff->shape_ints[current]];
                int val = de_dup_shape_index - index_gaps[de_dup_shape_index];
                stuff->shape_ints[current] = val;
            }
        }
    }
    
    else
    {
        LOG_INFO("No Shapes to De-Duplicate");
    }
    free(de_duplicated_indices);

    {
        int bad_ness = 0;
        
        for (int x = 0; x < map->map_width; ++x)
        {
            for (int y = 0; y < map->map_height; ++y)
            {
                int current = x + y * map->map_width;
                bad_ness = stuff->shape_ints[current] < 0 || stuff->shape_ints[current] >= stuff->num_shapes;
            }
        }

        if (bad_ness)
        {
            LOG_ERR("Somehow De-Duplication failed to correct Shape Indices");
            setError(ASSUMPTION_WRONG);
            return;
        }
    }
}

void move_shape_indices(chunkmap* map, find_shapes_speed_stuff* stuff)
{
    LOG_INFO("Counting up Shape numbers");
    stuff->shape_counts = calloc(stuff->num_shapes * 3 + map->map_width * map->map_height, sizeof(int));
    int *increasing_shape_indices = stuff->shape_counts + stuff->num_shapes;
    stuff->shape_offsets = increasing_shape_indices + stuff->num_shapes; // An array containing the index of the first chunk of each shape in the shapes array
    stuff->chunk_index_of = stuff->shape_offsets + stuff->num_shapes;    // Stores indices into the chunkmap

    for (int y = 0; y < map->map_height; ++y)
    {
        for (int x = 0; x < map->map_width; ++x)
        {
            int current = x + y * map->map_width;
            int index = stuff->shape_ints[current];

            if (index >= stuff->num_shapes || index < 0)
            {
                LOG_ERR("Shape Int of (%d, %d) was not properly updated in De-Duplication (has value: %d)", x, y, index);
                setError(ASSUMPTION_WRONG);
                return;
            }

            ++stuff->shape_counts[index];
        }
    }

    int total_found = 0;

    for (int i = 0; i < stuff->num_shapes; ++i)
    {
        total_found += stuff->shape_counts[i];
    }
    LOG_INFO("Total Shape Count: %d", total_found);

    if (total_found != map->map_width * map->map_height)
    {
        LOG_ERR("Total Shape Count was not %u (map w x h)! Something is wrong...", map->map_width * map->map_height);
        setError(ASSUMPTION_WRONG);
        return;
    }
    {
        LOG_INFO("Calculating Shape Offsets");
        int running_index = 0;

        for (int i = 0; i < stuff->num_shapes; ++i)
        {
            increasing_shape_indices[i] = running_index;
            stuff->shape_offsets[i] = running_index;
            running_index += stuff->shape_counts[i];
        }
    }
    LOG_INFO("Moving Chunk Indices into their Shapes");

    for (int y = 0; y < map->map_height; ++y)
    {
        for (int x = 0; x < map->map_width; ++x)
        {
            int current_chunk = x + y * map->map_width;
            int *p = &increasing_shape_indices[stuff->shape_ints[current_chunk]];
            stuff->chunk_index_of[*p] = current_chunk;
            *p += 1;
        }
    }
}

void free_shape_stuff(find_shapes_speed_stuff* stuff)
{
    if (!stuff)
        return;

    if (stuff->shape_ints)
        free(stuff->shape_ints);

    stuff->shape_ints = 0;

    if (stuff->border_counts)
        free(stuff->border_counts);


    stuff->border_counts = 0;

    if (stuff->shape_counts)
        free(stuff->shape_counts);

    stuff->border_counts = 0;

    if (stuff)
        free(stuff);
}

NSVGimage* produce_nsvg(chunkmap* map, float threshold, find_shapes_speed_stuff* stuff)
{
    map->shape_count = stuff->num_shapes;
    // START CONVERT TO ACTUAL SHAPES
    chunkshape* actual_shapes = calloc(stuff->num_shapes, sizeof(chunkshape));

    for (int i = 0; i < stuff->num_shapes; ++i)
    {
        actual_shapes[i].next = (i + 1 < stuff->num_shapes ? &actual_shapes[i + 1] : NULL);
        actual_shapes[i].boundaries_length = stuff->border_counts[i];
        actual_shapes[i].boundaries = calloc(stuff->border_counts[i], sizeof(pixelchunk_list));
        actual_shapes[i].chunks_amount = stuff->border_counts[i];
        int first_chunk = stuff->chunk_index_of[stuff->shape_offsets[i]];
        actual_shapes[i].colour = map->groups_array_2d[first_chunk % map->map_width][first_chunk / map->map_width].average_colour;

        for (int j = 0; j < actual_shapes[i].boundaries_length; ++j)
        {
            int chunk_index = stuff->chunk_index_of[j + stuff->shape_offsets[i]];
            actual_shapes[i].boundaries[j].chunk_p = &map->groups_array_2d[chunk_index % map->map_width][chunk_index / map->map_width];
            actual_shapes[i].boundaries[j].next = (j + 1 < stuff->border_counts[i] ? &actual_shapes[i].boundaries[j + 1] : NULL);
        }
    }

    // END CONVERT TO ACTUAL SHAPES

    chunkshape* tmp = map->shape_list;
    map->shape_list = actual_shapes;

    sort_boundary(map);

    write_chunkmap_to_png(map, "chunkmap.png");

    if(isBadError()) {
        LOG_ERR("write_chunkmap_to_png failed with code: %d\n", getLastError());
        map->shape_list = tmp;
        free_chunkmap(map);
        free(actual_shapes);
        return NULL;
    }

    NSVGimage* nsvg = create_nsvgimage(map->map_width, map->map_height);
    iterate_chunk_shapes(map, nsvg);

    if (isBadError())
    {
        LOG_ERR("Failed to iterate chunk shapes with error: %d", getLastError());
        map->shape_list = tmp;
        free_chunkmap(map);
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

    find_shapes_speed_stuff* stuff = produce_shape_stuff(map, options.shape_colour_threshhold);

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
