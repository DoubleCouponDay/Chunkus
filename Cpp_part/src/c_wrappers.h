#pragma once

#include "color.h"
#include "3d.h"
#include "texture.h"

#include <vector>

struct pixel_group
{
    Color32 color;
    int count;
    std::vector<Vector2i> pixels;
    Vector2i lower;
    Vector2i upper;

    inline unsigned int width() const { return (unsigned int)vec_max<int>(upper.x - lower.x, 0u); }
    inline unsigned int height() const { return (unsigned int)vec_max<int>(upper.y - lower.y, 0u); }
};

struct algorithm_data
{
    std::vector<pixel_group> groups;
    int width;
    int height;
};

algorithm_data get_algorithm_data();

struct positioned_texture
{
    WomboTexture texture;
    Vector2i position;
};

struct visual_algorithm_data
{
    algorithm_data data;
    std::vector<positioned_texture> textures;
};

visual_algorithm_data generate_visuals(algorithm_data data);