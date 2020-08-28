#pragma once

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "munit.h"
#include "../src/entrypoint.h"
#include "../src/types/colour.h"
#include <png.h>
#include <nanosvg.h>
#include <errno.h>
#include <dirent.h>
#include "../src/tools.h"
#include "./readpng.h"
#define NANOSVG_IMPLEMENTATION
#define ERROR -1

test2resources* readfile(const MunitParameterEnum params[], void* userdata) {
  DEBUG_PRINT("reading file... \n");
  test2resources* resources = calloc(1, sizeof(test2resources));
  resources->setup = userdata;
  munit_assert_not_null(resources);
  char* fileaddress = resources->setup->file_address;
  DEBUG_PRINT("fileaddress = %s\n", fileaddress);
  FILE* file = resources->setup->file;
  munit_assert_not_null(file);

  int width, height;
  png_byte color_type, bit_depth;
  png_bytep* row_pointers = NULL;

  png_image imagestruct;
  imagestruct.opaque = NULL;
  imagestruct.version = PNG_IMAGE_VERSION;
  
  png_structp png_ptr = png_create_read_struct
     (PNG_LIBPNG_VER_STRING, (png_voidp)NULL,
     NULL, NULL);

  if (!png_ptr) {
    return ERROR;
  }  
  resources->png_pointer = png_ptr;

  DEBUG_PRINT("Pre Create Info Struct\n");
  png_infop info_ptr = png_create_info_struct(png_ptr);
  resources->info_pointer = info_ptr;

  if (setjmp(png_jmpbuf(png_ptr))) return MUNIT_ERROR;

  DEBUG_PRINT("Pre Init IO\n");
  png_init_io(png_ptr, file);

  DEBUG_PRINT("Pre Read Info\n");
  png_read_info(png_ptr, info_ptr);

  DEBUG_PRINT("Pre Get dimensions\n");
  width = png_get_image_width(png_ptr, info_ptr);
  height = png_get_image_height(png_ptr, info_ptr);
  resources->width = width;
  resources->height = height;

  DEBUG_PRINT("Width: %d, Height: %d\n", width, height);
  color_type = png_get_color_type(png_ptr, info_ptr);
  bit_depth = png_get_bit_depth(png_ptr, info_ptr); 
  resources->color_type = color_type;
  resources->bit_depth = bit_depth;

  //image header is invalid
  if (!info_ptr)
  {
      png_destroy_read_struct(&png_ptr,
          (png_infopp)NULL, (png_infopp)NULL);

      *NULL; //throw segfault
  }
  DEBUG_PRINT("image is valid\n");
  resources->row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * height);

  for(int y = 0; y < height; y++) {
    resources->row_pointers[y] = (png_byte*)malloc(png_get_rowbytes(png_ptr, info_ptr));
  }
  DEBUG_PRINT("read the image\n");
  imagestruct.format = PNG_FORMAT_RGB;
  png_read_image(png_ptr, resources->row_pointers);


  DEBUG_PRINT("About to return populated resources\n");
  return resources;
}

MunitResult itCanDecompressAPng2(const MunitParameterEnum params[], void* userdata) {
  test2resources* resources = readfile(params, userdata);
  freefile(resources);
  return MUNIT_OK;
}

void freefile(test2resources* resources) {
  png_destroy_read_struct(&resources->png_pointer, &resources->info_pointer, NULL);
  for (int i = 0; i < resources->height; ++i)
    free(resources->row_pointers[i]);
  free(resources);
}

void* test2setup(const MunitParameter params[], void* userdata)
{
  // return address of global variable, or allocate on the heap
  test2filesetup* setup = calloc(1, sizeof(test2filesetup));
  
  DEBUG_PRINT("setting test2filesetup memory... \n");
  //memset(&setup, 0, sizeof(test2filesetup));

  // Open file, create png_struct, create info_struct
  DEBUG_PRINT("finding the file param \n");
  // Find file address parameter
  char* file_address = params[0].value;

  DEBUG_PRINT("Now checking file address \n");

  setup->file = fopen(file_address, "rb");

  if (setup->file == NULL)
  {
    DEBUG_PRINT("File pointer was null");
    return NULL;
  }
  
  DEBUG_PRINT("Setup 2 complete....\n");
  return setup;
}

void test2teardown(void* fixture) {
  if (fixture == NULL)
    return;

  test2filesetup* file_setup = (test2filesetup*)fixture;

  if (file_setup->file)
    fclose(file_setup->file);
}
