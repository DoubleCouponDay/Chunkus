#pragma once

#ifndef _WIN32
#include <dirent.h>
#endif

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "munit.h"
#include <png.h>
#include <nanosvg.h>
#include <errno.h>

#include "../src/tools.h"
#include "./readpng.h"
#include "../src/mapping.h"
#include "../src/imagefile.h"
#include "../src/entrypoint.h"
#include "../src/types/colour.h"

#define NANOSVG_IMPLEMENTATION
#define ERROR -1

MunitResult aTestCanPass(const MunitParameter params[], void* data) {
  return MUNIT_OK;
}

void* test3setup(const MunitParameter params[], void* userdata) {
  filesetup* setup = createfilesetup(params, NULL);
  return readfile(params, setup);
}

void test2teardown(void *);

void test3teardown(void* fixture) {
  fileresources* resources = fixture;
  void* accessaVoidproperty = resources->setup;
  test2teardown(accessaVoidproperty);
  freefile(fixture); 
}

MunitResult weKnowHowToGetPixelDataFromPng3(const MunitParameter params[], void* userdata) {
  DEBUG_PRINT("creating resources...\n");
  fileresources* resources = userdata;
  int x = 3, y = 9;
  png_color color, color2;
  DEBUG_PRINT("Row Pointers: %s\n", resources->row_pointers);
  color = *(png_color*)(resources->row_pointers[y] + x * 3);
  x += 1;
  color2 = *(png_color*)(resources->row_pointers[y] + x * 3);

  DEBUG_PRINT("Extracted Color: (%d, %d, %d)\n", color.red, color.green, color.blue);
  DEBUG_PRINT("Extracted 2nd Color: (%d, %d, %d)\n", color2.red, color2.green, color2.blue);

  // returns if didn't seg fault, that counts as success
  return MUNIT_OK;
}

MunitResult opensPngAndOutputsBmp(const MunitParameter params[], void *userdata) {
  // Use constant input/output path
  char* in_file = params[0].value;
  char *out_file = "test_out.bmp";

  // Delete output file
  remove(out_file);

  image img = convert_png_to_image(in_file);

  munit_assert_ptr_not_null(img.pixels); // FAILED TO CONVERT IMAGE

  write_image_to_file(img, out_file);

  FILE *fp = fopen(out_file, "r");
  munit_assert_ptr_not_null(fp); // OUTPUT FILE NOT FOUND
  fclose(fp);

  free_image_contents(&img);

  return MUNIT_OK;
}

MunitResult can_convert_file_to_node_map(const MunitParameter params[], void* userdata) {
  image img = convert_png_to_image(params[0].value);
  vectorize_options options = { 4 };
  groupmap map = generate_pixel_group(img, options);
}

void test2teardown(void* fixture) {
  if (fixture == NULL)
    return;

  filesetup* file_setup = (filesetup*)fixture;

  if (file_setup->file)
    fclose(file_setup->file);
}

MunitResult itCanDecompressAPng2(const MunitParameter params[], void* userdata) {
  fileresources* resources = readfile(params, userdata);
  freefile(resources);
  return MUNIT_OK;
}

int main(int argc, char** argv) {
  DEBUG_PRINT("test runner initializing... \n");

  char* filename;
  if (argc > 1)
    filename = argv[1];
  else
    filename = "../../../../test/test.png";

  char* filepp_params[] = { filename, NULL };

  MunitParameterEnum test_params[] = { 
    { 
      "filename", filepp_params
    }, 
    { NULL, NULL} 
  };
  
  MunitTest test1 = { "aTestCanPass", aTestCanPass, NULL, NULL, MUNIT_TEST_OPTION_NONE };
  MunitTest test2 = { "itCanDecompressAPng", itCanDecompressAPng2, createfilesetup, test2teardown, MUNIT_TEST_OPTION_NONE, test_params };
  MunitTest test3 = { "weKnowHowToGetPixelDataFromPng3", weKnowHowToGetPixelDataFromPng3, test3setup, test3teardown, MUNIT_TEST_OPTION_NONE, test_params };
  MunitTest test4 = { "can_convert_image_to_node_map", can_convert_file_to_node_map, NULL, NULL, MUNIT_TEST_OPTION_NONE, test_params };
  MunitTest test5 = { "opensPngAndOutputsBmp", opensPngAndOutputsBmp, NULL, NULL, MUNIT_TEST_OPTION_NONE, test_params };

  MunitTest testarray[] = { 
    test1, 
    test2, 
    test3, 
    test4,
    test5,
    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
  };
  printf("\n"); //just because munit prints messily
  MunitSuite suite = { "tests.", testarray };
  int result = munit_suite_main(&suite, NULL, 0, argv);
  return result;
}
