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
#define NANOSVG_IMPLEMENTATION

#define ERROR -1

//https://devdocs.io/c/error/errno

#define EPIC_MAN(x, y) void Gotem(int x, )

static MunitResult aTestCanPass(const MunitParameterEnum params[], void *data) {
  return MUNIT_OK;
}

static MunitResult itCanDecompressAPng(const MunitParameterEnum params[], void *data) {
  char *fileaddress = params[0].values[0];
  printf("%s\n", fileaddress);
  FILE *fp = fopen(fileaddress, "r");

  munit_assert_not_null(fp);

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
  printf("Pre Create Info Struct\n");
  png_infop info_ptr = png_create_info_struct(png_ptr);

  if (setjmp(png_jmpbuf(png_ptr))) return MUNIT_ERROR;

  printf("Pre Init IO\n");
  png_init_io(png_ptr, fp);

  printf("Pre Read Info\n");
  png_read_info(png_ptr, info_ptr);

  printf("Pre Get dimensions\n");
  width = png_get_image_width(png_ptr, info_ptr);
  height = png_get_image_height(png_ptr, info_ptr);

  printf("Width: %d, Height: %d\n", width, height);
  color_type = png_get_color_type(png_ptr, info_ptr);
  bit_depth = png_get_bit_depth(png_ptr, info_ptr);

  //image header is invalid
  if (!info_ptr)
  {
      png_destroy_read_struct(&png_ptr,
          (png_infopp)NULL, (png_infopp)NULL);
      return ERROR;
  }
  printf("image is valid\n");
  row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * height);
  for(int y = 0; y < height; y++) {
    row_pointers[y] = (png_byte*)malloc(png_get_rowbytes(png_ptr, info_ptr));
  }
  printf("read all the bytes\n");
  imagestruct.format = PNG_FORMAT_RGB;

  png_read_image(png_ptr, row_pointers);
  printf("read the image\n");

  png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
  if (fp)
    fclose(fp);
  

  return MUNIT_OK;
}

typedef struct
{
  FILE *fp;
  png_structp png_ptr;
  png_infop info_ptr;
  png_bytep *row_pointers;
  int width, height;
  png_byte color_type, bit_depth;
} test_resources;

void *test2setup(const MunitParameter params[], void *user_data)
{
  // return address of global variable, or allocate on the heap
  static test_resources g_Resources = NULL;

  // Open file, create png_struct, create info_struct

  // Find file address parameter
  char *file_address = "wtf_piss_off";
  for (int i = 0; params[i].name != NULL; ++i)
  {
    if (strcmp(params[i].name, "filepp"))
    {
      file_address = params[i].value;
      break;
    }
  }

  if (strcmp(file_address, "wtf_piss_off"))
    return NULL;

  g_Resources.fp = fopen(file_address, "rb");

  if (g_Resources.fp == NULL)
    return NULL;

  


  return &g_Resources;
}

void test2teardown(void* fixture) {
  if (fixture == NULL)
    return;

  test_resources *resources = (test_resources*)fixture;
  png_destroy_read_struct(&resources->png_ptr, &resources->info_ptr, NULL);

  // Free the buffer
  for (int i = 0; i < resources->height; ++i)
  {
    free(resources->row_pointers[i]);
  }

  if (resources->fp)
    fclose(resources->fp);
}

int main(int argc, char** argv) {
  printf("test runner initializing... \n");

  char *filename;
  if (argc > 1)
    filename = argv[1];
  else
    filename = "./basic.png";
  char *filepp_params[] = { filename, NULL };

  MunitParameterEnum test_params[] = { 
    { 
      "filepp", filepp_params
    }, 
    { NULL, NULL} 
  };
  
  MunitTest test1 = { "aTestCanPass", aTestCanPass, NULL, NULL, MUNIT_TEST_OPTION_NONE };
  MunitTest test2 = { "itCanDecompressAPng", itCanDecompressAPng, NULL, test2teardown, MUNIT_TEST_OPTION_NONE, test_params };

  MunitTest testarray[] = { 
    test1, 
    test2, 
    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
  };
  
  MunitSuite suite = { "tests.", testarray };

  return munit_suite_main(&suite, NULL, 0, argv);
  itCanDecompressAPng(test_params, NULL);
}
