#include "sort.h"

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "utility/defines.h"
#include <math.h>
#include <vector>

#include "chunkmap.h"
#include "utility/error.h"
#include "utility/vec.h"
#include "utility/logger.h"
#include "prune.h"

enum {
    ADJACENT_COUNT = 9
};

vector2 calculate_2d_diff(pixelchunk& subject, pixelchunk& previous) {
    int x_diff = subject.location.x - previous.location.x;
    int y_diff = subject.location.y - previous.location.y;
    vector2 diff = vector2{ (float)x_diff, (float)y_diff };
    return diff;
}

float calculate_angle_between(pixelchunk& eligible, pixelchunk& subject, pixelchunk& previous) {
    int eligible_x_diff = eligible.location.x - subject.location.x;
    int eligible_y_diff = eligible.location.y - subject.location.y;
    vector2 subject_to_eligible = vector2{ (float)eligible_x_diff, (float)eligible_y_diff };
    vector2 diff = calculate_2d_diff(subject, previous);
    float angle = diff.angle_between(subject_to_eligible);
    return angle;
}

void sort_item(std::vector<std::shared_ptr<pixelchunk>>& array, unsigned long a, unsigned long b) {
    std::swap(array[a], array[b]);
}

void dont_skip_corners(std::vector<std::shared_ptr<pixelchunk>>& array, unsigned long eligiblesubjects[ADJACENT_COUNT], pixelchunk& subject, pixelchunk& previous, 
                        unsigned long eligible_count, unsigned long next) {

    float smallest_angle = M_PI * 2.f; // set to largest possible radian to begin with
    std::shared_ptr<pixelchunk> most_eligible = NULL;
    unsigned long most_eligible_index = 0;

    for (unsigned long q = 0; q < eligible_count; ++q) {
        auto eligible = array[eligiblesubjects[q]];
        float angle = calculate_angle_between(*eligible, subject, previous);

        if (angle < smallest_angle)
        {
            smallest_angle = angle;
            most_eligible = eligible;
            most_eligible_index = eligiblesubjects[q];
        }
    }

    if (most_eligible)
    {
        sort_item(array, most_eligible_index, next);
    }

    else
    {
        return;
    }
}

void bubble_sort(std::vector<std::shared_ptr<pixelchunk>>& array, unsigned long start) {
    bool allsorted = false;

    while(allsorted == false) {        
        unsigned long next = start + 1;

        if(next >= array.size()) {
            allsorted = true;
            return;
        }
        unsigned long eligiblesubjects[ADJACENT_COUNT] = {0};
        pixelchunk& subject = *array[start];
        unsigned long eligible_count = 0;
        std::shared_ptr<pixelchunk> previous = (start ? array[start - 1] : nullptr);

        for(unsigned long i = start; i < array.size(); ++i) {
            pixelchunk& current = *array[i];

            if(chunk_is_adjacent(current, subject)) {
                if(previous == NULL) {
                    sort_item(array, i, next);
                    break;
                }

                else if(eligible_count == ADJACENT_COUNT) {
                    LOG_ERR("adjacent chunks are larger than known size!");
                    setError(ASSUMPTION_WRONG);
                    return;
                }
                eligiblesubjects[eligible_count] = i;
                ++eligible_count;            
            }

            else {

            }

            if(i == array.size() - 1) 
            {
                dont_skip_corners(array, eligiblesubjects, subject, *previous, eligible_count, next);
            }
        }
        ++start;
    }
}

void sort_boundary(chunkmap& map) {
    // chunkshape* shape = map->shape_list;

    // while (shape)
    // {
    //     pixelchunk** array = convert_boundary_list_toarray(shape->boundaries, shape->boundaries_length);
    //     bubble_sort(array, 0, shape->boundaries_length);

    //     if(isBadError()) {
    //         DEBUG("bubble_sort failed with code: %d\n", getLastError());
    //         return;
    //     }
    //     convert_array_to_boundary_list(array, shape->boundaries, shape->boundaries_length);
    //     prune_boundary(shape->boundaries);       
    //     shape = shape->next;
    //     free(array);
    // }

    for (auto& item : map.shape_list)
    {
        auto vec = std::vector<std::shared_ptr<pixelchunk>>(item->boundaries.begin(), item->boundaries.end());
        bubble_sort(vec, 0);
        item->boundaries = std::move(std::list<std::shared_ptr<pixelchunk>>(vec.begin(), vec.end()));
        prune_boundary(item->boundaries);
    }
}
