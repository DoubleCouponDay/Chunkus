#include "converter.h"

#include "tools.h"

#include <png.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    printf("opening image file...\n");

    if (!fileaddress)
        return (image){NULL, 0, 0};
    
    FILE *file = fopen(fileaddress, "rb");

    if (!file)
    {
        DEBUG_PRINT("Could not open file '%s' for reading", fileaddress);
        return (image){NULL, 0, 0};
    }
    
    printf("Creating png_image struct\n");

    png_byte color_type, bit_depth;

    // Return struct
    png_image image_struct;
    image_struct.opaque = NULL; 
    image_struct.version = PNG_IMAGE_VERSION;

    printf("creating pnglib read struct...\n");

    png_structp read_struct = png_create_read_struct(
        PNG_LIBPNG_VER_STRING, NULL, NULL, NULL
    );

    if (!read_struct)
    {
        DEBUG_PRINT("Failed to create png read struct");
        return (image){NULL, 0, 0};
    }
    
    printf("creating pnglib info struct...\n");

    png_infop info = png_create_info_struct(read_struct);
    if (setjmp(png_jmpbuf(read_struct)))
    {
        DEBUG_PRINT("Something... something...");
        return (image) { NULL, 0, 0 };
    }

    if (!info)
    {
        DEBUG_PRINT("Failed to create png info pointer");
        return (image) { NULL, 0, 0 };
    }

    printf("Getting pnglib information...\n");

    // Start Reading
    png_init_io(read_struct, file);

    png_read_info(read_struct, info);

    image final_image = create_image(png_get_image_width(read_struct, info), png_get_image_height(read_struct, info));

    color_type = png_get_color_type(read_struct, info);
    bit_depth = png_get_bit_depth(read_struct, info);

    printf("creating row pointers...\n");

    printf("dimensions: %d x %d \n", final_image.width, final_image.height);

    // Allocate row pointers to be filled
    png_bytep *row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * final_image.height);

    for (int y = 0; y < final_image.height; ++y)
    {
        row_pointers[y] = (png_byte*)malloc(png_get_rowbytes(read_struct, info));
    }

    printf("reading the image...\n");
    
    // Switch to RGB format, and fill the row pointers with values
    image_struct.format = PNG_FORMAT_RGB;
    png_read_image(read_struct, row_pointers);

    printf("closing image file...\n");

    // Clean up the file
    fclose(file);
    png_destroy_read_struct(&read_struct, &info, NULL);
    
    printf("putting dereferenced row pointers in custom struct...\n");

    for (int y = 0; y < final_image.height; ++y)
    {
        for (int x = 0; x < final_image.width; ++x)
        {
            color pix_color;
            pix_color.r = row_pointers[y][x * 3 + 0];
            pix_color.g = row_pointers[y][x * 3 + 1];
            pix_color.b = row_pointers[y][x * 3 + 2];
            final_image.pixels[y][x] = pix_color;
        }
    }
    
    printf("png file converted to image struct.\n");
    return final_image;
}

/// Writes image to simple bmp file (assumes '.bmp' is already in fileaddress)
void write_image_to_file2(image img, char *fileaddress)
{
    int filesize = 54 + sizeof(color) * img.width * img.height;
    int w = img.width;
    int h = img.height;

    unsigned char bmpfileheader[14] = {'B','M', 0,0,0,0, 0,0, 0,0, 54,0,0,0};
    unsigned char bmpinfoheader[40] = {40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0, 24,0};
    unsigned char bmppad[3] = {0,0,0};

    bmpfileheader[ 2] = (unsigned char)(filesize    );
    bmpfileheader[ 3] = (unsigned char)(filesize>> 8);
    bmpfileheader[ 4] = (unsigned char)(filesize>>16);
    bmpfileheader[ 5] = (unsigned char)(filesize>>24);

    bmpinfoheader[ 4] = (unsigned char)(       w    );
    bmpinfoheader[ 5] = (unsigned char)(       w>> 8);
    bmpinfoheader[ 6] = (unsigned char)(       w>>16);
    bmpinfoheader[ 7] = (unsigned char)(       w>>24);
    bmpinfoheader[ 8] = (unsigned char)(       h    );
    bmpinfoheader[ 9] = (unsigned char)(       h>> 8);
    bmpinfoheader[10] = (unsigned char)(       h>>16);
    bmpinfoheader[11] = (unsigned char)(       h>>24);

    if (!img.pixels || !fileaddress)
        return;

    FILE *file = fopen(fileaddress, "wb");

    // Convert the RGB data into BGR- data for bitmap
    unsigned char *data = malloc(4 * img.width * img.height);
    for (int x = 0; x < img.width; ++x)
    {
        for (int y = 0; y < img.height; ++y)
        {
            data[x * 4 + 0 + y * 4 * img.width] = img.pixels[y][x].b;
            data[x * 4 + 1 + y * 4 * img.width] = img.pixels[y][x].g;
            data[x * 4 + 2 + y * 4 * img.width] = img.pixels[y][x].r;
            data[x * 4 + 0 + y * 4 * img.width] = 0;
       }
    }

    // Write the headers
    fwrite(bmpfileheader, 1, 14, file);
    fwrite(bmpinfoheader, 1, 40, file);

    // Write the rows
   for (int i = 0; i < h; ++i)
   {
       fwrite(data+(4*w*i), sizeof(color), img.width, file);
       //fwrite(bmppad, 1, (4 - (w * 3) % 4) % 4, file);
   }

   fclose(file);
}

// /// Writes given node map to file as if it was an image (discards variance) (assumes fileaddress ends with .bmp)
// void write_node_map_to_file(node_map map, char *fileaddress)
// {
//     if (!map.nodes || !fileaddress)
//         return;


//     int filesize = 54 + sizeof(color) * map.width * map.height;
//     int w = map.width;
//     int h = map.height;

//     unsigned char bmpfileheader[14] = {'B','M', 0,0,0,0, 0,0, 0,0, 54,0,0,0};
//     unsigned char bmpinfoheader[40] = {40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0, 24,0};
//     unsigned char bmppad[3] = {0,0,0};

//     bmpfileheader[ 2] = (unsigned char)(filesize    );
//     bmpfileheader[ 3] = (unsigned char)(filesize>> 8);
//     bmpfileheader[ 4] = (unsigned char)(filesize>>16);
//     bmpfileheader[ 5] = (unsigned char)(filesize>>24);

//     bmpinfoheader[ 4] = (unsigned char)(       w    );
//     bmpinfoheader[ 5] = (unsigned char)(       w>> 8);
//     bmpinfoheader[ 6] = (unsigned char)(       w>>16);
//     bmpinfoheader[ 7] = (unsigned char)(       w>>24);
//     bmpinfoheader[ 8] = (unsigned char)(       h    );
//     bmpinfoheader[ 9] = (unsigned char)(       h>> 8);
//     bmpinfoheader[10] = (unsigned char)(       h>>16);
//     bmpinfoheader[11] = (unsigned char)(       h>>24);

//     // Convert the RGB data into BGR- data for bitmap
//     unsigned char *data = malloc(4 * map.width * map.height);
//     for (int x = 0; x < map.width; ++x)
//     {
//         for (int y = 0; y < map.height; ++y)
//         {
//             data[x * 4 + 0 + y * 4 * map.width] = map.nodes[x + y * map.width].color.b;
//             data[x * 4 + 1 + y * 4 * map.width] = map.nodes[x + y * map.width].color.g;
//             data[x * 4 + 2 + y * 4 * map.width] = map.nodes[x + y * map.width].color.r;
//             data[x * 4 + 0 + y * 4 * map.width] = 0;
//         }
//     }

//     FILE *file = fopen(fileaddress, "wb");

//     // Write the headers
//     fwrite(bmpfileheader, 1, 14, file);
//     fwrite(bmpinfoheader, 1, 40, file);

//     // Write the rows
//     for (int i = 0; i < h; ++i)
//     {
//         fwrite(data+(4*w*i), sizeof(color), map.width, file);
//         //fwrite(bmppad, 1, (4 - (w * 3) % 4) % 4, file);
//     }

//     fclose(file);
// }

void generateBitmapImage(unsigned char* image, int height, int width, char* imageFileName);
unsigned char* createBitmapFileHeader(int height, int stride);
unsigned char* createBitmapInfoHeader(int height, int width);

const int BYTES_PER_PIXEL = 3; /// red, green, & blue
const int FILE_HEADER_SIZE = 14;
const int INFO_HEADER_SIZE = 40;

void write_ppm(image img, char *file);

void write_image_to_file(image img, char *fileaddress)
{
    if (!img.pixels || !fileaddress)
        return;

    write_ppm(img, fileaddress);
    return;

    unsigned char *as_bytes = malloc(BYTES_PER_PIXEL * img.height * img.width);

    for (int x = 0; x < img.width; ++x)
    {
        for (int y = 0; y < img.height; ++y)
        {
            as_bytes[x * 3 + 0 + y * BYTES_PER_PIXEL * img.width] = img.pixels[y][x].b;
            as_bytes[x * 3 + 1 + y * BYTES_PER_PIXEL * img.width] = img.pixels[y][x].g;
            as_bytes[x * 3 + 2 + y * BYTES_PER_PIXEL * img.width] = img.pixels[y][x].r;
        }
    }

    generateBitmapImage(as_bytes, img.height, img.width, fileaddress);
}

/// Writes given node map to file as if it was an image (discards variance) (assumes fileaddress ends with .bmp)
void write_node_map_to_file(node_map map, char *fileaddress)
{
    if (!map.nodes || !fileaddress)
        return;

    unsigned char *as_bytes = malloc(BYTES_PER_PIXEL * map.height * map.width);

    for (int x = 0; x < map.width; ++x)
    {
        for (int y = 0; y < map.height; ++y)
        {
            as_bytes[x * 3 + 0 + y * BYTES_PER_PIXEL * map.width] = map.nodes[x + y * map.width].color.b;
            as_bytes[x * 3 + 1 + y * BYTES_PER_PIXEL * map.width] = map.nodes[x + y * map.width].color.g;
            as_bytes[x * 3 + 2 + y * BYTES_PER_PIXEL * map.width] = map.nodes[x + y * map.width].color.r;
        }
    }

    generateBitmapImage(as_bytes, map.height, map.width, fileaddress);
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
  unsigned char *data = malloc(img.height * img.width * 3);
  /* color component is coded from 0 to 255 ;  it is 8 bit color file */
  const int MaxColorComponentValue = 255;
  FILE * fp;
  /* comment should start with # */
  const char *comment = "# this is my new binary pgm file";
 
  /* fill the data array */
  for (y = 0; y < img.height; ++y) {
    for (x = 0; x < img.width; ++x) {
      data[y + x * img.height + 0] = img.pixels[y][x].r;
      data[y + x * img.height + 1] = img.pixels[y][x].g;
      data[y + x * img.height + 2] = img.pixels[y][x].b;
    }
  }
 
  /* write the whole data array to ppm file in one step */
  /* create new file, give it a name and open it in binary mode */
  fp = fopen(file_name, "wb");
  /* write header to the file */
  fprintf(fp, "P6\n%d %d\n 255\n", img.width, img.height);
  /* write image data bytes to the file */
  fwrite(data, 3, img.width * img.height, fp);
  fclose(fp);
}