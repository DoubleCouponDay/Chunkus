#include "bobsweep.h"

#include "utility/logger.h"
#include "utility/vec.h"
#include "imagefile/pngfile.h"
#include "mapping.h"
#include "mapparser.h"
#include "utility/error.h"
#include "sort.h"

#include <stdlib.h>
#include <stdio.h>
#include <vector>

typedef struct find_shapes_speed_stuff
{
    std::vector<int> shape_ints;
    std::vector<int> shape_counts;
    std::vector<int> shape_offsets;
    int num_shapes;
    std::vector<int> chunk_index_of;
    std::vector<int> border_counts;
    std::vector<int> border_offsets;
    std::vector<int> border_chunk_indices;
} find_shapes_speed_stuff;

void shape_initial_sweep(const chunkmap& map, find_shapes_speed_stuff& stuff, float threshold)
{
    stuff.shape_ints = std::vector<int>(map.width() * map.height());
    int next_shape_int = 0;
    LOG_INFO("Initial Shape Sweep");

    for (int x = 0; x < map.width(); ++x)
    {
        for (int y = 0; y < map.height(); ++y)
        {
            int current = x + y * map.width();
            int x_in_range = (x + 1 < map.width());
            int y_in_range = (y + 1 < map.height());
            int capped_x = (x_in_range * 2 + (!x_in_range) * 1);
            int capped_y = (y_in_range * 2 + (!y_in_range) * 1);
            
            for (int adj_x = 0 - (x > 0) * 1; adj_x < capped_x; ++adj_x)
            {
                for (int adj_y = 0 - (y > 0) * 1; adj_y < capped_y; ++adj_y)
                {
                    int similarity = map.get(x, y)->average_colour.is_similar_to(map.get(x + adj_x, y + adj_y)->average_colour, threshold);
                    int val = (((adj_x != 0) | (adj_y != 0)) * similarity * stuff.shape_ints[current + adj_x + adj_y * map.width()]);
                    stuff.shape_ints[current] = (!val * stuff.shape_ints[current]) + val;
                }
            }

            next_shape_int += 1 * (!stuff.shape_ints[current]); // Goes up every time a chunk has no similar neighbours with chunks
            int val = next_shape_int * (!stuff.shape_ints[current]);
            stuff.shape_ints[current] = (!val * stuff.shape_ints[current]) + val; // Assigns the incremented next_shape_int to this pixelchunk if it had no similar neighbours
        }
    }

    stuff.num_shapes = next_shape_int;
    LOG_INFO("Num Shapes calculated to: %d", stuff.num_shapes);
    LOG_INFO("Fixing Indices");
    
    for (int y = 0; y < map.height(); ++y)
    {
        for (int x = 0; x < map.width(); ++x)
        {
            int current = x + y * map.width();
            --stuff.shape_ints[current];
        }
    }
}

void shape_aggregate_sweep(const chunkmap& map, find_shapes_speed_stuff& stuff, float threshold)
{
    LOG_INFO("Shape Aggregation Sweep");
    int original_count = stuff.num_shapes;
    std::vector<int> de_duplicated_indices(stuff.num_shapes);
    std::vector<int> index_gaps(stuff.num_shapes);

    for (int i = 0; i < stuff.num_shapes; ++i)
        de_duplicated_indices[i] = i;
    
    for (int x = 0; x < map.width(); ++x)
    {
        for (int y = 0; y < map.height(); ++y)
        {
            int current = x + y * map.width();
            int x_in_range = (x + 1 < map.width());
            int y_in_range = (y + 1 < map.height());
            int capped_x = (x_in_range * 2 + (!x_in_range) * 1);
            int capped_y = (y_in_range * 2 + (!y_in_range) * 1);
            int skip_me = de_duplicated_indices[stuff.shape_ints[current]] == stuff.shape_ints[current];

            for (int adj_x = 0 - (x > 0) * 1; adj_x < capped_x; ++adj_x)
            {
                for (int adj_y = 0 - (y > 0) * 1; adj_y < capped_y; ++adj_y)
                {
                    int adjacent = current + adj_x + adj_y * map.width();
                    int similarity = map.get(x,y)->average_colour.is_similar_to(map.get(x + adj_x, y + adj_y)->average_colour, threshold);
                    int in_different_shapes = de_duplicated_indices[stuff.shape_ints[current]] != de_duplicated_indices[stuff.shape_ints[adjacent]];
                    int val = similarity * in_different_shapes * skip_me;
                    int already_changed = de_duplicated_indices[stuff.shape_ints[adjacent]] != stuff.shape_ints[adjacent];
                    de_duplicated_indices[stuff.shape_ints[adjacent]] = (de_duplicated_indices[stuff.shape_ints[adjacent]] * !val) + (de_duplicated_indices[stuff.shape_ints[current]] * val);
                    stuff.num_shapes += -1 * val * !already_changed;
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
    int lost_shapes = original_count - stuff.num_shapes;
    
    if (lost_shapes)
    {
        LOG_INFO("Found %d duplicate shapes", lost_shapes);
        LOG_INFO("Correcting Indices");
        
        for (int x = 0; x < map.width(); ++x)
        {
            for (int y = 0; y < map.height(); ++y)
            {
                int current = x + y * map.width();
                int de_dup_shape_index = de_duplicated_indices[stuff.shape_ints[current]];
                int val = de_dup_shape_index - index_gaps[de_dup_shape_index];
                stuff.shape_ints[current] = val;
            }
        }
    }
    
    else
    {
        LOG_INFO("No Shapes to De-Duplicate");
    }

    {
        int bad_ness = 0;
        
        for (int x = 0; x < map.width(); ++x)
        {
            for (int y = 0; y < map.height(); ++y)
            {
                int current = x + y * map.width();
                bad_ness = stuff.shape_ints[current] < 0 || stuff.shape_ints[current] >= stuff.num_shapes;
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

void move_shape_indices(const chunkmap& map, find_shapes_speed_stuff& stuff)
{
    LOG_INFO("Counting up Shape numbers");
    stuff.shape_counts = std::vector<int>(stuff.num_shapes);
    std::vector<int> increasing_shape_indices(stuff.num_shapes);
    stuff.shape_offsets = std::vector<int>(stuff.num_shapes);
    stuff.chunk_index_of = std::vector<int>(map.width() * map.height());

    for (int y = 0; y < map.height(); ++y)
    {
        for (int x = 0; x < map.width(); ++x)
        {
            int current = x + y * map.width();
            int index = stuff.shape_ints[current];

            if (index >= stuff.num_shapes || index < 0)
            {
                LOG_ERR("Shape Int of (%d, %d) was not properly updated in De-Duplication (has value: %d)", x, y, index);
                setError(ASSUMPTION_WRONG);
                return;
            }

            ++stuff.shape_counts[index];
        }
    }

    int total_found = 0;

    for (int i = 0; i < stuff.num_shapes; ++i)
    {
        total_found += stuff.shape_counts[i];
    }
    LOG_INFO("Total Shape Count: %d", total_found);

    if (total_found != map.width() * map.height())
    {
        LOG_ERR("Total Shape Count was not %u (map w x h)! Something is wrong...", map.width() * map.height());
        setError(ASSUMPTION_WRONG);
        return;
    }
    {
        LOG_INFO("Calculating Shape Offsets");
        int running_index = 0;

        for (int i = 0; i < stuff.num_shapes; ++i)
        {
            increasing_shape_indices[i] = running_index;
            stuff.shape_offsets[i] = running_index;
            running_index += stuff.shape_counts[i];
        }
    }
    LOG_INFO("Moving Chunk Indices into their Shapes");

    for (int y = 0; y < map.height(); ++y)
    {
        for (int x = 0; x < map.width(); ++x)
        {
            int current_chunk = x + y * map.width();
            int *p = &increasing_shape_indices[stuff.shape_ints[current_chunk]];
            stuff.chunk_index_of[*p] = current_chunk;
            *p += 1;
        }
    }
}

std::unique_ptr<find_shapes_speed_stuff> produce_shape_stuff(const chunkmap& map, float threshold)
{
    if (isBadError())
    {
        LOG_WARN("A Previous error occured: %d, you should bubble these errors up", getLastError());
        return nullptr;
    }
    
    if (map.width() < 1 || map.height() < 1)
    {
        LOG_ERR("Can not process empty chunkmap");
        setError(BAD_ARGUMENT_ERROR);
        return nullptr;
    }

    auto output = std::make_unique<find_shapes_speed_stuff>();

    LOG_INFO("Find Shapes Speedy with threshold: %.1f", threshold);
    shape_initial_sweep(map, *output, threshold);

    shape_aggregate_sweep(map, *output, threshold);

    if (isBadError())
    {
        LOG_ERR("Shape Aggregate Sweep failed with: %d", getLastError());
        return NULL;
    }
    
    LOG_INFO("Num Shapes Now calculated to: %d", output->num_shapes);

    move_shape_indices(map, *output);
    if (isBadError())
    {
        LOG_ERR("Moving Shape Indices failed with: %d");
        return nullptr;
    }

    {
        LOG_INFO("Checking for Duplicates");
        std::vector<unsigned char> checked_indices(map.width() * map.height());
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
            return nullptr;
        }
    }
    
    LOG_INFO("Beginning Border Detection");
    LOG_INFO("Border Counting Sweep");
    // Border Detection
    output->border_counts = std::vector<int>(output->num_shapes);
    output->border_offsets = std::vector<int>(output->num_shapes);
    std::vector<int> border_running_indices(output->num_shapes);
    std::vector<char> border_bits(map.width() * map.height());
    int border_total = 0;

    for (int i = 0; i < output->num_shapes; ++i)
    {
        for (int j = 0; j < output->shape_counts[i]; ++j)
        {
            // Look through all coordinates
            // See if any adjacents either: don't exist (edge of the image) or are a different colour
            int current = output->chunk_index_of[j + output->shape_offsets[i]];
            int x = current % map.width();
            int y = current / map.width();
            bool edge = (x < 1) | (y < 1) | (x + 1 >= map.width()) | (y + 1 >= map.height());
            bool border = edge;

            for (int adj_x = (2 * edge) + (-1 * !edge); adj_x < 2; ++adj_x)
            {
                for (int adj_y = (2 * edge) + (-1 * !edge); adj_y < 2; ++adj_y)
                {
                    border |= output->shape_ints[current] != output->shape_ints[x + adj_x + (y + adj_y) * map.width()];
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

    {
    LOG_INFO("Border Ordering Sweep");
    output->border_chunk_indices = std::vector<int>(border_total);
    int iterations = 0;
    for (int i = 0; i < output->num_shapes; ++i)
    {
        int first_chunk = output->chunk_index_of[output->shape_offsets[i]];
        int current_chunk = first_chunk;
        int x = current_chunk % map.width();
        int y = current_chunk / map.width();
        vector2 last = vector2{(float)x, (float)y};
        int pp = border_running_indices[i];
        output->border_chunk_indices[pp] = current_chunk;
        for (int j = 0; j < output->border_counts[i]; ++j)
        {
            float last_angle = 3.1415926535897982384626433f * 2.f;
            vector2 current_vec;
            current_vec.x = x;
            current_vec.y = y;
            vector2 towards_boundary{};
            int num_boundaries = 0;
            for (int adj_x = -1; adj_x < 2; ++adj_x)
            {
                for (int adj_y = -1; adj_y < 2; ++adj_y)
                {
                    int is_boundary = (x + adj_x < 0) | (x + adj_x >= map.width()) | (y + adj_y < 0) | (y + adj_y >= map.height());     
                    int corrected_index = current_chunk + adj_x * !is_boundary + (adj_y * !is_boundary) * map.width();
                    is_boundary |= output->shape_ints[corrected_index] != output->shape_ints[current_chunk]; // Different shape borders
                    num_boundaries += is_boundary;
                    towards_boundary.x += adj_x * is_boundary;
                    towards_boundary.y += adj_y * is_boundary;
                }
            }
            towards_boundary.x /= (float)(num_boundaries + 1 * !num_boundaries);
            towards_boundary.y /= (float)(num_boundaries + 1 * !num_boundaries);
            vector2 away_from_boundary = -towards_boundary;
            vector2 from_last_to_me = current_vec - last;
            vector2 sum = (from_last_to_me + away_from_boundary).normalized();
            int next_boundary_index = 0;

            
            int x_in_range = (x + 1 < map.width());
            int y_in_range = (y + 1 < map.height());
            int capped_x = (x_in_range * 2 + (!x_in_range) * 1);
            int capped_y = (y_in_range * 2 + (!y_in_range) * 1);
            for (int adj_x = -1 * (x > 0); adj_x < capped_x; ++adj_x)
            {
                for (int adj_y = -1 * (y > 0); adj_y < capped_y; ++adj_y)
                {
                    int adjacent = x + adj_x + (y + adj_y) * map.width();
                    //if (adjacent < 0 || adjacent >= total_found)
                        //LOG_ERR("Border detection Out-Of-Bounds error in adjacent block");//DEBUG
                    vector2 my_vec = vector2(adj_x, adj_y).normalized();
                    float angle = sum.angle_between(my_vec);
                    int angle_is_better = (angle < last_angle);
                    int adjacent_in_same_shape = (output->shape_ints[adjacent] == output->shape_ints[current_chunk]);
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
            output->border_chunk_indices[border_running_indices[i]] = next_boundary_index;
            border_running_indices[i] += 1 * (next_boundary_index != 0);

            //LOG_INFO("Border progress of shape %d at location (%d, %d) (iteration: %d, last: (%.2f, %.2f), sum: (%.2f, %.2f), from_last_to_me: (%.2f, %.2f), away_from_boundary: (%.2f, %.2f), # of boundaries: %d)", i, x, y, iterations, last.x, last.y, sum.x, sum.y, from_last_to_me.x, from_last_to_me.y, away_from_boundary.x, away_from_boundary.y, num_boundaries);
            current_chunk = current_chunk * (!next_boundary_index) + next_boundary_index;
            last.x = x;
            last.y = y;
            x = current_chunk % map.width();
            y = current_chunk / map.width();
            ++iterations;
        }
    }
    }

    return output;
}

void sweepfill_chunkmap(chunkmap& map, float threshold)
{
    auto stuff = produce_shape_stuff(map, threshold);

    if (!stuff)
    {
        LOG_ERR("Failed to produce shape stuff");
        if (!isBadError())
            setError(ASSUMPTION_WRONG);
        return;
    }
    // START CONVERT TO ACTUAL SHAPES

    for (int i = 0; i < stuff->num_shapes; ++i)
    {
        map.shape_list.emplace_back(std::make_shared<chunkshape>());
        auto& pp = map.shape_list.back();
        int first_chunk = stuff->chunk_index_of[stuff->shape_offsets[i]];
        pp->colour = map.get1D(first_chunk)->average_colour;

        LOG_INFO("Shape starts at (%d, %d) with colour (%d, %d, %d)", first_chunk % map.width(), first_chunk / map.width(), pp->colour.R, pp->colour.G, pp->colour.B);

        for (int j = 0; j < stuff->border_counts[i]; ++j)
        {   
            int chunk_index = stuff->border_chunk_indices[j + stuff->border_offsets[i]];
            pp->boundaries.emplace_back(map.get1D(chunk_index));
        }

        for (int j = 0; j < stuff->shape_counts[i]; ++j)
        {
            int chunk_index = stuff->chunk_index_of[j + stuff->shape_offsets[i]];
            pp->chunks.emplace_back(map.get1D(chunk_index));
        }
    }

    // END CONVERT TO ACTUAL SHAPES
    map.chunks_to_file("bobsweep chunks.png");
    map.shapes_to_file("bobsweep shapes.png");

    if(isBadError()) {
        LOG_ERR("write_chunkmap_to_png failed with code: %d\n", getLastError());
        return;
    }
    return;
}

