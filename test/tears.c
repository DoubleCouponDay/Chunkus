#include <stdio.h>
#include "readpng.h"
#include "munit.h"
#include "tears.h"

void test2teardown(void* fixture) {
  if (fixture == NULL)
    return;

  filesetup* file_setup = (filesetup*)fixture;

  if (file_setup->file)
    fclose(file_setup->file);
}

void* test3setup(const MunitParameter params[], void* userdata) {
  filesetup* setup = createfilesetup(params, NULL);
  return readfile(params, setup);
}

void test3teardown(void* fixture) {
  fileresources* resources = fixture;
  void* accessaVoidproperty = resources->setup;
  test2teardown(accessaVoidproperty);
  freefile(fixture); 
}

void* test4setup(const MunitParameter params[], void* userdata) {
  return malloc(sizeof(test4stuff));
};

void test4teardown(void* fixture) {
  DEBUG("freeing test 4\n");
  test4stuff* stuff = fixture;
  free_image_contents(stuff->img);
  free_group_map(stuff->map);
  free(fixture);
}

void* test5setup(const MunitParameter params[], void* userdata) {
  return malloc(sizeof(test5stuff));
};

void test5teardown(void* fixture) {
  DEBUG("freeing test 5\n");
  test5stuff* stuff = fixture;
  fclose(stuff->fp);
  free_image_contents(stuff->img);
  free(fixture);
}

void* test7setup(const MunitParameter params[], void* userdata)
{
  return malloc(sizeof(test7stuff));
}

void test7teardown(void* fixture)
{
  test7stuff *stuff = fixture;

  free_image_contents(stuff->img);
  free_group_map(stuff->map);
  free_image(stuff->svg);  


  free(stuff);
}