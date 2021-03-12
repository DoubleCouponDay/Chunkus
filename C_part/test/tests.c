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
#include "../src/imagefile/converter.h"

MunitResult aTestCanPass(const MunitParameter params[], void* data) {
  DEBUG_OUT("test 1 passed");
  return MUNIT_OK;
}

MunitResult can_read_png(const MunitParameter params[], void* userdata) {
  test2stuff* stuff = userdata;
  char* filepath = params[0].value;
  image subject = convert_png_to_image(filepath);
  munit_assert_int(getAndResetErrorCode(), ==, SUCCESS_CODE);
  stuff->img = subject;
  return MUNIT_OK;
}

MunitResult can_convert_png_to_chunkmap(const MunitParameter params[], void* userdata) {
  test4stuff* stuff = userdata;
  int num_colours = atoi(params[4].value);
  
  vectorize_options options = {
    params[0].value,
    atoi(params[1].value),
    atof(params[2].value),
    num_colours
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
  char* out_file = "peach.bmp";

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
  int num_colours = atoi(params[4].value);

  vectorize_options options = {
    params[0].value,
    atoi(params[1].value),
    atof(params[2].value),
    num_colours
  };
  
  stuff->img = convert_png_to_image(options.file_path);
  stuff->nsvg_image = dcdfill_for_nsvg(stuff->img, options);

  munit_assert_int(getAndResetErrorCode(), ==, SUCCESS_CODE);
  munit_assert_ptr_not_null(stuff->nsvg_image);

  return MUNIT_OK;
}

MunitResult can_write_chunkmap_shapes_to_file(const MunitParameter params[], void* userdata)
{
  test69stuff* stuff = userdata;

  char* fileaddress = params[0].value;

  char* chunk_size_str = params[1].value;
  int chunk_size = atoi(chunk_size_str);

  char* threshold_str = params[2].value;
  float threshold = atof(threshold_str);
  
  char* out_fileaddress = params[3].value;
  int num_colours = atoi(params[4].value);

  vectorize_options options = {
    fileaddress,
    chunk_size,
    threshold,
    num_colours
  };

  stuff->img = convert_png_to_image(fileaddress);
  DEBUG_OUT("asserting pixels_array_2d not null");
  munit_assert_ptr_not_null(stuff->img.pixels_array_2d);
  DEBUG_OUT("generating chunkmap");
  chunkmap* map = generate_chunkmap(stuff->img, options);
  munit_assert_int(getAndResetErrorCode(), ==, SUCCESS_CODE);
  stuff->map = map;

  DEBUG_OUT("asserting groups_array_2d not null");
  munit_assert_ptr_not_null(map->groups_array_2d);
  DEBUG_OUT("filling chunkmap");
  fill_chunkmap(stuff->map, &options);
  munit_assert_int(getAndResetErrorCode(), ==, SUCCESS_CODE);

  DEBUG_OUT("writing chunkmap to file");
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
  int num_colours = atoi(params[4].value);

  vectorize_options options = {
    fileaddress,
    chunk_size,
    threshold,
    num_colours
  };

  stuff->img = convert_png_to_image(fileaddress);
  DEBUG_OUT("asserting pixels_array_2d not null");
  munit_assert_ptr_not_null(stuff->img.pixels_array_2d);
  munit_assert_int(getAndResetErrorCode(), ==, SUCCESS_CODE);

  stuff->nsvg_image = dcdfill_for_nsvg(stuff->img, options);
  munit_assert_int(getAndResetErrorCode(), ==, SUCCESS_CODE);

  bool outcome = write_svg_file(stuff->nsvg_image);
  
  if(outcome)
    DEBUG_OUT("svg writing outcome: %s", "succeeded");

  else 
    DEBUG_OUT("svg writing outcome: %s", "failed");
    
  munit_assert_int(getAndResetErrorCode(), ==, SUCCESS_CODE);

  FILE* fp = fopen(OUTPUT_PATH, "r"); //check it at least creates a file every time

  munit_assert_ptr_not_null(fp);

  fclose(fp);

  return MUNIT_OK;
}

MunitResult can_do_speedy_vectorize(const MunitParameter params[], void* userdata)
{
  speedy_vectorize_stuff* stuff = userdata;

  char* fileaddress = params[0].value;

  char* chunk_size_str = params[1].value;
  int chunk_size = atoi(chunk_size_str);

  char* threshold_str = params[2].value;
  float threshold = atof(threshold_str);
  
  char* out_fileaddress = params[3].value;
  int num_colours = atoi(params[4].value);

  vectorize_options options = {
    fileaddress,
    chunk_size,
    threshold,
    num_colours
  };

  stuff->img = convert_png_to_image(fileaddress);
  munit_assert_ptr_not_null(stuff->img.pixels_array_2d);
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

MunitResult just_run(const MunitParameter params[], void* userdata) {
  entrypoint(0, NULL);
}

MunitResult JPEG_to_image(const MunitParameter params[], void* userdata) {
  char* inputjpeg = params[5].value;
  image result = convert_file_to_image(inputjpeg);
  munit_assert_int(getAndResetErrorCode(), ==, SUCCESS_CODE);
  munit_assert(result.width != 0);
  munit_assert(result.height != 0);
  munit_assert_ptr_not_null(result.pixels_array_2d);
  free_image_contents(result);
}
