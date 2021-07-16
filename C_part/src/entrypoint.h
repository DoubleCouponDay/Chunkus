#pragma once

extern const int NUM_COLOURS;

typedef struct {
    char* file_path;
    int chunk_size;
    float shape_colour_threshhold;
    int num_colours;
} vectorize_options;

int entrypoint(int argc, char* argv[]);
int set_algorithm(char* argv);
int just_crash();

extern const char* format1_p;
extern const char* format2_p;