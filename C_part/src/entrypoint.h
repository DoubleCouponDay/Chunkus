#pragma once

int entrypoint(int argc, char* argv[]);
int debug_image(char* input, int chunk_size, float threshold, char* shape_file, char* border_file);
int set_algorithm(int algo);

extern const char* format1_p;
extern const char* format2_p;