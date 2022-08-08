#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "utility/error.h"

typedef struct {
    const char* file_path;
    int chunk_size;
    float threshold;
    int num_colours;
	int step_index; //optional argument
} vectorize_options;

int execute_program(vectorize_options options);
int entrypoint(int argc, char* argv[]);
int just_crash();

#ifdef __cplusplus
};
#endif