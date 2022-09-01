#pragma once

#include "../image.h"
#include <stdio.h>

bool file_is_jpeg(const char* fileaddress);
image convert_jpeg_to_image(const char* fileaddress);

