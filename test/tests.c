#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "munit.h"
#include "../src/entrypoint.h"
#include "../src/types/colour.h"
#include <png.h>
#define NANOSVG_IMPLEMENTATION

#define ERROR -1

static MunitResult itCanDoEverything(int argc, char** argv) {
  //decompress the png 
  //detect line boundaries
    //figure out the definition of a line boundary
    //how much difference between colours
  //define a shape in svg based on the line boundary
  //entrypoint(argc, argv);
  return MUNIT_FAIL;
}

static MunitResult itCanDecompressAPng(int argc, char** argv) {
  char* fileaddress = argv[1];

  png_image imagestruct;
  imagestruct.opaque = NULL;
  imagestruct.version = PNG_IMAGE_VERSION;
  //png_sig_cmp();
  
  png_structp png_ptr = png_create_read_struct
      (PNG_LIBPNG_VER_STRING, (png_voidp)NULL,
      NULL, NULL);

  if (!png_ptr)
      return ERROR;

  png_infop info_ptr = png_create_info_struct(png_ptr);

  if (!info_ptr)
  {
      png_destroy_read_struct(&png_ptr,
          (png_infopp)NULL, (png_infopp)NULL);
      return ERROR;
  }
  
  png_image_begin_read(&imagestruct, fileaddress);



  return MUNIT_OK;
}

int main(int argc, char** argv) {
  MunitTest test1 = { "itCanDoEverything", itCanDoEverything };
  MunitTest test2 = { "itCanDecompressAPng", itCanDecompressAPng };
  MunitTest testarray[] = { test2, {} };
  MunitSuite suite1 = { "tests.", testarray };
  return munit_suite_main(&suite1, NULL, sizeof(testarray) / sizeof(MunitTest), argv);
}
