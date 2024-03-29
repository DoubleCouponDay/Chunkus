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
#include "../src/algorithm/algorithm.h"
#include "../src/utility/logger.h"
#include "../src/imagefile/jpegfile.h"

MunitResult can_test(const MunitParameter params[], void* data) {
  LOG_INFO("test 1 passed");
  return MUNIT_OK;
}

MunitResult can_read_png(const MunitParameter params[], void* userdata) {
  tear1* stuff = userdata;
  char* filepath = params[0].value;
  image subject = convert_png_to_image(filepath);
  munit_assert_int(getAndResetErrorCode(), ==, SUCCESS_CODE);
  stuff->img = subject;
  return MUNIT_OK;
}

MunitResult can_convert_png_to_chunkmap(const MunitParameter params[], void* userdata) {
  tear2* stuff = userdata;
  char* png_input = params[0].value;
  int num_colours = atoi(params[6].value);
  int chunksize = atoi(params[3].value);
  float thresholds = atof(params[4].value);
  
  vectorize_options options = {
    png_input,
    chunksize,
    thresholds,
    0,
    num_colours
  };
  stuff->img = convert_png_to_image(options.file_path);
  chunkmap* map = generate_chunkmap(stuff->img, options);
  munit_assert_int(getAndResetErrorCode(), ==, SUCCESS_CODE);
  stuff->map = map;
  return MUNIT_OK;
}

MunitResult can_convert_png_to_bmp(const MunitParameter params[], void* userdata) {
  tear3* stuff = userdata;
  // Use constant input/output path
  char* in_file = params[0].value;
  char* out_file = params[8].value;

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

MunitResult can_vectorize_png(const MunitParameter params[], void* userdata)
{
  tear4* stuff = userdata;
  char* png_input = params[0].value;
  int num_colours = atoi(params[6].value);
  int chunksize = atoi(params[3].value);
  float thresholds = atof(params[4].value);
  
  vectorize_options options = {
    png_input,
    chunksize,
    thresholds,
    0,
    num_colours
  };
  
  stuff->img = convert_png_to_image(options.file_path);
  vectorize(stuff->img, options);

  munit_assert_int(getAndResetErrorCode(), ==, SUCCESS_CODE);

  return MUNIT_OK;
}

MunitResult can_convert_png_to_svg(const MunitParameter params[], void* userdata) {
  tear4* stuff = userdata;

  char* fileaddress = params[0].value;
  int chunk_size = atoi(params[3].value);
  float thresholds = atof(params[4].value);
  int num_colours = atoi(params[6].value);

  vectorize_options options = {
    fileaddress,
    chunk_size,
    thresholds,
    0,
    num_colours
  };

  stuff->img = convert_png_to_image(fileaddress);
  LOG_INFO("asserting pixels_array_2d not null");
  munit_assert_ptr_not_null(stuff->img.pixels_array_2d);
  munit_assert_int(getAndResetErrorCode(), ==, SUCCESS_CODE);

  vectorize(stuff->img, options);
  munit_assert_int(getAndResetErrorCode(), ==, SUCCESS_CODE);

  FILE* fp = fopen(OUTPUT_PATH, "r"); //check it at least creates a file every time
  munit_assert_ptr_not_null(fp);
  fclose(fp);

  return MUNIT_OK;
}


MunitResult can_convert_png2_to_svg(const MunitParameter params[], void* userdata) {
  tear4* stuff = userdata;

  char* fileaddress = params[1].value;
  int chunk_size = atoi(params[3].value);
  float thresholds = atof(params[4].value);
  int num_colours = atoi(params[6].value);

  vectorize_options options = {
    fileaddress,
    chunk_size,
    thresholds,
    0,
    num_colours
  };

  stuff->img = convert_png_to_image(fileaddress);
  LOG_INFO("asserting pixels_array_2d not null");
  munit_assert_ptr_not_null(stuff->img.pixels_array_2d);
  munit_assert_int(getAndResetErrorCode(), ==, SUCCESS_CODE);

  vectorize(stuff->img, options);
  munit_assert_int(getAndResetErrorCode(), ==, SUCCESS_CODE);

  FILE* fp = fopen(OUTPUT_PATH, "r"); //check it at least creates a file every time
  munit_assert_ptr_not_null(fp);
  fclose(fp);

  return MUNIT_OK;
}


MunitResult can_convert_png3_to_svg(const MunitParameter params[], void* userdata) {
  tear4* stuff = userdata;

  char* fileaddress = params[2].value;
  int chunk_size = atoi(params[3].value);
  float thresholds = atof(params[4].value);
  int num_colours = atoi(params[6].value);

  vectorize_options options = {
    fileaddress,
    chunk_size,
    thresholds,
    0,
    num_colours
  };

  stuff->img = convert_png_to_image(fileaddress);
  LOG_INFO("asserting pixels_array_2d not null");
  munit_assert_ptr_not_null(stuff->img.pixels_array_2d);
  munit_assert_int(getAndResetErrorCode(), ==, SUCCESS_CODE);

  vectorize(stuff->img, options);
  munit_assert_int(getAndResetErrorCode(), ==, SUCCESS_CODE);

  FILE* fp = fopen(OUTPUT_PATH, "r"); //check it at least creates a file every time
  munit_assert_ptr_not_null(fp);
  fclose(fp);

  return MUNIT_OK;
}

MunitResult can_read_jpeg(const MunitParameter params[], void* userdata) {
  char* inputjpeg = params[7].value;
  tear1* stuff = userdata;
  stuff->img = convert_file_to_image(inputjpeg);
  munit_assert_int(getAndResetErrorCode(), ==, SUCCESS_CODE);
  munit_assert(stuff->img.width != 0);
  munit_assert(stuff->img.height != 0);
  munit_assert_ptr_not_null(stuff->img.pixels_array_2d);
  return MUNIT_OK;
}

MunitResult can_convert_jpeg_to_bmp(const MunitParameter params[], void* userdata){
  tear3* stuff = userdata;
  // Use constant input/output path
  char* in_file = params[7].value;
  char* out_file = params[8].value;

  // Delete output file
  remove(out_file);

  stuff->img = convert_file_to_image(in_file);

  munit_assert_ptr_not_null(stuff->img.pixels_array_2d); // FAILED TO CONVERT IMAGE

  write_image_to_bmp(stuff->img, out_file);

  FILE* fp = fopen(out_file, "r");
  stuff->fp = fp;
  munit_assert_ptr_not_null(fp); // OUTPUT FILE NOT FOUND
  return MUNIT_OK;
}

MunitResult can_vectorize_jpeg(const MunitParameter params[], void* userdata) {
  tear4* stuff = userdata;

  char* inputjpeg = params[7].value;
  int chunk_size = atoi(params[3].value);
  float thresholds = atof(params[4].value);
  int num_colours = atoi(params[6].value);

  vectorize_options options = {
    inputjpeg,
    chunk_size,
    thresholds,
    0,
    num_colours
  };

  stuff->img = convert_jpeg_to_image(inputjpeg);
  LOG_INFO("asserting pixels_array_2d not null");
  munit_assert_ptr_not_null(stuff->img.pixels_array_2d);
  munit_assert_int(getAndResetErrorCode(), ==, SUCCESS_CODE);

  vectorize(stuff->img, options);
  munit_assert_int(getAndResetErrorCode(), ==, SUCCESS_CODE);

  return MUNIT_OK;
}

MunitResult can_convert_jpeg_to_svg(const MunitParameter params[], void* userdata) {
  tear4* stuff = userdata;

  char* jpegaddress = params[7].value;
  int chunk_size = atoi(params[3].value);
  float thresholds = atof(params[4].value);
  int num_colours = atoi(params[6].value);

  vectorize_options options = {
    jpegaddress,
    chunk_size,
    thresholds,
    0,
    num_colours
  };

  stuff->img = convert_jpeg_to_image(jpegaddress);
  LOG_INFO("asserting pixels_array_2d not null");
  munit_assert_ptr_not_null(stuff->img.pixels_array_2d);
  munit_assert_int(getAndResetErrorCode(), ==, SUCCESS_CODE);

  vectorize(stuff->img, options);
  munit_assert_int(getAndResetErrorCode(), ==, SUCCESS_CODE);

  FILE* fp = fopen(OUTPUT_PATH, "r"); //check it at least creates a file every time
  munit_assert_ptr_not_null(fp);
  fclose(fp);

  return MUNIT_OK;
}
