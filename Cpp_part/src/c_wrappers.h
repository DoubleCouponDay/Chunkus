#pragma once

#include "color.h"
#include "3d.h"
#include "texture.h"

#include <vector>

// A colored group of pixels
struct pixel_group
{
    Color32 color;
    std::vector<Vector2i> pixels; // The pixels in the group (coords are relative to algorithm data not group)
    Vector2i lower; // Lower bounds of the group
    Vector2i upper; // Upper bounds of the group

    inline unsigned int width() const { return (unsigned int)vec_max<int>(upper.x - lower.x, 0u); }
    inline unsigned int height() const { return (unsigned int)vec_max<int>(upper.y - lower.y, 0u); }
};

// Data expected from the vectorizer algorithm.
struct algorithm_data
{
    std::vector<pixel_group> groups;
    int width;
    int height;
};

// Placeholder function to get algorithm_data
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
    WomboTexture complete_texture;
};

visual_algorithm_data generate_visuals(algorithm_data data);