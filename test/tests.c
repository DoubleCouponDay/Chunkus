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
#include "debug.h"
#include "../src/chunkmap.h"
#include "../src/imagefile/pngfile.h"
#include "../src/imagefile/bmp.h"
#include "../src/entrypoint.h"
#include "../src/nsvg/usage.h"
#include "tears.h"
#include "../src/utility/error.h"
#include "../src/imagefile/svg.h"

MunitResult aTestCanPass(const MunitParameter params[], void* data) {
  DEBUG("test 1 passed\n");
  return MUNIT_OK;
}

MunitResult can_read_png(const MunitParameter params[], void* userdata) {
  test2stuff* stuff = userdata;
  image subject = convert_png_to_image("test.png");
  stuff->img = subject;
  return MUNIT_OK;
}

MunitResult can_convert_png_to_chunkmap(const MunitParameter params[], void* userdata) {
  test4stuff* stuff = userdata;
  
  vectorize_options options = {
    params[0].value,
    atoi(params[1].value),
    atof(params[2].value)
  };
  stuff->img = convert_png_to_image(options.file_path);
  chunkmap* map = generate_chunkmap(stuff->img, options);
  munit_assert_int(getAndResetErrorCode(), ==, SUCCESS_CODE);
  stuff->map = map;
  return MUNIT_OK;
}

MunitResult opensPngAndOutputsBmp(const MunitParameter params[], void* userdata) {
  test5stuff* stuff = userdata;
  // Use constant input/output path
  char* in_file = params[0].value;
  char* out_file = "test_out.bmp";

  // Delete output file
  remove(out_file);

  stuff->img = convert_png_to_image(in_file);

  munit_assert_ptr_not_null(stuff->img.pixels_array_2d); // FAILED TO CONVERT IMAGE

  write_image_to_bmp(stuff->img, out_file);

  FILE* fp = fopen(out_file, "r");
  stuff->fp = fp;
  munit_assert_ptr_not_null(fp); // OUTPUT FILE NOT FOUND

  return MUNIT_OK;
}

MunitResult can_vectorize_image(const MunitParameter params[], void* userdata)
{
  test6stuff* stuff = userdata;

  vectorize_options options = {
    params[0].value,
    atoi(params[1].value),
    atof(params[2].value)
  };
  
  stuff->img = convert_png_to_image(options.file_path);

  stuff->result = vectorize_image(stuff->img, options);
  munit_assert_int(getAndResetErrorCode(), ==, SUCCESS_CODE);
  munit_assert_ptr_not_null(stuff->result.nsvg_image);
  munit_assert_ptr_not_null(stuff->result.map);

  return MUNIT_OK;
}

MunitResult can_write_chunkmap_shapes_to_file(const MunitParameter params[], void* userdata)
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

  stuff->img = convert_png_to_image(fileaddress);
  DEBUG("asserting pixels_array_2d not null\n");
  munit_assert_ptr_not_null(stuff->img.pixels_array_2d);
  DEBUG("generating chunkmap\n");
  chunkmap* map = generate_chunkmap(stuff->img, options);
  munit_assert_int(getAndResetErrorCode(), ==, SUCCESS_CODE);
  stuff->map = map;

  DEBUG("asserting groups_array_2d not null\n");
  munit_assert_ptr_not_null(map->groups_array_2d);
  DEBUG("filling chunkmap\n");
  fill_chunkmap(stuff->map, &options);
  munit_assert_int(getAndResetErrorCode(), ==, SUCCESS_CODE);

  DEBUG("Now winding back chunk_shapes\n");
  wind_back_chunkshapes(&map->shape_list);
  munit_assert_int(getAndResetErrorCode(), ==, SUCCESS_CODE);

  DEBUG("writing chunkmap to file\n");
  write_chunkmap_to_png(stuff->map, out_fileaddress);
  munit_assert_int(getAndResetErrorCode(), ==, SUCCESS_CODE);

  FILE* fp = fopen(out_fileaddress, "r");

  munit_assert_ptr_not_null(fp);

  fclose(fp);
  return MUNIT_OK;
}

MunitResult can_write_to_svgfile(const MunitParameter params[], void* userdata) {
  test8stuff* stuff = userdata;

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
  DEBUG("asserting pixels_array_2d not null\n");
  munit_assert_ptr_not_null(stuff->img.pixels_array_2d);
  munit_assert_int(getAndResetErrorCode(), ==, SUCCESS_CODE);

  stuff->result = vectorize_image(stuff->img, options);
  munit_assert_int(getAndResetErrorCode(), ==, SUCCESS_CODE);

  munit_assert(write_svg_file(stuff->result.nsvg_image, stuff->result.map));
  munit_assert_int(getAndResetErrorCode(), ==, SUCCESS_CODE);

  FILE* fp = fopen(OUTPUT_PATH, "r");

  munit_assert_ptr_not_null(fp);

  fclose(fp);

  return MUNIT_OK;
}

int main(int argc, char** argv) {
  DEBUG("test runner initializing... \n");
  DEBUG("args: ");
  for (int i = 0; i < argc; ++i)
    DEBUG("%s, ", argv[i]);
  DEBUG("\n");

  char* param1[] = { "../../../../test/test.png", NULL };
  char* param2[] = { "3", NULL };
  char* param3[] = { "400", NULL };
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
  MunitTest banana = { "nsvg_to_svg", can_write_to_svgfile, test8setup, test8teardown, MUNIT_TEST_OPTION_NONE, test_params };

  enum { 
    NUM_TESTS = 7 //UPDATE THIS WHEN YOU ADD NEW TESTS
  }; 

  namedtest tests[NUM_TESTS] = {
    {apple.name, apple},
    {orange.name, orange},
    {mango.name, mango},
    {peach.name, peach},
    {melon.name, melon},
    {cherry.name, cherry},
    {banana.name, banana}
  };
  DEBUG("filtering tests...\n");
  MunitTest* filteredtests = filtertests(tests, NUM_TESTS, testname);
  MunitSuite suite = { "tests.", filteredtests };
  int result = munit_suite_main(&suite, NULL, 0, argv);
  return result;
}
