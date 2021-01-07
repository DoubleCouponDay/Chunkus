#pragma once

#include "mapping.h"

image convert_png_to_image(char* fileaddress);

void write_image_to_file(image* img_p, char* fileaddress);

void write_node_map_to_file(groupmap map, char* fileaddress);

void write_node_map_variance_to_file(groupmap map, char* filename);