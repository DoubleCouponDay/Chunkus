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
#include "../src/algorithm/usage.h"
#include "tears.h"
#include "../src/utility/error.h"
#include "../src/imagefile/svg.h"
#include "../src/imagefile/converter.h"
#include "tests.h"
#include "../src/utility/logger.h"

int main(int argc, char** argv) {
  clear_logfile();
  LOG_INFO("test runner initializing... ");
  LOG_INFO("args: ");

  for (int i = 0; i < argc; ++i) {
    LOG_INFO("%s", argv[i]);
  }

  char* png[] = { "./install/bin/test.png", NULL };
  char* chunkSize[] = { "1", NULL };
  char* thresholds[] = { "5", NULL };
  char* png_output[] = { "./install/bin/output.png", NULL };
  char* num_colours[] = { "256", NULL };
  char* jpeg[] = { "./install/bin/test.jpeg", NULL };
  char* bmp_output[] = { "./install/bin/output.bmp", NULL };
  char* png2[] = { "./install/bin/test2.png", NULL };
  char* png3[] = { "./install/bin/test3.png", NULL };

  MunitParameterEnum test_params[] = { 
    { 
      "0_png", png,
    },
    {
      "1_png2", png2
    },
    {
      "2_png3", png3
    },
    {
      "3_chunk_size", chunkSize
    },
    {
      "4_thresholds", thresholds
    },
    { 
      "5_png_output", png_output 
    },
    {
      "6_num_colours", num_colours
    },
    {
      "7_jpeg", jpeg
    },
    {
      "8_bmp_output", bmp_output
    },
    { NULL, NULL} 
  };

  MunitTest can_pass = { "empty", can_test, NULL, NULL, MUNIT_TEST_OPTION_NONE };
  
  MunitTest read_png = { "read_png", can_read_png, tear1setup, tear1teardown, MUNIT_TEST_OPTION_NONE, test_params };
  MunitTest png_to_chunkmap = { "png_to_chunkmap", can_convert_png_to_chunkmap, tear2setup, tear2teardown, MUNIT_TEST_OPTION_NONE, test_params };
  MunitTest png_to_bmp = { "png_to_bmp", can_convert_png_to_bmp, tear3setup, tear3teardown, MUNIT_TEST_OPTION_NONE, test_params };
  MunitTest png_to_nsvg = { "png_to_nsvg", can_vectorize_png, tear4setup, tear4teardown, MUNIT_TEST_OPTION_NONE, test_params };
  MunitTest png_to_svg = { "png_to_svg", can_convert_png_to_svg, tear4setup, tear4teardown, MUNIT_TEST_OPTION_NONE, test_params };
  MunitTest png2_to_svg = { "png2_to_svg", can_convert_png2_to_svg, tear4setup, tear4teardown, MUNIT_TEST_OPTION_NONE, test_params };
  MunitTest png3_to_svg = { "png3_to_svg", can_convert_png3_to_svg, tear4setup, tear4teardown, MUNIT_TEST_OPTION_NONE, test_params };

  MunitTest read_jpeg = { "read_jpeg", can_read_jpeg, tear1setup, tear1teardown, MUNIT_TEST_OPTION_NONE, test_params };
  MunitTest jpeg_to_bmp = { "jpeg_to_bmp", can_convert_jpeg_to_bmp, tear3setup, tear3teardown, MUNIT_TEST_OPTION_NONE, test_params };
  MunitTest jpeg_to_nsvg = { "jpeg_to_nsvg", can_vectorize_jpeg, tear4setup, tear4teardown, MUNIT_TEST_OPTION_NONE, test_params };
  MunitTest jpeg_to_svg = { "jpeg_to_svg", can_convert_jpeg_to_svg, tear4setup, tear4teardown, MUNIT_TEST_OPTION_NONE, test_params};

  enum { 
    NUM_TESTS = 12 //UPDATE THIS WHEN YOU ADD NEW TESTS
  }; 

  namedtest tests[NUM_TESTS] = {
    {can_pass.name, can_pass},

    {read_png.name, read_png},
    {png_to_chunkmap.name, png_to_chunkmap},
    {png_to_bmp.name, png_to_bmp},
    {png_to_nsvg.name, png_to_nsvg},
    {png_to_svg.name, png_to_svg},
    {png2_to_svg.name, png2_to_svg},
    {png3_to_svg.name, png3_to_svg},

    {read_jpeg.name, read_jpeg},
    {jpeg_to_bmp.name, jpeg_to_bmp},
    {jpeg_to_nsvg.name, jpeg_to_nsvg},
    {jpeg_to_svg.name, jpeg_to_svg}
  };
  char* testname = argv[1];
  MunitTest* filteredtests = filtertests(tests, NUM_TESTS, testname);
  MunitSuite suite = { "tests.", filteredtests };
  int result = munit_suite_main(&suite, NULL, 0, argv);
  return result;
}
