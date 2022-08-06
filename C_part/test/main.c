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

  char* param1[] = { "test.png", NULL };
  char* param2[] = { "1", NULL };
  char* param3[] = { "1", NULL }; //max threshhold 440
  char* param4[] = { "./chunkmap.png", NULL };
  char* param5[] = { "256", NULL }; //max colours 256
  char* param6[] = { "test.jpeg", NULL };
  char* testname = argv[1];

  MunitParameterEnum test_params[] = { 
    { 
      "filename", param1,
    },
    {
      "chunk_size", param2
    },
    {
      "threshold", param3
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

  MunitTest can_pass = { "can_pass", aTestCanPass, NULL, NULL, MUNIT_TEST_OPTION_NONE };
  MunitTest read_png = { "read_png", can_read_png, test2setup, test2teardown, MUNIT_TEST_OPTION_NONE, test_params };
  MunitTest png_to_chunkmap = { "png_to_chunkmap", can_convert_png_to_chunkmap, test4setup, test4teardown, MUNIT_TEST_OPTION_NONE, test_params };
  MunitTest png_to_bmp = { "png_to_bmp", can_convert_png_to_bmp, test5setup, test5teardown, MUNIT_TEST_OPTION_NONE, test_params };
  MunitTest png_to_nsvg = { "png_to_nsvg", can_vectorize_png, test6setup, test6teardown, MUNIT_TEST_OPTION_NONE, test_params };
  MunitTest chunkmap_to_png = { "chunkmap_to_png", can_write_chunkmap_shapes_to_file, test69setup, test69teardown, MUNIT_TEST_OPTION_NONE, test_params };
  MunitTest png_to_svg = { "png_to_svg", can_convert_png_to_svg, test8setup, test8teardown, MUNIT_TEST_OPTION_NONE, test_params };

  MunitTest jpeg_to_image = { "jpeg_to_image", can_convert_jpeg_to_image, NULL, NULL, MUNIT_TEST_OPTION_NONE, test_params };
  MunitTest jpeg_to_bmp = { "jpeg_to_bmp", can_convert_jpeg_to_bmp, jpeg_to_bmp_setup, jpeg_to_bmp_teardown, MUNIT_TEST_OPTION_NONE, test_params };
  MunitTest jpeg_to_nsvg = { "jpeg_to_nsvg", can_vectorize_jpeg, test8setup, test8teardown, MUNIT_TEST_OPTION_NONE, test_params };

  enum { 
    NUM_TESTS = 10 //UPDATE THIS WHEN YOU ADD NEW TESTS
  }; 

  namedtest tests[NUM_TESTS] = {
    {can_pass.name, can_pass},
    {read_png.name, read_png},
    {png_to_chunkmap.name, png_to_chunkmap},
    {png_to_bmp.name, png_to_bmp},
    {png_to_nsvg.name, png_to_nsvg},
    {chunkmap_to_png.name, chunkmap_to_png},
    {png_to_svg.name, png_to_svg},
    {jpeg_to_image.name, jpeg_to_image},
    {jpeg_to_bmp.name, jpeg_to_bmp},
    {jpeg_to_nsvg.name, jpeg_to_nsvg}
  };
  MunitTest* filteredtests = filtertests(tests, NUM_TESTS, testname);
  MunitSuite suite = { "tests.", filteredtests };
  int result = munit_suite_main(&suite, NULL, 0, argv);
  return result;
}
