#include "aggregating.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "../chunkmap.h"
#include "splitting.h"
#include "../utility/logger.h"
#include "../utility/error.h"

uint64_t getslice(split input, int startindex, int endindex, int height) {
    LOG_INFO("");
    int size = endindex - startindex;

    uint64_t output;
    int bitshift = 63;
    int offset = 0;

    if(size >= bitshift) { //size cannot be larger than 64
        LOG_ERR("ERROR: buffer size is larger than processor architecture");
        setError(ASSUMPTION_WRONG);
        return -1;
    }
    
    for(int i = 0 ; i < size; ++i) { //if size loops 64 times max it will guarantee the correct max amount of loops
        uint64_t currentbuffer = ((uint64_t)(input.nodes[startindex + offset][height].is_boundary)) << bitshift;
        output |= currentbuffer;
        --bitshift; //bitshift caps out at 0
        ++offset;
    }
    return output;
}

uint64_t aggregatedata(splits* input, int startindex, int endindex, int y) {
    LOG_INFO("");
    uint64_t splitslice1 = getslice(input->splits[0], startindex, endindex, y);
    uint64_t splitslice2 = getslice(input->splits[1], startindex, endindex, y);
    uint64_t splitslice3 = getslice(input->splits[2], startindex, endindex, y);
    uint64_t splitslice4 = getslice(input->splits[3], startindex, endindex, y);
    uint64_t splitslice5 = getslice(input->splits[4], startindex, endindex, y);
    uint64_t splitslice6 = getslice(input->splits[5], startindex, endindex, y);
    uint64_t splitslice7 = getslice(input->splits[6], startindex, endindex, y);
    uint64_t splitslice8 = getslice(input->splits[7], startindex, endindex, y);

    #if SIMD_X86_64
    #include <mmintrin.h>
    __m64 aggregated = _mm_or_si64(splitslice1, splitslice2); //using MMX extensions
    __m64 aggregated2 = _mm_or_si64(aggregated, splitslice3);
    __m64 aggregated3 = _mm_or_si64(aggregated2, split2slice4);
    __m64 aggregated4 = _mm_or_si64(aggregated3, split2slice5);
    __m64 aggregated5 = _mm_or_si64(aggregated4, split2slice6);
    __m64 aggregated6 = _mm_or_si64(aggregated5, split2slice7);
    __m64 aggregated_final = _mm_or_si64(aggregated6, split2slice8);
    
    #elif SIMD_ARM
    #include <arm_neon.h>
    uint64x1_t aggregated = vorr_u64(splitslice1, splitslice2); //using NEON extensions
    uint64x1_t aggregated2 = vorr_u64(aggregated, splitslice3);
    uint64x1_t aggregated3 = vorr_u64(aggregated2, splitslice4);
    uint64x1_t aggregated4 = vorr_u64(aggregated3, splitslice5);
    uint64x1_t aggregated5 = vorr_u64(aggregated4, splitslice6);
    uint64x1_t aggregated6 = vorr_u64(aggregated5, splitslice7);
    uint64x1_t aggregated_final = vorr_u64(aggregated6, splitslice8);

    #else
    uint64_t aggregated_final = splitslice1 | splitslice2 | splitslice3 | splitslice4 | splitslice5 | splitslice6 | splitslice7 | splitslice8;
    #endif
    
    LOG_INFO("aggregate: %d", aggregated_final);
    return aggregated_final;
}

/// @brief returns a 2D array of bools
/// @param input 
/// @param width 
/// @param height 
/// @return 
bool** OR_64(splits* input, int width, int height) {
    //1. take each row in 64 bit slices
    //2. make 8 variables for each slice
    //3. bitwise OR them together
    //4. write to a single chunkmap
    //5. do it for every slice in the current row
    LOG_INFO("");
    bool** output = calloc(1, sizeof(bool*) * height);

    for(int y = 0; y < height; ++y) {
        int startindex = 0;
        int endindex = startindex + MAX_BITS;
        int column_size = ceil(width);
        LOG_INFO("column_size: %d", column_size);
        output[y] = calloc(1, column_size);

        //write 64b aggregates
        while(endindex < width) {
            uint64_t aggregate = aggregatedata(input, startindex, endindex, y);
            uint64_t* aggregate_p = &aggregate;
            memcpy(output[y], aggregate_p, sizeof(uint64_t));
            *output[y] = aggregate;
            startindex = endindex + 1;
            endindex = endindex + MAX_BITS;
        }
        //write the remainder aggregate
        uint64_t remainder = aggregatedata(input, startindex, endindex, y);
        uint64_t* remainder_p = &remainder;
        int remaindersize = endindex - startindex;
        memcpy(output[y], remainder_p, sizeof(bool) * remaindersize);
        LOG_INFO("output[y]: %s", output[y]);
    }
    return output;
}

void free_aggregate(bool** input, int width) {
    LOG_INFO("");

    if(input == NULL) {
        return;
    }
    for(int x = 0; x < width; ++x) {
        bool* currentX = input[width];
        free(currentX);
    }
    free(input);
}
