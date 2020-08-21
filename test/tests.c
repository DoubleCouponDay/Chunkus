#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "munit.h"
#include "../src/entrypoint.h"
#include "../src/types/colour.h"
#include <png.h>
#include <nanosvg.h>
#define NANOSVG_IMPLEMENTATION

#define ERROR -1

#define EPIC_MAN(x, y) void Gotem(int x, )

static MunitResult aTestCanPass(const MunitParameter params[], void *data) {
  return MUNIT_OK;
}

static MunitResult itCanDecompressAPng(const MunitParameter params[], void *data) {
  char *fileaddress = "poop";

  FILE *fp = fopen(fileaddress, "rb");

  int width, height;
  png_byte color_type, bit_depth;
  png_bytep *row_pointers = NULL;

  png_image imagestruct;
  imagestruct.opaque = NULL;
  imagestruct.version = PNG_IMAGE_VERSION;
  
  png_structp png_ptr = png_create_read_struct
     (PNG_LIBPNG_VER_STRING, (png_voidp)NULL,
     NULL, NULL);

  if (!png_ptr) {
    return ERROR;
  }  
  png_infop info_ptr = png_create_info_struct(png_ptr);

  if (setjmp(png_jmpbuf(png_ptr))) return MUNIT_ERROR;

  png_init_io(png_ptr, fp);

  png_read_info(png_ptr, info_ptr);

  width = png_get_image_width(png_ptr, info_ptr);
  height = png_get_image_height(png_ptr, info_ptr);
  color_type = png_get_color_type(png_ptr, info_ptr);
  bit_depth = png_get_bit_depth(png_ptr, info_ptr);

  //image header is invalid
  if (!info_ptr)
  {
      png_destroy_read_struct(&png_ptr,
          (png_infopp)NULL, (png_infopp)NULL);
      return ERROR;
  }

  row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * height);
  for(int y = 0; y < height; y++) {
    row_pointers[y] = (png_byte*)malloc(png_get_rowbytes(png_ptr, info_ptr));
  }

  return MUNIT_OK;
  png_read_image(png_ptr, row_pointers);

  imagestruct.format = PNG_FORMAT_RGB;
  png_destroy_read_struct(&png_ptr, info_ptr, NULL);
  fclose(fp);
  return MUNIT_OK;
}

int main(int argc, char** argv) {
  MunitTest test1 = { "aTestCanPass", aTestCanPass, NULL, NULL, MUNIT_TEST_OPTION_NONE };
  MunitTest test2 = { "itCanDecompressAPng", itCanDecompressAPng, NULL, NULL, MUNIT_TEST_OPTION_NONE };

  MunitTest testarray[] = { 
    test1, 
    test2, 
    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
  };

  MunitSuite suite = { "tests.", testarray };
  return munit_suite_main(&suite, NULL, 0, argv);
}
