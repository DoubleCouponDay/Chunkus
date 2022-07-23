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
#include "../src/nsvg/dcdfiller.h"
#include "../src/utility/logger.h"
#include "../src/imagefile/jpegfile.h"

MunitResult aTestCanPass(const MunitParameter params[], void* data) {
  LOG_INFO("test 1 passed");
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
  fclose(fp);
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
  LOG_INFO("asserting pixels_array_2d not null");
  munit_assert_ptr_not_null(stuff->img.pixels_array_2d);
  LOG_INFO("generating chunkmap");
  chunkmap* map = generate_chunkmap(stuff->img, options);
  munit_assert_int(getAndResetErrorCode(), ==, SUCCESS_CODE);
  stuff->map = map;

  LOG_INFO("asserting groups_array_2d not null");
  munit_assert_ptr_not_null(map->groups_array_2d);
  LOG_INFO("filling chunkmap");
  fill_chunkmap(stuff->map, &options);
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
  LOG_INFO("asserting pixels_array_2d not null");
  munit_assert_ptr_not_null(stuff->img.pixels_array_2d);
  munit_assert_int(getAndResetErrorCode(), ==, SUCCESS_CODE);

  stuff->nsvg_image = dcdfill_for_nsvg(stuff->img, options);
  munit_assert_int(getAndResetErrorCode(), ==, SUCCESS_CODE);

  bool outcome = write_svg_file(stuff->nsvg_image, OUTPUT_PATH);
  
  if(outcome)
    LOG_INFO("svg writing outcome: %s", "succeeded");

  else 
    LOG_INFO("svg writing outcome: %s", "failed");
    
  munit_assert_int(getAndResetErrorCode(), ==, SUCCESS_CODE);

  FILE* fp = fopen(OUTPUT_PATH, "r"); //check it at least creates a file every time
  munit_assert_ptr_not_null(fp);
  fclose(fp);

  return MUNIT_OK;
}

MunitResult just_run(const MunitParameter params[], void* userdata) {
  entrypoint(0, NULL);
  return MUNIT_OK;
}

MunitResult JPEG_to_image(const MunitParameter params[], void* userdata) {
  char* inputjpeg = params[5].value;
  image result = convert_file_to_image(inputjpeg);
  munit_assert_int(getAndResetErrorCode(), ==, SUCCESS_CODE);
  munit_assert(result.width != 0);
  munit_assert(result.height != 0);
  munit_assert_ptr_not_null(result.pixels_array_2d);
  free_image_contents(result);
  return MUNIT_OK;
}

MunitResult can_convert_jpeg_to_bmp(const MunitParameter params[], void* userdata){
  jpeg_bmp_stuff* stuff = userdata;
  // Use constant input/output path
  char* in_file = params[5].value;
  char* out_file = "peach.bmp";

  // Delete output file
  remove(out_file);

  stuff->img = convert_file_to_image(in_file);

  munit_assert_ptr_not_null(stuff->img.pixels_array_2d); // FAILED TO CONVERT IMAGE

  write_image_to_bmp(stuff->img, out_file);

  FILE* fp = fopen(out_file, "r");
  stuff->fp = fp;
  munit_assert_ptr_not_null(fp); // OUTPUT FILE NOT FOUND
  fclose(fp);
  return MUNIT_OK;
}

MunitResult jpeg_dcd(const MunitParameter params[], void* userdata) {
    test8stuff* stuff = userdata;

  char* inputjpeg = params[5].value;

  char* chunk_size_str = params[1].value;
  int chunk_size = atoi(chunk_size_str);

  char* threshold_str = params[2].value;
  float threshold = atof(threshold_str);
  
  char* out_fileaddress = params[3].value;
  int num_colours = atoi(params[4].value);

  vectorize_options options = {
    inputjpeg,
    chunk_size,
    threshold,
    num_colours
  };

  stuff->img = convert_jpeg_to_image(inputjpeg);
  LOG_INFO("asserting pixels_array_2d not null");
  munit_assert_ptr_not_null(stuff->img.pixels_array_2d);
  munit_assert_int(getAndResetErrorCode(), ==, SUCCESS_CODE);

  stuff->nsvg_image = dcdfill_for_nsvg(stuff->img, options);
  munit_assert_int(getAndResetErrorCode(), ==, SUCCESS_CODE);

  bool outcome = write_svg_file(stuff->nsvg_image, OUTPUT_PATH);
  
  if(outcome)
    LOG_INFO("svg writing outcome: %s", "succeeded");

  else 
    LOG_INFO("svg writing outcome: %s", "failed");
    
  munit_assert_int(getAndResetErrorCode(), ==, SUCCESS_CODE);

  FILE* fp = fopen(OUTPUT_PATH, "r"); //check it at least creates a file every time
  munit_assert_ptr_not_null(fp);
  fclose(fp);

  return MUNIT_OK;
}
