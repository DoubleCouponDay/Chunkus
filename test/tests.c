#include <nanosvg.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <png.h>
#include <errno.h>

#include "../src/defines.h"
#include "munit.h"
#include "tools.h"
#include "readpng.h"
#include "../src/mapping.h"
#include "../src/imagefile.h"
#include "../src/entrypoint.h"
#include "../src/types/colour.h"
#include "../src/svg/svg.h"
#include "tears.h"
#include "../src/error.h"

const int SINGLE_TEST_RUN = 2;

MunitResult aTestCanPass(const MunitParameter params[], void* data) {
  DEBUG("test 1 passed\n");
  return MUNIT_OK;
}

MunitResult test2_itCanDecompressAPng(const MunitParameter params[], void* userdata) {
  DEBUG("reading file.\n");
  fileresources* resources = readfile(params, userdata);
  DEBUG("freeing file\n");
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

MunitResult test6_can_vectorize_image(const MunitParameter params[], void* userdata)
{
  test6stuff* stuff = userdata;

  vectorize_options options = {
    params[0].value,
    atoi(params[1].value),
    atof(params[2].value)
  };
  
  stuff->img = convert_png_to_image(options.file_path);  
  stuff->svg = vectorize_image(stuff->img, options);

  return MUNIT_OK;
}

MunitResult test69_can_write_chunkmap_shapes_to_file(const MunitParameter params[], void* userdata)
{
  typedef struct
  {
    int a;
  } your_mom;

  test69stuff* stuff = userdata;

  char* fileaddress = params[0].value;

  char* chunk_size_str = params[1].value;
  int chunk_size = atoi(chunk_size_str);

  char* threshold_str = params[2].value;
  float threshold = atof(threshold_str);
  
  char* out_fileaddress = params[3].value;

  vectorize_options options = {
    fileaddress,
    chunk_size,
    threshold,
  };

  //DEBUG("test69 with: file=%s out_file=%s chunk_size=%d threshold=%d, options struct: file=%s chunk_size=%d, threshold=%d \n", fileaddress, out_fileaddress, chunk_size, threshold, options.file_path, options.chunk_size, options.shape_colour_threshhold);

  stuff->img = convert_png_to_image(fileaddress);
  DEBUG("asserting pixels_array_2d not null\n");
  munit_assert_ptr_not_null(stuff->img.pixels_array_2d);

  DEBUG("generating chunkmap\n");
  chunkmap thing3 = generate_chunkmap(stuff->img, options);
  stuff->map = thing3;
  DEBUG("asserting groups_array_2d not null\n");
  munit_assert_ptr_not_null(stuff->map.groups_array_2d);
  DEBUG("writing chunkmap to file\n");
  write_chunkmap_to_file(stuff->map, out_fileaddress);

  FILE* fp = fopen(out_fileaddress, "r");

  munit_assert_ptr_not_null(fp);

  fclose(fp);
  return MUNIT_OK;
}

int main(int argc, char** argv) {
  DEBUG("test runner initializing... \n");

  char* param1[] = { "../../../../test/test.png", NULL };
  char* param2[] = { "4", NULL };
  char* param3[] = { "1", NULL };
  char* param4[] = { "./chunkmap.bmp", NULL };

  MunitParameterEnum test_params[] = { 
    { 
      "filename", param1,
    },
    {
      "chunk_size", param2
    },
    {
      "shape_colour_threshhold", param3
    },
    { 
      "output_filename", param4 
    },
    { NULL, NULL} 
  };
  
  MunitTest test1 = { "aTestCanPass", aTestCanPass, NULL, NULL, MUNIT_TEST_OPTION_NONE };
  MunitTest test2 = { "itCanDecompressAPng", test2_itCanDecompressAPng, createfilesetup, test2teardown, MUNIT_TEST_OPTION_NONE, test_params };
  MunitTest test3 = { "weKnowHowToGetPixelDataFromPng", test3_weKnownHowToGetPixelDataFromPng, test3setup, test3teardown, MUNIT_TEST_OPTION_NONE, test_params };
  MunitTest test4 = { "can_convert_image_to_node_map", test4_can_convert_file_to_node_map, test4setup, test4teardown, MUNIT_TEST_OPTION_NONE, test_params };
  MunitTest test5 = { "opensPngAndOutputsBmp", test5_opensPngAndOutputsBmp, test5setup, test5teardown, MUNIT_TEST_OPTION_NONE, test_params };
  MunitTest test6 = { "canVectorizeImage", test6_can_vectorize_image, test6setup, test6teardown, MUNIT_TEST_OPTION_NONE, test_params };
  MunitTest test7 = { "can_write_chunkmap_to_file", test69_can_write_chunkmap_shapes_to_file, test69setup, test69teardown, MUNIT_TEST_OPTION_NONE, test_params };

  //UPDATE THIS WHEN NEEDED
  int testcount = 8; //testcount plus the last null item
  char* unindexed = argv[1];

  if(unindexed != NULL) {
    testcount = SINGLE_TEST_RUN;
  }
  MunitTest* testarray = calloc(1, sizeof(MunitTest) * testcount);
  MunitTest endofarray = { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL };
  
  if(testcount == SINGLE_TEST_RUN) {
    if(!strcmp(unindexed, "test1")) {
      testarray[0] = test1;
    }

    else if(!strcmp(unindexed, "test2")) {
      testarray[0] = test2;
    }
    
    else if(!strcmp(unindexed, "test3")) {
      testarray[0] = test3;
    }

    else if(!strcmp(unindexed, "test4")) {
      testarray[0] = test4;
    }
    
    else if(!strcmp(unindexed, "test5")) {
      testarray[0] = test5;
    }

    else if(!strcmp(unindexed, "test6")) {
      testarray[0] = test6;
    }
    
    else if(!strcmp(unindexed, "test7")) {
      testarray[0] = test7;
    }

    else {
      DEBUG("assumption wrong. argv was: %s\n", unindexed);
      exit(ASSUMPTION_WRONG);
    }
    testarray[1] = endofarray;
  }

  else {
    testarray[0] = test1;
    testarray[1] = test2;
    testarray[2] = test3;
    testarray[3] = test4;
    testarray[4] = test5;
    testarray[5] = test6;
    testarray[6] = test7;
    testarray[7] = endofarray;
  }
  MunitSuite suite = { "tests.", testarray };
  int result = munit_suite_main(&suite, NULL, 0, argv);
  return result;
}
