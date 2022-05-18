#pragma once

#include "../image.h"

#ifdef __cplusplus
extern "C" {
#endif

void write_image_to_bmp(image img, const char* fileaddress);

#ifdef __cplusplus
};
#endif
