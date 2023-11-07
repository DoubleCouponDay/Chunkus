#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "utility/error.h"

typedef struct {
    const char* file_path;
    int chunk_size;
    int num_colours;
	int64_t step_index; //optional argument
} vectorize_options;

int execute_program(vectorize_options options);
int entrypoint(int argc, char* argv[]);
int just_crash();

#ifdef __cplusplus
};
#endif
