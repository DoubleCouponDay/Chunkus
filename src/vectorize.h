#ifndef _VECTORIZE_H

typedef char byte;

typedef struct
{
    byte r;
    byte g;
    byte b;
} Color;

typedef struct
{
    Color *pixels;
    int width;
    int height;
} image;

typedef Color node_variance;

typedef struct 
{
    Color color;
    node_variance variance;
} node;

typedef struct 
{
    node *nodes;
    int width;
    int height;
} node_map;

typedef struct
{
    int chunk_size;
} node_map_options;

node_map generate_epic_map(image from, node_map_options options);
Color calculate_color_variance(Color *colors, int num_colors);

#endif