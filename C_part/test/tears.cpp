#include <stdio.h>
#include "munit.h"
#include "tears.h"
#include "utility/logger.h"
#include "../src/nsvg/usage.h"

void *test2setup(const MunitParameter params[], void* userdata) {
  return new test2stuff();
};

void test2teardown(void* fixture) {
  delete reinterpret_cast<test2stuff*>(fixture);
}

void* test4setup(const MunitParameter params[], void* userdata) {
  return new test4stuff();
};

void test4teardown(void* fixture) {
  delete reinterpret_cast<test4stuff*>(fixture);  
}

void* test5setup(const MunitParameter params[], void* userdata) {
  return new test5stuff();
};

void test5teardown(void* fixture) {
  LOG_INFO("freeing test 5");
  test5stuff* stuff = reinterpret_cast<test5stuff*>(fixture);
  fclose(stuff->fp);
  delete stuff;
}

void* test6setup(const MunitParameter params[], void* userdata)
{
  return new test6stuff();
}

void test6teardown(void* fixture)
{
  delete reinterpret_cast<test6stuff*>(fixture);
}

void* test69setup(const MunitParameter params[], void* userdata)
{
  return new test69stuff();
}

void test69teardown(void* fixture)
{
  delete reinterpret_cast<test69stuff*>(fixture);
}

void* test8setup(const MunitParameter params[], void* userdata) {
  return new test8stuff();
}

void test8teardown(void* fixture) {
  delete reinterpret_cast<test8stuff*>(fixture);
}

void* speedy_vectorize_setup(const MunitParameter params[], void* userdata)
{
  return new speedy_vectorize_stuff();
}

void speedy_vectorize_teardown(void* fixture)
{
  delete reinterpret_cast<speedy_vectorize_stuff*>(fixture);
}