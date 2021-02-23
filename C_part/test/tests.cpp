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
#include "../src/nsvg/dcdfiller.h"
#include "utility/logger.h"


MunitResult aTestCanPass(const MunitParameter params[], void* data) {
  LOG_INFO("test 1 passed");
  return MUNIT_OK;
}

MunitResult can_read_png(const MunitParameter params[], void* userdata) {
  test2stuff* stuff = reinterpret_cast<test2stuff*>(userdata);
  char* filepath = params[0].value;
  image subject = convert_png_to_image(filepath);
  munit_assert_int(getAndResetErrorCode(), ==, SUCCESS_CODE);
  stuff->img = std::move(subject);
  return MUNIT_OK;
}

MunitResult can_convert_png_to_chunkmap(const MunitParameter params[], void* userdata) {
  test4stuff* stuff = reinterpret_cast<test4stuff*>(userdata);
  
  vectorize_options options = {
    params[0].value,
    atoi(params[1].value),
    atof(params[2].value)
  };
  stuff->img = convert_png_to_image(options.file_path);
  chunkmap map = chunkmap(stuff->img, options.chunk_size);
  munit_assert_int(getAndResetErrorCode(), ==, SUCCESS_CODE);
  stuff->map = std::move(map);
  return MUNIT_OK;
}

MunitResult opensPngAndOutputsBmp(const MunitParameter params[], void* userdata) {
  test5stuff* stuff = reinterpret_cast<test5stuff*>(userdata);
  // Use constant input/output path
  char* in_file = params[0].value;
  char* out_file = "peach.bmp";

  // Delete output file
  remove(out_file);

  stuff->img = convert_png_to_image(in_file);

  munit_assert_false(stuff->img.pixels.empty()); // FAILED TO CONVERT IMAGE

  write_image_to_bmp(stuff->img, out_file);

  FILE* fp = fopen(out_file, "r");
  stuff->fp = fp;
  munit_assert_ptr_not_null(fp); // OUTPUT FILE NOT FOUND

  return MUNIT_OK;
}

MunitResult can_vectorize_image(const MunitParameter params[], void* userdata)
{
  test6stuff* stuff = reinterpret_cast<test6stuff*>(userdata);

  vectorize_options options = {
    params[0].value,
    atoi(params[1].value),
    atof(params[2].value)
  };
  
  stuff->img = convert_png_to_image(options.file_path);
  stuff->nsvg_image = dcdfill_for_nsvg(stuff->img, options);

  munit_assert_int(getAndResetErrorCode(), ==, SUCCESS_CODE);
  munit_assert((bool)stuff->nsvg_image);

  return MUNIT_OK;
}

MunitResult can_write_chunkmap_shapes_to_file(const MunitParameter params[], void* userdata)
{
  test69stuff* stuff = reinterpret_cast<test69stuff*>(userdata);

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

  stuff->img = convert_png_to_image(fileaddress);
  LOG_INFO("asserting pixels_array_2d not null");
  munit_assert_false(stuff->img.pixels.empty());
  LOG_INFO("generating chunkmap");
  chunkmap map = chunkmap(stuff->img, options.chunk_size);
  munit_assert_int(getAndResetErrorCode(), ==, SUCCESS_CODE);
  stuff->map = std::move(map);

  LOG_INFO("asserting groups_array_2d not null");
  munit_assert_false(stuff->map.groups.empty());
  LOG_INFO("filling chunkmap");
  fill_chunkmap(stuff->map, options);
  munit_assert_int(getAndResetErrorCode(), ==, SUCCESS_CODE);

  LOG_INFO("writing chunkmap to file");
  write_chunkmap_to_png(stuff->map, out_fileaddress);
  munit_assert_int(getAndResetErrorCode(), ==, SUCCESS_CODE);

  FILE* fp = fopen(out_fileaddress, "r");

  munit_assert_ptr_not_null(fp);

  fclose(fp);
  return MUNIT_OK;
}

MunitResult can_write_to_svgfile(const MunitParameter params[], void* userdata) {
  test8stuff* stuff = reinterpret_cast<test8stuff*>(userdata);

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

  stuff->img = convert_png_to_image(fileaddress);
  munit_assert_false(stuff->img.pixels.empty());
  munit_assert_int(getAndResetErrorCode(), ==, SUCCESS_CODE);

  stuff->nsvg_image = dcdfill_for_nsvg(stuff->img, options);
  munit_assert_int(getAndResetErrorCode(), ==, SUCCESS_CODE);

  munit_assert(write_svg_file(stuff->nsvg_image));
    
  munit_assert_int(getAndResetErrorCode(), ==, SUCCESS_CODE);

  FILE* fp = fopen(OUTPUT_PATH, "r"); //check it at least creates a file every time

  munit_assert_ptr_not_null(fp);

  fclose(fp);

  return MUNIT_OK;
}

MunitResult can_do_speedy_vectorize(const MunitParameter params[], void* userdata)
{
  speedy_vectorize_stuff* stuff = reinterpret_cast<speedy_vectorize_stuff*>(userdata);

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

  stuff->img = convert_png_to_image(fileaddress);
  munit_assert_false(stuff->img.pixels.empty());
  munit_assert_int(getAndResetErrorCode(), ==, SUCCESS_CODE);

  stuff->nsvg_image = bobsweep_for_nsvg(stuff->img, options);
  munit_assert_int(getAndResetErrorCode(), ==, SUCCESS_CODE);

  munit_assert(write_svg_file(stuff->nsvg_image));
  munit_assert_int(getAndResetErrorCode(), ==, SUCCESS_CODE);
  
  FILE* fp = fopen(OUTPUT_PATH, "r");

  munit_assert_ptr_not_null(fp);

  fclose(fp);

  return MUNIT_OK;
}

int main(int argc, char** argv) {
  LOG_INFO("test runner initializing...");
  LOG_INFO("args: ");
  printf("\t");
  for (int i = 0; i < argc; ++i)
    printf("%s, ", argv[i]);
  printf("\n");

  char* param1[] = { "../../../../test/test.png", NULL };
  char* param2[] = { "1", NULL };
  char* param3[] = { "100", NULL }; //max threshhold 440
  char* param4[] = { "./chunkmap.png", NULL };
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

  enum { 
    NUM_TESTS = 8 //UPDATE THIS WHEN YOU ADD NEW TESTS
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
  };
  MunitTest* filteredtests = filtertests(tests, NUM_TESTS, testname);
  MunitSuite suite = { "tests.", filteredtests };
  int result = munit_suite_main(&suite, NULL, 0, argv);
  return result;
}
