#include "image.h"

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <nanosvg.h>

#include "utility/error.h"

#include "imagefile/pngfile.h"


bool pixel::is_similar_to(const pixel& other, float threshold)
{
    return pixelInt(*this).is_similar_to(other, threshold);
}

bool pixelInt::is_similar_to(const pixelInt& other, float threshold)
{
    pixelInt diff{R - other.R, G - other.G, B - other.B};

    float mag = sqrtf(diff.R * diff.R + diff.G * diff.G + diff.B * diff.B);

    return mag <= threshold;
}

bool image::to_png(char *file) const
{
    write_image_to_png(*this, file);

    return getAndResetErrorCode() != SUCCESS_CODE;
}