#include <nanosvg.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <png.h>
#include <errno.h>

#include "init.h"
#include "../src/utility/defines.h"
#include "munit.h"

#include "../src/chunkmap.h"
#include "../src/imagefile/pngfile.h"
#include "../src/imagefile/bmp.h"
#include "../src/entrypoint.h"
#include "../src/nsvg/usage.h"
#include "tears.h"
#include "../src/utility/error.h"
#include "../src/imagefile/svg.h"
#include "../src/imagefile/converter.h"
#include "tests.h"
#include "../src/utility/logger.h"

int main(int argc, char** argv) {
  LOG_INFO("test runner initializing... ");
  LOG_INFO("args: ");

  for (int i = 0; i < argc; ++i) {
    LOG_INFO("%s, ", argv[i]);
  }

  char* param1[] = { "../test/test.png", NULL };
  char* param2[] = { "1", NULL };
  char* param3[] = { "1", NULL }; //max threshhold 440
  char* param4[] = { "./chunkmap.png", NULL };
  char* param5[] = { "256", NULL }; //max colours 256
  char* param6[] = { "../test/test.jpeg", NULL };
  char* testname = argv[1];

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
    {
      "num_colours", param5
    },
    {
      "jpeg", param6
    },
    { NULL, NULL} 
  };

  MunitTest apple = { "can_pass", aTestCanPass, NULL, NULL, MUNIT_TEST_OPTION_NONE };
  MunitTest orange = { "read_png", can_read_png, test2setup, test2teardown, MUNIT_TEST_OPTION_NONE, test_params };
  MunitTest mango = { "png_to_chunkmap", can_convert_png_to_chunkmap, test4setup, test4teardown, MUNIT_TEST_OPTION_NONE, test_params };
  MunitTest peach = { "png_to_bmp", opensPngAndOutputsBmp, test5setup, test5teardown, MUNIT_TEST_OPTION_NONE, test_params };
  MunitTest melon = { "png_to_nsvg", can_vectorize_image, test6setup, test6teardown, MUNIT_TEST_OPTION_NONE, test_params };
  MunitTest cherry = { "chunkmap_to_png", can_write_chunkmap_shapes_to_file, test69setup, test69teardown, MUNIT_TEST_OPTION_NONE, test_params };
  MunitTest banana = { "dcdfill", can_write_to_svgfile, test8setup, test8teardown, MUNIT_TEST_OPTION_NONE, test_params };
  MunitTest yo_mama = { "bobsweep", can_do_speedy_vectorize, speedy_vectorize_setup, speedy_vectorize_teardown, MUNIT_TEST_OPTION_NONE, test_params };
  MunitTest run = { "run", just_run, NULL, NULL, MUNIT_TEST_OPTION_NONE, test_params };
  MunitTest pineapple = { "jpeg_to_image", JPEG_to_image, NULL, NULL, MUNIT_TEST_OPTION_NONE, test_params };
  MunitTest apricot = { "jpeg_to_bmp", can_convert_jpeg_to_bmp, jpeg_to_bmp_setup, jpeg_to_bmp_teardown, MUNIT_TEST_OPTION_NONE, test_params };
  MunitTest dragonfruit = { "jpeg_dcd", jpeg_dcd, test8setup, test8teardown, MUNIT_TEST_OPTION_NONE, test_params };
  //passionfruit
  //guava
  //mandarin
  //date

  enum { 
    NUM_TESTS = 12 //UPDATE THIS WHEN YOU ADD NEW TESTS
  }; 

  namedtest tests[NUM_TESTS] = {
    {apple.name, apple},
    {orange.name, orange},
    {mango.name, mango},
    {peach.name, peach},
    {melon.name, melon},
    {cherry.name, cherry},
    {banana.name, banana},
    {yo_mama.name, yo_mama},
    {run.name, run},
    {pineapple.name, pineapple},
    {apricot.name, apricot},
    {dragonfruit.name, dragonfruit}
  };
  MunitTest* filteredtests = filtertests(tests, NUM_TESTS, testname);
  MunitSuite suite = { "tests.", filteredtests };
  int result = munit_suite_main(&suite, NULL, 0, argv);
  return result;
}
