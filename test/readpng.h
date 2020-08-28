#pragma once

#include "./munit.h"
#include "../src/entrypoint.h"
#include "../src/types/colour.h"
#include <png.h>
#include <nanosvg.h>
#include <errno.h>
#include <dirent.h>
#include "../src/tools.h"


typedef struct
{
  char* file_address;
  FILE *fp;
  png_structp png_ptr;
  png_infop info_ptr;
  png_bytep *row_pointers;
  int width, height;
  png_byte color_type, bit_depth;
} test_resources;

MunitResult itCanDecompressAPng2(const MunitParameterEnum params[], void *user_data);

void* test2setup(const MunitParameter params[], void *user_data);

void test2teardown(void* fixture);