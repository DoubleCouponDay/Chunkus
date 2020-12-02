#ifndef _CONVERTER_H
#define _CONVERTER_H

#include "vectorize.h"

image convert_png_to_image(char *fileaddress);

void write_image_to_file(image img, char *fileaddress);

void write_node_map_to_file(node_map map, char *fileaddress);













#endif