#pragma once

#include "../image.h"
#include <stdio.h>

bool file_is_jpeg(char* fileaddress);
image convert_jpeg_to_image(char* fileaddress);

