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

MunitResult itCanDecompressAPng2(const MunitParameterEnum params[], void *user_data) {
  test_resources* kit = user_data;
  char* fileaddress = kit->file_address;
  DEBUG_PRINT("%s\n", fileaddress);
  FILE* fp = kit->fp;

  munit_assert_not_null(fp);

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
  DEBUG_PRINT("Pre Create Info Struct\n");
  png_infop info_ptr = png_create_info_struct(png_ptr);

  if (setjmp(png_jmpbuf(png_ptr))) return MUNIT_ERROR;

  DEBUG_PRINT("Pre Init IO\n");
  png_init_io(png_ptr, fp);

  DEBUG_PRINT("Pre Read Info\n");
  png_read_info(png_ptr, info_ptr);

  DEBUG_PRINT("Pre Get dimensions\n");
  width = png_get_image_width(png_ptr, info_ptr);
  height = png_get_image_height(png_ptr, info_ptr);

  DEBUG_PRINT("Width: %d, Height: %d\n", width, height);
  color_type = png_get_color_type(png_ptr, info_ptr);
  bit_depth = png_get_bit_depth(png_ptr, info_ptr);  

  //image header is invalid
  if (!info_ptr)
  {
      png_destroy_read_struct(&png_ptr,
          (png_infopp)NULL, (png_infopp)NULL);
      return ERROR;
  }
  DEBUG_PRINT("image is valid\n");
  row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * height);
  for(int y = 0; y < height; y++) {
    row_pointers[y] = (png_byte*)malloc(png_get_rowbytes(png_ptr, info_ptr));
  }
  DEBUG_PRINT("read all the bytes\n");
  imagestruct.format = PNG_FORMAT_RGB;

  png_read_image(png_ptr, row_pointers);
  DEBUG_PRINT("read the image\n");

  png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
  

  return MUNIT_OK;
}

void* test2setup(const MunitParameter params[], void *user_data)
{
  // return address of global variable, or allocate on the heap
  static test_resources g_Resources;
  DEBUG_PRINT("setting test_resources memory... \n");
  memset(&g_Resources, 0, sizeof(test_resources));

  // Open file, create png_struct, create info_struct
  DEBUG_PRINT("finding the file param \n");
  // Find file address parameter
  char* file_address = params[0].value;

  DEBUG_PRINT("Now checking file address \n");

  g_Resources.fp = fopen(file_address, "rb");

  if (g_Resources.fp == NULL)
  {
    DEBUG_PRINT("File pointer was null");
    return NULL;
  }
  
  DEBUG_PRINT("Setup 2 complete....\n");
  return &g_Resources;
}

void test2teardown(void* fixture) {
  if (fixture == NULL)
    return;

  test_resources* resources = (test_resources*)fixture;

  // Free the buffer
  for (int i = 0; i < resources->height; ++i)
  {
    free(resources->row_pointers[i]);
  }

  if (resources->fp)
    fclose(resources->fp);
}
