#include "imagefile.h"

#include "../test/tools.h"
#include "error.h"

#include <png.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void generateBitmapImage(unsigned char* image_p, int height, int width, char* imageFileName_p);
unsigned char* createBitmapFileHeader(int height, int stride);
unsigned char* createBitmapInfoHeader(int height, int width);

const int BYTES_PER_PIXEL = 3; /// red, green, & blue
const int FILE_HEADER_SIZE = 14;
const int INFO_HEADER_SIZE = 40;

extern int error_code;

/// Takes a filename (assumed to be a png file), and creates an image struct full of the png's pixels
/// 
/// Steps involve:
/// Open the file for reading
/// Create necessary libpng structs and populate them
/// Allocate memory to hold the image contents
/// Read the image into the memory
/// Convert the obtained memory into a contiguous format (convert the row pointers from a 2 dimensional array into a single array)
image convert_png_to_image(char *fileaddress)
{
    DEBUG("opening image file...\n");

    if (!fileaddress)
        return (image){NULL, 0, 0};

    /// Open File
    FILE* file_p = fopen(fileaddress, "rb");

    if (!file_p)
    {
        DEBUG("Could not open file '%s' for reading\n", fileaddress);
        return (image){NULL, 0, 0};
    }
    else
    {
        DEBUG("File '%s' exists!\n", fileaddress);
    }

    /// Verify File
    DEBUG("Checking if file is PNG type\n");

    unsigned char header[8];
    fread(header, 1, 8, file_p);
    if (png_sig_cmp(header, 0, 8))
    {
        DEBUG("File \'%s\' was not recognised as a PNG file\n", fileaddress);
        error_code = NOT_PNG;
        return (image){NULL, 0, 0};
    }
    
    /// Prepare and read structs
    DEBUG("Creating png_image struct\n");

    png_byte color_type, bit_depth;

    // Return struct
    png_image image_struct;
    image_struct.opaque = NULL; 
    image_struct.version = PNG_IMAGE_VERSION;

    DEBUG("creating pnglib read struct...\n");

    png_structp read_struct = png_create_read_struct(
        PNG_LIBPNG_VER_STRING, NULL, NULL, NULL
    );

    if (!read_struct)
    {
        DEBUG("Failed to create png read struct\n");
        return (image){NULL, 0, 0};
    }
    
    DEBUG("Creating pnglib info struct...\n");

    png_infop info = png_create_info_struct(read_struct);
    if (!info)
    {
        DEBUG("Error: png_create_info_struct failed\n");
        return (image) { NULL, 0, 0 };
    }

    if (setjmp(png_jmpbuf(read_struct)))
    {
        DEBUG("Error during init_io\n");
        png_destroy_read_struct(read_struct, info, NULL);
        return (image) { NULL, 0, 0 };
    }

    DEBUG("Beginning PNG Reading \n");

    // Start Reading
    png_init_io(read_struct, file_p);
    png_set_sig_bytes(read_struct, 8);

    png_read_info(read_struct, info);

    DEBUG("Reading image width/height and allocating image space\n");
    image output = create_image(png_get_image_width(read_struct, info), png_get_image_height(read_struct, info));

    color_type = png_get_color_type(read_struct, info);
    if (color_type != PNG_COLOR_TYPE_RGB && color_type != PNG_COLOR_TYPE_RGBA)
    {
        DEBUG("Only RGB/A PNGs are supported for import, format: %d\n", color_type);
        return (image){NULL, 0, 0};
    }
    bit_depth = png_get_bit_depth(read_struct, info);
    if (bit_depth != 8)
    {
        DEBUG("Only 24bpp PNGs are supported, depth: %d \n", bit_depth * 3);
        return (image){NULL, 0, 0};
    }

    png_read_update_info(read_struct, info);

    if (setjmp(png_jmpbuf(read_struct)))
    {
        DEBUG("Error during early PNG reading \n");
        png_destroy_read_struct(read_struct, info, NULL);
        return (image){NULL, 0, 0};
    }

    DEBUG("Allocating row pointers...\n");

    DEBUG("dimensions: %d x %d \n", output.width, output.height);

    // Allocate row pointers to be filled
    png_bytep* row_pointers_p = (png_bytep*)calloc(1, sizeof(png_bytep) * output.height);

    for (int y = 0; y < output.height; ++y)
    {
        row_pointers_p[y] = (png_byte*)calloc(1, png_get_rowbytes(read_struct, info));
    }

    DEBUG("reading the image...\n");
    
    // Switch to RGB format, and fill the row pointers with values
    image_struct.format = PNG_FORMAT_RGB;
    png_read_image(read_struct, row_pointers_p);

    DEBUG("closing image file...\n");

    // Clean up the file
    fclose(file_p);
    png_destroy_read_struct(&read_struct, &info, NULL);
    
    DEBUG("putting dereferenced row pointers in custom struct...\n");

    if (color_type == PNG_COLOR_TYPE_RGB)
    {
        DEBUG("Type is RGB\n");
        for (int y = 0; y < output.height; ++y)
        {
            png_byte *row_p = row_pointers_p[y];

            for (int x = 0; x < output.width; ++x)
            {
                png_byte *pixel_p = &(row_p[x * 3]);

                output.pixels_array_2d[x][y].r = pixel_p[0];
                output.pixels_array_2d[x][y].g = pixel_p[1];
                output.pixels_array_2d[x][y].b = pixel_p[2];

                output.pixels_array_2d[x][y].location = (coordinate){
                    x, y};
            }
        }
    }
    else if (color_type == PNG_COLOR_TYPE_RGBA)
    {
        DEBUG("Type is RGBA\n");
        for (int y = 0; y < output.height; ++y)
        {
            png_byte *row_p = row_pointers_p[y];

            for (int x = 0; x < output.width; ++x)
            {
                png_byte *pixel_p = &(row_p[x * 4]);

                output.pixels_array_2d[x][y].r = pixel_p[0];
                output.pixels_array_2d[x][y].g = pixel_p[1];
                output.pixels_array_2d[x][y].b = pixel_p[2];

                output.pixels_array_2d[x][y].location = (coordinate){
                    x, y};
            }
        }
    }
    else
    {
        DEBUG("color type is not RGBA\n");
    }

    for (int i = 0; i < output.height; ++i)
        free(row_pointers_p[i]);
        
    free(row_pointers_p);
    
    DEBUG("png file converted to image struct.\n");
    return output;
}

void write_image_to_file(image img, char* fileaddress_p) {
    if (!img.pixels_array_2d || !fileaddress_p)
        return;

    unsigned char *as_bytes = calloc(1, BYTES_PER_PIXEL * img.height * img.width);

    for (int x = 0; x < img.width; ++x)
    {
        for (int y = 0; y < img.height; ++y)
        {
            int index = x * 3 + 0 + y * BYTES_PER_PIXEL * img.width;
            as_bytes[index]     = img.pixels_array_2d[x][y].b;
            as_bytes[index + 1] = img.pixels_array_2d[x][y].g;
            as_bytes[index + 2] = img.pixels_array_2d[x][y].r;
        }
    }
    generateBitmapImage(as_bytes, img.height, img.width, fileaddress_p);
}

void write_image_to_png_file(image img, char* fileaddress)
{
    if (!img.pixels_array_2d || !fileaddress)
        return;

FILE* fp = fopen(fileaddress, "wb");
    if (!fp)
    {
        DEBUG("File: %s not found\n", fileaddress);
        return;
    }

    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr)
    {
        DEBUG("Couldn't create png_struct for writing\n");
        return;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
        DEBUG("Couldn't create png_info struct for writing\n");
        return;
    }

    if (setjmp(png_jmpbuf(png_ptr)))
    {
        DEBUG("Unknown Failure during png writing\n");
        png_destroy_write_struct(&png_ptr, &info_ptr);
        return;
    }

    png_init_io(png_ptr, fp);

    // Output 8bit RGBA
    png_set_IHDR(
        png_ptr,
        info_ptr,
        img.width, 
        img.height,
        8,
        PNG_COLOR_TYPE_RGB,
        PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_DEFAULT,
        PNG_FILTER_TYPE_DEFAULT
    );

    png_write_info(png_ptr, info_ptr);

    // Popular some row_pointers
    png_bytep *row_pointers = calloc(img.height, sizeof(png_bytep));
    for (int y = 0; y < img.height; ++y)
    {
        row_pointers[y] = calloc(img.width, sizeof(png_byte) * 3);
        for (int x = 0; x < img.width; ++x)
        {
            row_pointers[y][x * 3 + 0] = img.pixels_array_2d[x][y].r;
            row_pointers[y][x * 3 + 1] = img.pixels_array_2d[x][y].g;
            row_pointers[y][x * 3 + 2] = img.pixels_array_2d[x][y].b;
        }
    }

    png_write_image(png_ptr, row_pointers);
    png_write_end(png_ptr, NULL);

    // Clean up
    for (int y = 0; y < img.height; ++y)
    {
        free(row_pointers[y]);
    }
    free(row_pointers);

    fclose(fp);

    png_destroy_write_struct(&png_ptr, &info_ptr);
}

void write_bytes_to_png(unsigned char* data, int width, int height, char* fileaddress)
{
    FILE* fp = fopen(fileaddress, "wb");
    if (!fp)
    {
        DEBUG("File: %s not found\n", fileaddress);
        return;
    }

    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr)
    {
        DEBUG("Couldn't create png_struct for writing\n");
        return;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
        DEBUG("Couldn't create png_info struct for writing\n");
        return;
    }

    if (setjmp(png_jmpbuf(png_ptr)))
    {
        DEBUG("Unknown Failure during png writing\n");
        png_destroy_write_struct(&png_ptr, &info_ptr);
        return;
    }

    png_init_io(png_ptr, fp);

    // Output 8bit RGBA
    png_set_IHDR(
        png_ptr,
        info_ptr,
        width, 
        height,
        8,
        PNG_COLOR_TYPE_RGB,
        PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_DEFAULT,
        PNG_FILTER_TYPE_DEFAULT
    );

    png_write_info(png_ptr, info_ptr);

    // Popular some row_pointers
    png_bytep *row_pointers = calloc(height, sizeof(png_bytep));
    for (int y = 0; y < height; ++y)
    {
        row_pointers[y] = calloc(width, sizeof(png_byte) * 3);
        for (int x = 0; x < width; ++x)
        {
            row_pointers[y][x * 3 + 0] = data[x * 3 + 2 + y * height];
            row_pointers[y][x * 3 + 1] = data[x * 3 + 1 + y * height];
            row_pointers[y][x * 3 + 2] = data[x * 3 + 0 + y * height];
        }
    }

    png_write_image(png_ptr, row_pointers);
    png_write_end(png_ptr, NULL);

    // Clean up
    for (int y = 0; y < height; ++y)
    {
        free(row_pointers[y]);
    }
    free(row_pointers);

    fclose(fp);

    png_destroy_write_struct(&png_ptr, &info_ptr);
}

/// Writes given pixelchunk map to file as if it was an image
void write_node_map_to_file(chunkmap* map, char *fileaddress)
{
    if (!map->groups_array_2d || !fileaddress)
        return;

    unsigned char* as_bytes = calloc(1, BYTES_PER_PIXEL * map->map_height * map->map_width);

    for (int x = 0; x < map->map_width; ++x)
    {
        for (int y = 0; y < map->map_height; ++y)
        {
            int index = x * 3 + y * BYTES_PER_PIXEL * map->map_width;
            as_bytes[index]     = map->groups_array_2d[x][y].average_colour.b;
            as_bytes[index + 1] = map->groups_array_2d[x][y].average_colour.g;
            as_bytes[index + 2] = map->groups_array_2d[x][y].average_colour.r;
        }
    }

    generateBitmapImage(as_bytes, map->map_height, map->map_width, fileaddress);
}

void generateBitmapImage(unsigned char* image, int height, int width, char* imageFileName)
{
    int widthInBytes = width * BYTES_PER_PIXEL;

    unsigned char padding[3] = {0, 0, 0};
    int paddingSize = (4 - (widthInBytes) % 4) % 4;

    int stride = (widthInBytes) + paddingSize;

    FILE* imageFile = fopen(imageFileName, "wb");

    unsigned char* fileHeader = createBitmapFileHeader(height, stride);
    fwrite(fileHeader, 1, FILE_HEADER_SIZE, imageFile);

    unsigned char* infoHeader = createBitmapInfoHeader(height, width);
    fwrite(infoHeader, 1, INFO_HEADER_SIZE, imageFile);

    int i;
    for (i = 0; i < height; i++) {
        fwrite(image + (i*widthInBytes), BYTES_PER_PIXEL, width, imageFile);
        fwrite(padding, 1, paddingSize, imageFile);
    }

    fclose(imageFile);
}

unsigned char* createBitmapFileHeader (int height, int stride)
{
    int fileSize = FILE_HEADER_SIZE + INFO_HEADER_SIZE + (stride * height);

    static unsigned char fileHeader[] = {
        0,0,     /// signature
        0,0,0,0, /// image file size in bytes
        0,0,0,0, /// reserved
        0,0,0,0, /// start of pixel array
    };

    fileHeader[ 0] = (unsigned char)('B');
    fileHeader[ 1] = (unsigned char)('M');
    fileHeader[ 2] = (unsigned char)(fileSize      );
    fileHeader[ 3] = (unsigned char)(fileSize >>  8);
    fileHeader[ 4] = (unsigned char)(fileSize >> 16);
    fileHeader[ 5] = (unsigned char)(fileSize >> 24);
    fileHeader[10] = (unsigned char)(FILE_HEADER_SIZE + INFO_HEADER_SIZE);

    return fileHeader;
}

unsigned char* createBitmapInfoHeader (int height, int width)
{
    static unsigned char infoHeader[] = {
        0,0,0,0, /// header size
        0,0,0,0, /// image width
        0,0,0,0, /// image height
        0,0,     /// number of color planes
        0,0,     /// bits per pixel
        0,0,0,0, /// compression
        0,0,0,0, /// image size
        0,0,0,0, /// horizontal resolution
        0,0,0,0, /// vertical resolution
        0,0,0,0, /// colors in color table
        0,0,0,0, /// important color count
    };

    infoHeader[ 0] = (unsigned char)(INFO_HEADER_SIZE);
    infoHeader[ 4] = (unsigned char)(width      );
    infoHeader[ 5] = (unsigned char)(width >>  8);
    infoHeader[ 6] = (unsigned char)(width >> 16);
    infoHeader[ 7] = (unsigned char)(width >> 24);
    infoHeader[ 8] = (unsigned char)(height      );
    infoHeader[ 9] = (unsigned char)(height >>  8);
    infoHeader[10] = (unsigned char)(height >> 16);
    infoHeader[11] = (unsigned char)(height >> 24);
    infoHeader[12] = (unsigned char)(1);
    infoHeader[14] = (unsigned char)(BYTES_PER_PIXEL*8);

    return infoHeader;
}

void write_ppm(image img, char *file_name)
{
    int x, y;
  /* 2D array for colors (shades of gray) */
  unsigned char* data = calloc(1, img.height * img.width * 3);
  /* color component is coded from 0 to 255 ;  it is 8 bit color file */
  const int MaxColorComponentValue = 255;
  FILE * fp;
  /* comment should start with # */
  const char *comment = "# this is my new binary pgm file";
 
  /* fill the data array */
  for (y = 0; y < img.height; ++y) {
    for (x = 0; x < img.width; ++x) {
        int index = y * 3 + x * img.height * 3;
        data[index]     = img.pixels_array_2d[x][y].r;
        data[index + 1] = img.pixels_array_2d[x][y].g;
        data[index + 2] = img.pixels_array_2d[x][y].b;
    }
  }
 
  /* write the whole data array to ppm file in one step */
  /* create new file, give it a name and open it in binary mode */
  fp = fopen(file_name, "wb\n");
  /* write header to the file */
  fprintf(fp, "P6\n%d %d\n 255\n", img.width, img.height);
  /* write image data bytes to the file */
  fwrite(data, 3, img.width * img.height, fp);
  fclose(fp);
}

void write_ppm_map(chunkmap map, char* filename)
{
    int x, y;
  /* 2D array for colors (shades of gray) */
  unsigned char* data = calloc(1, map.map_height * map.map_width * 3);
  /* color component is coded from 0 to 255 ;  it is 8 bit color file */
  const int MaxColorComponentValue = 255;
  FILE * fp;
  /* comment should start with # */
  const char *comment = "# this is my new binary pgm file";
 
  /* fill the data array */
  for (y = 0; y < map.map_height; ++y) {
    for (x = 0; x < map.map_width; ++x) {
      int index = y * 3 + x * map.map_height * 3;
      data[index]       = map.groups_array_2d[x][y].average_colour.r;
      data[index + 1]   = map.groups_array_2d[x][y].average_colour.g;
      data[index + 2]   = map.groups_array_2d[x][y].average_colour.b;
    }
  }
 
  /* write the whole data array to ppm file in one step */
  /* create new file, give it a name and open it in binary mode */
  fp = fopen(filename, "wb \n");
  /* write header to the file */
  fprintf(fp, "P6\n%d %d\n 255\n", map.map_width, map.map_height);
  /* write image data bytes to the file */
  fwrite(data, 3, map.map_width * map.map_height, fp);
  fclose(fp);
}

bool iterate_through_chunk(const void* item, void* udata)
{
    pixelchunk* chunk = item;    
    struct write_node_map_chunks_struct* stuff = udata;
    struct nodemap* map = stuff->map;
    
    if (chunk->location.x < 0 || chunk->location.y < 0 || chunk->location.x >= map->width || chunk->location.y >= map->height)
        return true;

    //DEBUG("Writing (%d, %d, %d) to pos (%d, %d)\n", stuff->colour.r, stuff->colour.g, stuff->colour.b, chunk->location.x, chunk->location.y);
    map->colours[chunk->location.x + map->width * chunk->location.y] = stuff->colour;
    return true;
}

void write_chunkmap_to_file(chunkmap* map, char* fileaddress)
{
    if (map->map_width < 1 || map->map_height < 1)
    {
        DEBUG("can not write 0 dimension chunkmap to file\n");
        return;
    }

    const colour shape_colours[] = { { 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x33 }, { 0x00, 0x00, 0x66 }, { 0x00, 0x00, 0x99 }, { 0x00, 0x00, 0xcc }, { 0x00, 0x00, 0xff }, { 0x00, 0x33, 0x00 }, { 0x00, 0x33, 0x33 }, { 0x00, 0x33, 0x66 }, { 0x00, 0x33, 0x99 }, { 0x00, 0x33, 0xcc }, { 0x00, 0x33, 0xff }, { 0x00, 0x66, 0x00 }, { 0x00, 0x66, 0x33 }, { 0x00, 0x66, 0x66 }, { 0x00, 0x66, 0x99 }, { 0x00, 0x66, 0xcc }, { 0x00, 0x66, 0xff }, { 0x00, 0x99, 0x00 }, { 0x00, 0x99, 0x33 }, { 0x00, 0x99, 0x66 }, { 0x00, 0x99, 0x99 }, { 0x00, 0x99, 0xcc }, { 0x00, 0x99, 0xff }, { 0x00, 0xcc, 0x00 }, { 0x00, 0xcc, 0x33 }, { 0x00, 0xcc, 0x66 }, { 0x00, 0xcc, 0x99 }, { 0x00, 0xcc, 0xcc }, { 0x00, 0xcc, 0xff }, { 0x00, 0xff, 0x00 }, { 0x00, 0xff, 0x33 }, { 0x00, 0xff, 0x66 }, { 0x00, 0xff, 0x99 }, { 0x00, 0xff, 0xcc }, { 0x00, 0xff, 0xff }, { 0x33, 0x00, 0x00 }, { 0x33, 0x00, 0x33 }, { 0x33, 0x00, 0x66 }, { 0x33, 0x00, 0x99 }, { 0x33, 0x00, 0xcc }, { 0x33, 0x00, 0xff }, { 0x33, 0x33, 0x00 }, { 0x33, 0x33, 0x33 }, { 0x33, 0x33, 0x66 }, { 0x33, 0x33, 0x99 }, { 0x33, 0x33, 0xcc }, { 0x33, 0x33, 0xff }, { 0x33, 0x66, 0x00 }, { 0x33, 0x66, 0x33 }, { 0x33, 0x66, 0x66 }, { 0x33, 0x66, 0x99 }, { 0x33, 0x66, 0xcc }, { 0x33, 0x66, 0xff }, { 0x33, 0x99, 0x00 }, { 0x33, 0x99, 0x33 }, { 0x33, 0x99, 0x66 }, { 0x33, 0x99, 0x99 }, { 0x33, 0x99, 0xcc }, { 0x33, 0x99, 0xff }, { 0x33, 0xcc, 0x00 }, { 0x33, 0xcc, 0x33 }, { 0x33, 0xcc, 0x66 }, { 0x33, 0xcc, 0x99 }, { 0x33, 0xcc, 0xcc }, { 0x33, 0xcc, 0xff }, { 0x33, 0xff, 0x00 }, { 0x33, 0xff, 0x33 }, { 0x33, 0xff, 0x66 }, { 0x33, 0xff, 0x99 }, { 0x33, 0xff, 0xcc }, { 0x33, 0xff, 0xff }, { 0x66, 0x00, 0x00 }, { 0x66, 0x00, 0x33 }, { 0x66, 0x00, 0x66 }, { 0x66, 0x00, 0x99 }, { 0x66, 0x00, 0xcc }, { 0x66, 0x00, 0xff }, { 0x66, 0x33, 0x00 }, { 0x66, 0x33, 0x33 }, { 0x66, 0x33, 0x66 }, { 0x66, 0x33, 0x99 }, { 0x66, 0x33, 0xcc }, { 0x66, 0x33, 0xff }, { 0x66, 0x66, 0x00 }, { 0x66, 0x66, 0x33 }, { 0x66, 0x66, 0x66 }, { 0x66, 0x66, 0x99 }, { 0x66, 0x66, 0xcc }, { 0x66, 0x66, 0xff }, { 0x66, 0x99, 0x00 }, { 0x66, 0x99, 0x33 }, { 0x66, 0x99, 0x66 }, { 0x66, 0x99, 0x99 }, { 0x66, 0x99, 0xcc }, { 0x66, 0x99, 0xff }, { 0x66, 0xcc, 0x00 }, { 0x66, 0xcc, 0x33 }, { 0x66, 0xcc, 0x66 }, { 0x66, 0xcc, 0x99 }, { 0x66, 0xcc, 0xcc }, { 0x66, 0xcc, 0xff }, { 0x66, 0xff, 0x00 }, { 0x66, 0xff, 0x33 }, { 0x66, 0xff, 0x66 }, { 0x66, 0xff, 0x99 }, { 0x66, 0xff, 0xcc }, { 0x66, 0xff, 0xff }, { 0x99, 0x00, 0x00 }, { 0x99, 0x00, 0x33 }, { 0x99, 0x00, 0x66 }, { 0x99, 0x00, 0x99 }, { 0x99, 0x00, 0xcc }, { 0x99, 0x00, 0xff }, { 0x99, 0x33, 0x00 }, { 0x99, 0x33, 0x33 }, { 0x99, 0x33, 0x66 }, { 0x99, 0x33, 0x99 }, { 0x99, 0x33, 0xcc }, { 0x99, 0x33, 0xff }, { 0x99, 0x66, 0x00 }, { 0x99, 0x66, 0x33 }, { 0x99, 0x66, 0x66 }, { 0x99, 0x66, 0x99 }, { 0x99, 0x66, 0xcc }, { 0x99, 0x66, 0xff }, { 0x99, 0x99, 0x00 }, { 0x99, 0x99, 0x33 }, { 0x99, 0x99, 0x66 }, { 0x99, 0x99, 0x99 }, { 0x99, 0x99, 0xcc }, { 0x99, 0x99, 0xff }, { 0x99, 0xcc, 0x00 }, { 0x99, 0xcc, 0x33 }, { 0x99, 0xcc, 0x66 }, { 0x99, 0xcc, 0x99 }, { 0x99, 0xcc, 0xcc }, { 0x99, 0xcc, 0xff }, { 0x99, 0xff, 0x00 }, { 0x99, 0xff, 0x33 }, { 0x99, 0xff, 0x66 }, { 0x99, 0xff, 0x99 }, { 0x99, 0xff, 0xcc }, { 0x99, 0xff, 0xff }, { 0xcc, 0x00, 0x00 }, { 0xcc, 0x00, 0x33 }, { 0xcc, 0x00, 0x66 }, { 0xcc, 0x00, 0x99 }, { 0xcc, 0x00, 0xcc }, { 0xcc, 0x00, 0xff }, { 0xcc, 0x33, 0x00 }, { 0xcc, 0x33, 0x33 }, { 0xcc, 0x33, 0x66 }, { 0xcc, 0x33, 0x99 }, { 0xcc, 0x33, 0xcc }, { 0xcc, 0x33, 0xff }, { 0xcc, 0x66, 0x00 }, { 0xcc, 0x66, 0x33 }, { 0xcc, 0x66, 0x66 }, { 0xcc, 0x66, 0x99 }, { 0xcc, 0x66, 0xcc }, { 0xcc, 0x66, 0xff }, { 0xcc, 0x99, 0x00 }, { 0xcc, 0x99, 0x33 }, { 0xcc, 0x99, 0x66 }, { 0xcc, 0x99, 0x99 }, { 0xcc, 0x99, 0xcc }, { 0xcc, 0x99, 0xff }, { 0xcc, 0xcc, 0x00 }, { 0xcc, 0xcc, 0x33 }, { 0xcc, 0xcc, 0x66 }, { 0xcc, 0xcc, 0x99 }, { 0xcc, 0xcc, 0xcc }, { 0xcc, 0xcc, 0xff }, { 0xcc, 0xff, 0x00 }, { 0xcc, 0xff, 0x33 }, { 0xcc, 0xff, 0x66 }, { 0xcc, 0xff, 0x99 }, { 0xcc, 0xff, 0xcc }, { 0xcc, 0xff, 0xff }, { 0xff, 0x00, 0x00 }, { 0xff, 0x00, 0x33 }, { 0xff, 0x00, 0x66 }, { 0xff, 0x00, 0x99 }, { 0xff, 0x00, 0xcc }, { 0xff, 0x00, 0xff }, { 0xff, 0x33, 0x00 }, { 0xff, 0x33, 0x33 }, { 0xff, 0x33, 0x66 }, { 0xff, 0x33, 0x99 }, { 0xff, 0x33, 0xcc }, { 0xff, 0x33, 0xff }, { 0xff, 0x66, 0x00 }, { 0xff, 0x66, 0x33 }, { 0xff, 0x66, 0x66 }, { 0xff, 0x66, 0x99 }, { 0xff, 0x66, 0xcc }, { 0xff, 0x66, 0xff }, { 0xff, 0x99, 0x00 }, { 0xff, 0x99, 0x33 }, { 0xff, 0x99, 0x66 }, { 0xff, 0x99, 0x99 }, { 0xff, 0x99, 0xcc }, { 0xff, 0x99, 0xff }, { 0xff, 0xcc, 0x00 }, { 0xff, 0xcc, 0x33 }, { 0xff, 0xcc, 0x66 }, { 0xff, 0xcc, 0x99 }, { 0xff, 0xcc, 0xcc }, { 0xff, 0xcc, 0xff }, { 0xff, 0xff, 0x00 }, { 0xff, 0xff, 0x33 }, { 0xff, 0xff, 0x66 }, { 0xff, 0xff, 0x99 }, { 0xff, 0xff, 0xcc }, };
    colour* colours = calloc(map->map_width * map->map_height, sizeof(struct colour));

    nodemap intermediate = {
        colours,
        map->map_width,
        map->map_height
    };

    int cur_colour = 0;

    DEBUG("now iterating chunkshapes in chunkmap with dims %d x %d\n", map->map_width, map->map_height);
    chunkshape* current = map->shape_list;
    int shape_count = 0;

    while (current)
    {
        //DEBUG("shape %d has %d chunks\n", shape_count, hashmap_count(current->chunks));
        shape_count++;

        write_node_map_chunks_struct stuff = {
            shape_colours[cur_colour],
            &intermediate
        };

        hashmap_scan(current->chunks, iterate_through_chunk, &stuff);

        current = current->next;
        int array_size = sizeof(shape_colours);
        int colour_size = sizeof(colour);
        cur_colour = (cur_colour + 1 < (array_size / colour_size) ? cur_colour + 1 : 0);
    }
    DEBUG("iterated %d shapes in chunkmap\n", shape_count);
    image output_img = create_image(intermediate.width * 3, intermediate.height * 3);
    
    for (int x = 0; x < intermediate.width; ++x)
    {
        for (int y = 0; y < intermediate.height; ++y)
        {
            colour* bob = &intermediate.colours[x + intermediate.width * y];
            for (int xx = 0; xx < 3; ++xx)
            {
                for (int yy = 0; yy < 3; ++yy)
                {
                    pixel* img_pix = &(output_img.pixels_array_2d[x * 3 + xx][y * 3 + yy]);
                    img_pix->r = bob->r;
                    img_pix->g = bob->g;
                    img_pix->b = bob->b;
                }
            }
        }
    }

    write_image_to_png_file(output_img, fileaddress);
}
