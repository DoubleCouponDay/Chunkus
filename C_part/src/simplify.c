#include "simplify.h"

#include <stdlib.h>
#include "image.h"
#include "../test/debug.h"
#include "utility/error.h"

const int TOTAL_COLOURS = 256;

byte quantize_int(int subject, int divisions) {
    // 106 % 10 = 6
    // 106 - 6 = 100 which is a multiple of 10
    // 106 + (10 - 6) = 110
    // 27 % 5 = 2
    // 7 % 5 = 2
    
    int r_mod = subject % divisions;
    int difference = divisions - (subject % divisions);

    if ((subject - r_mod) < 0 || (subject + difference) > 255)
        return subject;

    if ((r_mod) < (divisions / 2))
        subject -= r_mod; // Remove the remainder to make it a multiple of the division

    else {        
        subject += difference;
    }
    return (byte)subject;
}

void quantize_image(image* subject, int num_colours) {
    if(num_colours < 0 ||
        num_colours > TOTAL_COLOURS) {
        DEBUG("num colours out of bounds!\n");
        setError(BAD_ARGUMENT_ERROR);
        return;
    }
    DEBUG("simplifying colour scheme to %d colours\n", num_colours);
    int divisions = TOTAL_COLOURS / num_colours;

    for(int x = 0; x < subject->width; ++x) {
        for(int y = 0; y < subject->height; ++y) {
            pixel* pix = &subject->pixels_array_2d[x][y];
            pix->r = quantize_int(pix->r, divisions);
            pix->g = quantize_int(pix->g, divisions);
            pix->b = quantize_int(pix->b, divisions);
        }
    }
}

