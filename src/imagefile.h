#pragma once

#include "mapping.h"

image convert_png_to_image(char* fileaddress);

void write_image_to_file(image img, char* fileaddress);

void write_node_map_to_file(chunkmap map, char* fileaddress);
