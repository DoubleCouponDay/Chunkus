#include "color.h"

#include <cmath>

Color32 Convert8to32(Color8 color)
{
    return Color32{ (float)color.R / 255.f, (float)color.G / 255.f, (float)color.B / 255.f };
}

Color8 Convert32to8(Color32 color)
{
    int r = roundf(color.R * 255.f);
    int g = roundf(color.G * 255.f);
    int b = roundf(color.B * 255.f);
    return Color8{ (unsigned char)r, (unsigned char)g, (unsigned char)b };
}