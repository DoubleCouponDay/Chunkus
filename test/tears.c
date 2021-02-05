#include <stdio.h>
#include "munit.h"
#include "tears.h"
#include "debug.h"
#include "../src/nsvg/usage.h"

void test2setup(const MunitParameter params[], void* userdata) {
  return calloc(1, sizeof(test2stuff));
};

void test2teardown(void* fixture) {
  test2stuff* stuff = fixture;
  free_image_contents(stuff->img);
  free(stuff);
}

void* test4setup(const MunitParameter params[], void* userdata) {
  return calloc(1, sizeof(test4stuff));
};

void test4teardown(void* fixture) {
  DEBUG("freeing test 4\n");
  test4stuff* stuff = fixture;
  free_image_contents(stuff->img);
  free_chunkmap(stuff->map);
  free(fixture);
}

void* test5setup(const MunitParameter params[], void* userdata) {
  return calloc(1, sizeof(test5stuff));
};

void test5teardown(void* fixture) {
  DEBUG("freeing test 5\n");
  test5stuff* stuff = fixture;
  fclose(stuff->fp);
  free_image_contents(stuff->img);
  free(fixture);
}

void* test6setup(const MunitParameter params[], void* userdata)
{
  return calloc(1, sizeof(test6stuff));
}

void test6teardown(void* fixture)
{
  test6stuff* stuff = fixture;
  DEBUG("freeing image contents\n");
  free_image_contents(stuff->img);
  DEBUG("freeing image\n");
  free_nsvg(stuff->result.nsvg_image);  
  free_chunkmap(stuff->result.map);
  DEBUG("freeing test6stuff\n");
  free(stuff);
}

void* test69setup(const MunitParameter params[], void* userdata)
{
  return calloc(1, sizeof(test69stuff));
}

void test69teardown(void* fixture)
{
  test69stuff* stuff = fixture;
  free_image_contents(stuff->img);
  free_chunkmap(stuff->map);
  free(fixture);
}

void* test8setup(const MunitParameter params[], void* userdata) {
  return calloc(1, sizeof(test8stuff));
}

void test8teardown(void* fixture) {
  test8stuff* stuff = fixture;
  free_image_contents(stuff->img);
  free_nsvg(stuff->result.nsvg_image);
  free_chunkmap(stuff->result.map);
  free(fixture);
}