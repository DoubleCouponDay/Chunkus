#include "imagefile.h"

#include "tools.h"

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
    DEBUG_PRINT("opening image file...\n");

    if (!fileaddress)
        return (image){NULL, 0, 0};

    /// Open File
    FILE* file_p = fopen(fileaddress, "rb");

    if (!file_p)
    {
        DEBUG_PRINT("Could not open file '%s' for reading", fileaddress);
        return (image){NULL, 0, 0};
    }

    /// Verify File
    DEBUG_PRINT("Checking if file is PNG type\n");

    unsigned char header[8];
    fread(header, 1, 8, file_p);
    if (png_sig_cmp(header, 0, 8))
    {
        DEBUG_PRINT("File \'%s\' was not recognised as a PNG file\n", fileaddress);
        return (image){NULL, 0, 0};
    }
    
    /// Prepare and read structs
    DEBUG_PRINT("Creating png_image struct\n");

    png_byte color_type, bit_depth;

    // Return struct
    png_image image_struct;
    image_struct.opaque = NULL; 
    image_struct.version = PNG_IMAGE_VERSION;

    DEBUG_PRINT("creating pnglib read struct...\n");

    png_structp read_struct = png_create_read_struct(
        PNG_LIBPNG_VER_STRING, NULL, NULL, NULL
    );

    if (!read_struct)
    {
        DEBUG_PRINT("Failed to create png read struct");
        return (image){NULL, 0, 0};
    }
    
    DEBUG_PRINT("Creating pnglib info struct...\n");

    png_infop info = png_create_info_struct(read_struct);
    if (!info)
    {
        DEBUG_PRINT("Error: png_create_info_struct failed");
        return (image) { NULL, 0, 0 };
    }

    if (setjmp(png_jmpbuf(read_struct)))
    {
        DEBUG_PRINT("Error during init_io");
        png_destroy_read_struct(read_struct, info, NULL);
        return (image) { NULL, 0, 0 };
    }

    DEBUG_PRINT("Beginning PNG Reading \n");

    // Start Reading
    png_init_io(read_struct, file_p);
    png_set_sig_bytes(read_struct, 8);

    png_read_info(read_struct, info);

    DEBUG_PRINT("Reading image width/height and allocating image space\n");
    image output = create_image(png_get_image_width(read_struct, info), png_get_image_height(read_struct, info));

    color_type = png_get_color_type(read_struct, info);
    if (color_type != PNG_COLOR_TYPE_RGB)
    {
        DEBUG_PRINT("Only RGB PNGs are supported for import, format: %d", color_type);
        return (image){NULL, 0, 0};
    }
    bit_depth = png_get_bit_depth(read_struct, info);
    if (bit_depth != 8)
    {
        DEBUG_PRINT("Only 24bpp PNGs are supported, depth: %d", bit_depth * 3);
        return (image){NULL, 0, 0};
    }

    png_read_update_info(read_struct, info);

    if (setjmp(png_jmpbuf(read_struct)))
    {
        DEBUG_PRINT("Error during early PNG reading");
        png_destroy_read_struct(read_struct, info, NULL);
        return (image){NULL, 0, 0};
    }

    DEBUG_PRINT("Allocating row pointers...\n");

    DEBUG_PRINT("dimensions: %d x %d \n", output.width, output.height);

    // Allocate row pointers to be filled
    png_bytep* row_pointers_p = (png_bytep*)malloc(sizeof(png_bytep) * output.height);

    for (int y = 0; y < output.height; ++y)
    {
        row_pointers_p[y] = (png_byte*)malloc(png_get_rowbytes(read_struct, info));
    }

    DEBUG_PRINT("reading the image...\n");
    
    // Switch to RGB format, and fill the row pointers with values
    image_struct.format = PNG_FORMAT_RGB;
    png_read_image(read_struct, row_pointers_p);

    DEBUG_PRINT("closing image file...\n");

    // Clean up the file
    fclose(file_p);
    png_destroy_read_struct(&read_struct, &info, NULL);
    
    DEBUG_PRINT("putting dereferenced row pointers in custom struct...\n");

    for (int y = 0; y < output.height; ++y)
    {
        png_byte* row_p = row_pointers_p[y];

        for (int x = 0; x < output.width; ++x)
        {
            png_byte* pixel_p = &(row_p[x * 3]);

            output.pixels_array_2d[x][y].r = pixel_p[0];
            output.pixels_array_2d[x][y].g = pixel_p[1];
            output.pixels_array_2d[x][y].b = pixel_p[2];

            output.pixels_array_2d[x][y].location = (coordinate) {
                x, y
            };
        }
    }

    for (int i = 0; i < output.height; ++i)
        free(row_pointers_p[i]);
    free(row_pointers_p);
    
    DEBUG_PRINT("png file converted to image struct.\n");
    return output;
}



void write_image_to_file(image img, char* fileaddress_p) {
    if (!img.pixels_array_2d || !fileaddress_p)
        return;

    unsigned char *as_bytes = malloc(BYTES_PER_PIXEL * img.height * img.width);

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

/// Writes given pixelgroup map to file as if it was an image (discards variance) (assumes fileaddress ends with .bmp)
void write_node_map_to_file(groupmap map, char *fileaddress)
{
    if (!map.groups_array_2d || !fileaddress)
        return;

    unsigned char* as_bytes = malloc(BYTES_PER_PIXEL * map.map_height * map.map_width);

    for (int x = 0; x < map.map_width; ++x)
    {
        for (int y = 0; y < map.map_height; ++y)
        {
            int index = x * 3 + y * BYTES_PER_PIXEL * map.map_width;
            as_bytes[index]     = map.groups_array_2d[x][y].average_colour.b;
            as_bytes[index + 1] = map.groups_array_2d[x][y].average_colour.g;
            as_bytes[index + 2] = map.groups_array_2d[x][y].average_colour.r;
        }
    }

    generateBitmapImage(as_bytes, map.map_height, map.map_width, fileaddress);
}

void write_node_map_variance_to_file(groupmap map, char *filename)
{
    if (!map.groups_array_2d || !filename)
        return;

    unsigned char *as_bytes = malloc(BYTES_PER_PIXEL * map.map_height * map.map_width);

    for (int x = 0; x < map.map_width; ++x)
    {
        for (int y = 0; y < map.map_height; ++y)
        {
            pixel p = convert_colorf_to_pixel(map.groups_array_2d[x][y].variance);
            int index = x * 3 + y * BYTES_PER_PIXEL * map.map_width;
            as_bytes[index]     = p.b;
            as_bytes[index + 1] = p.g;
            as_bytes[index + 2] = p.r;
        }
    }
    generateBitmapImage(as_bytes, map.map_height, map.map_width, filename);
}

void generateBitmapImage (unsigned char* image, int height, int width, char* imageFileName)
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
  unsigned char* data = malloc(img.height * img.width * 3);
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

void write_ppm_map(groupmap map, char* filename)
{
    int x, y;
  /* 2D array for colors (shades of gray) */
  unsigned char* data = malloc(map.map_height * map.map_width * 3);
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