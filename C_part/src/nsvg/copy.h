#pragma once

#include <nanosvg.h>
#include <memory>

int NSVG_RGB(int r, int g, int b);

char* gettemplate(int width, int height);

void free_template(char* data);
