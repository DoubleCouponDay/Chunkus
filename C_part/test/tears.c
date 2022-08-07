#include <stdio.h>
#include "munit.h"
#include "tears.h"

#include "../src/nsvg/usage.h"
#include "../src/utility/logger.h"

void* tear1setup(const MunitParameter params[], void* userdata) {
  return calloc(1, sizeof(tear1));
};

void tear1teardown(void* fixture) {
  tear1* stuff = fixture;
  free_image_contents(stuff->img);
  free(stuff);
}

void* tear2setup(const MunitParameter params[], void* userdata) {
  return calloc(1, sizeof(tear2));
};

void tear2teardown(void* fixture) {
  LOG_INFO("freeing test 4");
  tear2* stuff = fixture;
  free_image_contents(stuff->img);
  free_chunkmap(stuff->map);
  free(fixture);
}

void* tear3setup(const MunitParameter params[], void* userdata) {
  return calloc(1, sizeof(tear3));
};

void tear3teardown(void* fixture) {
  LOG_INFO("freeing test 5");
  tear3* stuff = fixture;
  fclose(stuff->fp);
  free_image_contents(stuff->img);
  free(fixture);
}

void* tear4setup(const MunitParameter params[], void* userdata)
{
  return calloc(1, sizeof(tear4));
}

void tear4teardown(void* fixture)
{
  tear4* stuff = fixture;
  LOG_INFO("freeing image contents");
  free_image_contents(stuff->img);
  LOG_INFO("freeing image");
  free_nsvg(stuff->nsvg_image);  
  LOG_INFO("freeing test6stuff");
  free(stuff);
}
