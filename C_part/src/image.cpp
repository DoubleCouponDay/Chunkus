#include "image.h"

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <nanosvg.h>

#include "utility/error.h"
#include "utility/vec.h"

#include "imagefile/pngfile.h"


bool pixel::is_similar_to(const pixel& other, float threshold)
{
    return pixelInt(*this).is_similar_to(other, threshold);
}

bool pixelInt::is_similar_to(const pixelInt& other, float threshold)
{
    pixelInt diff{R - other.R, G - other.G, B - other.B};

    float mag = sqrtf((int)(diff.R * diff.R + diff.G * diff.G + diff.B * diff.B));

    return mag <= threshold;
}

bool image::to_png(char *file) const
{
    write_image_to_png(*this, file);

    return getAndResetErrorCode() != SUCCESS_CODE;
}

pixelD pixelD::lerp(pixelD a, pixelD b, float t)
{
    return pixelD(::lerp(a.R, b.R, t), ::lerp(a.G, b.G, t), ::lerp(a.B, b.B, t));
}

pixelF pixelF::lerp(pixelF a, pixelF b, float t)
{
    return pixelF(::lerp(a.R, b.R, t), ::lerp(a.G, b.G, t), ::lerp(a.B, b.B, t));
}