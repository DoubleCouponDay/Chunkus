#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "munit.h"
#include "../src/entrypoint.h"
#include "../src/types/colour.h"
#include <png.h>
#include <errno.h>
#ifndef _WIN32
#include <dirent.h>
#endif
#include "../test/tools.h"
#include "./readpng.h"
#define ERROR -1

filesetup* createfilesetup(const MunitParameter params[], void* userdata)
{
  DEBUG("setting filesetup memory... \n");
  // return address of global variable, or allocate on the heap
  filesetup* setup = calloc(1, sizeof(filesetup));

  // Open file, create png_struct, create info_struct
  DEBUG("finding the file param \n");
  // Find file address parameter  
  
  char* file_address = NULL;
  for (int i = 0; params[i].name != NULL && params[i].value != NULL; ++i)
  {
    if (strcmp(params[i].name, "filename") == 0)
    {
      file_address = params[i].value;
    }
  }

  munit_assert_ptr_not_null(file_address);

  DEBUG("Now checking file address: %s \n", file_address);

  setup->file = fopen(file_address, "rb");

  if (setup->file == NULL)
  {
    DEBUG("File pointer was null \n");
    return NULL;
  }
  
  DEBUG("Setup 2 complete....\n");
  return setup;
}

fileresources* readfile(const MunitParameter params[], filesetup* setup) {
  DEBUG("reading file... \n");
  fileresources* resources = calloc(1, sizeof(fileresources));
  resources->setup = setup;
  munit_assert_not_null(resources);
  char* fileaddress = resources->setup->file_address;
  DEBUG("fileaddress = %s \n", fileaddress);
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
    return NULL;
  }  
  resources->png_pointer = png_ptr;

  DEBUG("Pre Create Info Struct\n");
  png_infop info_ptr = png_create_info_struct(png_ptr);
  resources->info_pointer = info_ptr;

  if (setjmp(png_jmpbuf(png_ptr))) return NULL;

  DEBUG("Pre Init IO\n");
  png_init_io(png_ptr, file);

  DEBUG("Pre Read Info\n");
  png_read_info(png_ptr, info_ptr);

  DEBUG("Pre Get dimensions\n");
  width = png_get_image_width(png_ptr, info_ptr);
  height = png_get_image_height(png_ptr, info_ptr);
  resources->width = width;
  resources->height = height;

  DEBUG("Width: %d, Height: %d\n", width, height);
  color_type = png_get_color_type(png_ptr, info_ptr);
  bit_depth = png_get_bit_depth(png_ptr, info_ptr); 
  resources->color_type = color_type;
  resources->bit_depth = bit_depth;

  //image header is invalid
  if (!info_ptr)
  {
      png_destroy_read_struct(&png_ptr,
          (png_infopp)NULL, (png_infopp)NULL);

      //*NULL; //throw segfault
  }
  DEBUG("image is valid\n");
  resources->row_pointers = (png_bytep*)calloc(1, sizeof(png_bytep) * height);

  for(int y = 0; y < height; y++) {
    resources->row_pointers[y] = (png_byte*)calloc(1, png_get_rowbytes(png_ptr, info_ptr));
  }
  DEBUG("read the image\n");
  imagestruct.format = PNG_FORMAT_RGB;
  png_read_image(png_ptr, resources->row_pointers);


  DEBUG("About to return populated resources\n");
  return resources;
}

void freefile(fileresources* resources) {
  png_destroy_read_struct(&resources->png_pointer, &resources->info_pointer, NULL);
  
  for (int i = 0; i < resources->height; ++i)
    free(resources->row_pointers[i]);
  free(resources);
}

