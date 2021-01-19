#pragma once

#ifndef _WIN32
#include <dirent.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <png.h>
#include <nanosvg.h>
#include <errno.h>
#include "munit.h"

#include "tools.h"
#include "readpng.h"
#include "../src/mapping.h"
#include "../src/imagefile.h"
#include "../src/entrypoint.h"
#include "../src/types/colour.h"
#include "../src/svg/svg.h"
#include "tears.h"

#define NANOSVG_IMPLEMENTATION
#define ERROR -1

MunitResult aTestCanPass(const MunitParameter params[], void* data) {
  return MUNIT_OK;
}

MunitResult test2_itCanDecompressAPng(const MunitParameter params[], void* userdata) {
  fileresources* resources = readfile(params, userdata);
  freefile(resources);
  return MUNIT_OK;
}

MunitResult test3_weKnownHowToGetPixelDataFromPng(const MunitParameter params[], void* userdata) {
  DEBUG("creating resources...\n");
  fileresources* resources = userdata;
  int x = 3, y = 9;
  png_color color, color2;
  DEBUG("Row Pointers: %s\n", resources->row_pointers);
  color = *(png_color*)(resources->row_pointers[y] + x * 3);
  x += 1;
  color2 = *(png_color*)(resources->row_pointers[y] + x * 3);

  DEBUG("Extracted Color: (%d, %d, %d)\n", color.red, color.green, color.blue);
  DEBUG("Extracted 2nd Color: (%d, %d, %d)\n", color2.red, color2.green, color2.blue);

  // returns if didn't seg fault, that counts as success
  return MUNIT_OK;
}

MunitResult test4_can_convert_file_to_node_map(const MunitParameter params[], void* userdata) {
  test4stuff* stuff = userdata;
  
  vectorize_options options = {
    params[0].value,
    (int)params[1].value,
    atof(params[2].value)
  };
  stuff->img = convert_png_to_image(options.file_path);
  stuff->map = generate_chunkmap(stuff->img, options);
  return MUNIT_OK;
}

MunitResult test5_opensPngAndOutputsBmp(const MunitParameter params[], void* userdata) {
  test5stuff* stuff = userdata;
  // Use constant input/output path
  char* in_file = params[0].value;
  char* out_file = "test_out.bmp";

  // Delete output file
  remove(out_file);

  stuff->img = convert_png_to_image(in_file);

  munit_assert_ptr_not_null(stuff->img.pixels_array_2d); // FAILED TO CONVERT IMAGE

  write_image_to_file(stuff->img, out_file);

  FILE* fp = fopen(out_file, "r");
  stuff->fp = fp;
  munit_assert_ptr_not_null(fp); // OUTPUT FILE NOT FOUND

  return MUNIT_OK;
}

MunitResult test6_can_convert_chunkmap_to_svgmap(const MunitParameter params[], void* userdata) {
  return MUNIT_OK;
}

MunitResult test7_can_vectorize_image(const MunitParameter params[], void* userdata)
{
  test7stuff* stuff = userdata;

  vectorize_options options = {
    params[0].value,
    (int)params[1].value,
    atof(params[2].value)
  };
  
  stuff->img = convert_png_to_image(options.file_path);
  stuff->map = generate_chunkmap(stuff->img, options);
  
  stuff->svg = vectorize_image(stuff->img, stuff->map, options.shape_colour_threshhold);

  return MUNIT_OK;
}

int main(int argc, char** argv) {
  DEBUG("test runner initializing... \n");

  char* filename = "../../../../test/test.png";
  char* filepp_params[] = { filename, NULL };

  MunitParameterEnum test_params[] = { 
    { 
      "filename", filepp_params
    },
    {
      "chunk_size", "4"
    },
    {
      "shape_colour_threshhold", "5.0"
    },
    { NULL, NULL} 
  };
  
  MunitTest test1 = { "aTestCanPass", aTestCanPass, NULL, NULL, MUNIT_TEST_OPTION_NONE };
  MunitTest test2 = { "itCanDecompressAPng", test2_itCanDecompressAPng, createfilesetup, test2teardown, MUNIT_TEST_OPTION_NONE, test_params };
  MunitTest test3 = { "weKnowHowToGetPixelDataFromPng3", test3_weKnownHowToGetPixelDataFromPng, test3setup, test3teardown, MUNIT_TEST_OPTION_NONE, test_params };
  MunitTest test4 = { "can_convert_image_to_node_map", test4_can_convert_file_to_node_map, test4setup, test4teardown, MUNIT_TEST_OPTION_NONE, test_params };
  MunitTest test5 = { "opensPngAndOutputsBmp", test5_opensPngAndOutputsBmp, test5setup, test5teardown, MUNIT_TEST_OPTION_NONE, test_params };
  MunitTest test7 = { "canVectorizeImage", test7_can_vectorize_image, test7setup, test7teardown, MUNIT_TEST_OPTION_NONE, test_params };

  MunitTest testarray[] = { 
    test1, 
    test2, 
    test3, 
    test4,
    test5,
    test7,
    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
  };
  MunitSuite suite = { "tests.", testarray };
  int result = munit_suite_main(&suite, NULL, 0, argv);
  return result;
}
