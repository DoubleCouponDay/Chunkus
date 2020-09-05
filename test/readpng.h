#pragma once

#include "./munit.h"
#include "../src/entrypoint.h"
#include "../src/types/colour.h"
#include <png.h>
#include <nanosvg.h>
#include <errno.h>
#ifndef _WIN32
#include <dirent.h>
#endif
#include "../src/tools.h"

typedef struct {
  FILE* file;
  
  char* file_address;
} test2filesetup;


typedef struct
{  
  test2filesetup* setup;
  png_structp png_pointer;
  png_infop info_pointer;
  png_bytep* row_pointers;
  int width, height;
  png_byte color_type, bit_depth;
} test2resources;

test2resources* readfile(const MunitParameterEnum params[], void* userdata);

MunitResult itCanDecompressAPng2(const MunitParameterEnum params[], void* userdata);

void* test2setup(const MunitParameter params[], void* userdata);

void test2teardown(void* fixture);