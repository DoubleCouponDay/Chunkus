#pragma once

#include "aggregating.h"

#include <stdlib.h>
#include <stdio.h>

#include "../chunkmap.h"
#include "splitting.h"

uint64_t getslice(split* input, int startindex, int endindex, int width, int height) {
    if(endindex - startindex == 64) {
        uint64t one =           ((uint64_t)input.nodes[startindex][height].is_boundary) << 63);
        uint64t two =           ((uint64_t)input.nodes[startindex + 1][height].is_boundary) << 62);
        uint64t three =         ((uint64_t)input.nodes[startindex + 2][height].is_boundary) << 61);
        uint64t four =          ((uint64_t)input.nodes[startindex + 3][height].is_boundary) << 60);
        uint64t five =          ((uint64_t)input.nodes[startindex + 4][height].is_boundary) << 59);
        uint64t six =           ((uint64_t)input.nodes[startindex + 5][height].is_boundary) << 58);
        uint64t seven =         ((uint64_t)input.nodes[startindex + 6][height].is_boundary) << 57);
        uint64t eight =         ((uint64_t)input.nodes[startindex + 7][height].is_boundary) << 56);
        uint64t nine =          ((uint64_t)input.nodes[startindex + 8][height].is_boundary) << 55);
        uint64t ten =           ((uint64_t)input.nodes[startindex + 9][height].is_boundary) << 54);
        uint64t eleven =        ((uint64_t)input.nodes[startindex + 10][height].is_boundary) << 53);
        uint64t twelve =        ((uint64_t)input.nodes[startindex + 11][height].is_boundary) << 52);
        uint64t thirteen =      ((uint64_t)input.nodes[startindex + 12][height].is_boundary) << 51);
        uint64t fourteen =      ((uint64_t)input.nodes[startindex + 13][height].is_boundary) << 50);
        uint64t fifteen =       ((uint64_t)input.nodes[startindex + 14][height].is_boundary) << 49);
        uint64t sixteen =       ((uint64_t)input.nodes[startindex + 15][height].is_boundary) << 48);
        uint64t seventeen =     ((uint64_t)input.nodes[startindex + 16][height].is_boundary) << 47);
        uint64t eighteen =      ((uint64_t)input.nodes[startindex + 17][height].is_boundary) << 46);
        uint64t nineteen =      ((uint64_t)input.nodes[startindex + 18][height].is_boundary) << 45);
        uint64t twenty =        ((uint64_t)input.nodes[startindex + 19][height].is_boundary) << 44);
        uint64t twentyone =     ((uint64_t)input.nodes[startindex + 20][height].is_boundary) << 43);
        uint64t twentytwo =     ((uint64_t)input.nodes[startindex + 21][height].is_boundary) << 42);
        uint64t twentythree =   ((uint64_t)input.nodes[startindex + 22][height].is_boundary) << 41);
        uint64t twentyfour =    ((uint64_t)input.nodes[startindex + 23][height].is_boundary) << 40);
        uint64t twentyfive =    ((uint64_t)input.nodes[startindex + 24][height].is_boundary) << 39);
        uint64t twentysix =     ((uint64_t)input.nodes[startindex + 25][height].is_boundary) << 38);
        uint64t twentyseven =   ((uint64_t)input.nodes[startindex + 26][height].is_boundary) << 37);
        uint64t twentyeight =   ((uint64_t)input.nodes[startindex + 27][height].is_boundary) << 36);
        uint64t twentynine =    ((uint64_t)input.nodes[startindex + 28][height].is_boundary) << 35);
        uint64t thirty =        ((uint64_t)input.nodes[startindex + 29][height].is_boundary) << 34);
        uint64t thirtyone =     ((uint64_t)input.nodes[startindex + 30][height].is_boundary) << 33);
        uint64t thirtytwo =     ((uint64_t)input.nodes[startindex + 31][height].is_boundary) << 32);
        uint64t thirtythree =   ((uint64_t)input.nodes[startindex + 32][height].is_boundary) << 31);
        uint64t thirtyfour =    ((uint64_t)input.nodes[startindex + 33][height].is_boundary) << 30);
        uint64t thirtyfive =    ((uint64_t)input.nodes[startindex + 34][height].is_boundary) << 29);
        uint64t thirtysix =     ((uint64_t)input.nodes[startindex + 35][height].is_boundary) << 28);
        uint64t thirtyseven =   ((uint64_t)input.nodes[startindex + 36][height].is_boundary) << 27);
        uint64t thirtyeight =   ((uint64_t)input.nodes[startindex + 37][height].is_boundary) << 26);
        uint64t thirtynine =    ((uint64_t)input.nodes[startindex + 38][height].is_boundary) << 25);
        uint64t forty =         ((uint64_t)input.nodes[startindex + 39][height].is_boundary) << 24);
        uint64t fortyone =      ((uint64_t)input.nodes[startindex + 40][height].is_boundary) << 23);
        uint64t fortytwo =      ((uint64_t)input.nodes[startindex + 41][height].is_boundary) << 22);
        uint64t fortythree =    ((uint64_t)input.nodes[startindex + 42][height].is_boundary) << 21);
        uint64t fortyfour =     ((uint64_t)input.nodes[startindex + 43][height].is_boundary) << 20);
        uint64t fortyfive =     ((uint64_t)input.nodes[startindex + 44][height].is_boundary) << 19);
        uint64t fortysix =      ((uint64_t)input.nodes[startindex + 45][height].is_boundary) << 18);
        uint64t fortyseven =    ((uint64_t)input.nodes[startindex + 46][height].is_boundary) << 17);
        uint64t fortyeight =    ((uint64_t)input.nodes[startindex + 47][height].is_boundary) << 16);
        uint64t fortynine =     ((uint64_t)input.nodes[startindex + 48][height].is_boundary) << 15);
        uint64t fifty =         ((uint64_t)input.nodes[startindex + 49][height].is_boundary) << 14);
        uint64t fiftyone =      ((uint64_t)input.nodes[startindex + 50][height].is_boundary) << 13);
        uint64t fiftytwo =      ((uint64_t)input.nodes[startindex + 51][height].is_boundary) << 12);
        uint64t fiftythree =    ((uint64_t)input.nodes[startindex + 52][height].is_boundary) << 11);
        uint64t fiftyfour =     ((uint64_t)input.nodes[startindex + 53][height].is_boundary) << 10);
        uint64t fiftyfive =     ((uint64_t)input.nodes[startindex + 54][height].is_boundary) << 9);
        uint64t fiftysix =      ((uint64_t)input.nodes[startindex + 55][height].is_boundary) << 8);
        uint64t fiftyseven =    ((uint64_t)input.nodes[startindex + 56][height].is_boundary) << 7);
        uint64t fiftyeight =    ((uint64_t)input.nodes[startindex + 57][height].is_boundary) << 6);
        uint64t fiftynine =     ((uint64_t)input.nodes[startindex + 58][height].is_boundary) << 5);
        uint64t sixty =         ((uint64_t)input.nodes[startindex + 59][height].is_boundary) << 4);
        uint64t sixtyone =      ((uint64_t)input.nodes[startindex + 60][height].is_boundary) << 3);
        uint64t sixtytwo =      ((uint64_t)input.nodes[startindex + 61][height].is_boundary) << 1);
        uint64t sixtythree =    ((uint64_t)input.nodes[startindex + 62][height].is_boundary) << 1);
        uint64t sixtyfour =     ((uint64_t)input.nodes[startindex + 63][height].is_boundary));
    }
}

/// @brief returns a 2D array of ints
/// @param input 
/// @param width 
/// @param height 
/// @return 
uint64_t* OR_64(splits* input, int width, int height) {
    //1. take each row in 64 bit slices
    //2. make 8 variables for each slice
    //3. bitwise OR them together
    //4. write to a single chunkmap
    //5. do it for every slice in the current row
    bool** output = calloc(1, sizeof(bool) * height);

    for(int y = 0; y < height; ++y) {
        int startindex = 0;
        int endindex = 0;
        int widthindex = width - 1;
        output[y] = calloc(1, sizeof(bool) * width);

        while(endindex < width) {   
            endindex = endindex + 64;

            if(endindex >= widthindex) {
                endindex = widthindex;
            }
            uint64_t split1slice = getslice(input.splits[0], startindex, endindex);
            uint64_t split2slice = getslice(input.splits[1], startindex, endindex);
            uint64_t split3slice = getslice(input.splits[2], startindex, endindex);
            uint64_t split4slice = getslice(input.splits[3], startindex, endindex);
            uint64_t split5slice = getslice(input.splits[4], startindex, endindex);
            uint64_t split6slice = getslice(input.splits[5], startindex, endindex);
            uint64_t split7slice = getslice(input.splits[6], startindex, endindex);
            uint64_t split8slice = getslice(input.splits[7], startindex, endindex);
            uint64_t aggregate = split1slice | split2slice | split3slice | split4slice | split5slice | split6slice | split7slice | split8slice;

            startindex = endindex + 1;
            endindex = 
        }
    }
}
