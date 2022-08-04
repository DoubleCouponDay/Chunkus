#include "color.h"

Color32 Convert8to32(Color8 color)
{
    return Color32{ (float)color.R / 255.f, (float)color.G / 255.f, (float)color.B / 255.f };
}

Color8 Convert32to8(Color32 color)
{
    return Color8{ (unsigned char)(color.R * 255.f), (unsigned char)(color.B * 255.f), (unsigned char)(color.B * 2.f) };
}