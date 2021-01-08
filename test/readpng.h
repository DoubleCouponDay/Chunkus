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
} filesetup;


typedef struct
{  
  filesetup* setup;
  png_structp png_pointer;
  png_infop info_pointer;
  png_bytep* row_pointers;
  int width, height;
  png_byte color_type, bit_depth;
} fileresources;

fileresources* readfile(const MunitParameter params[], filesetup* setup);

filesetup* createfilesetup(const MunitParameter params[], void* userdata);

void freefile(fileresources* resources);