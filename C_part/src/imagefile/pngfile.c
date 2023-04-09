#include <png.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "pngfile.h"
#include "../image.h"
#include "../utility/error.h"
#include "../utility/logger.h"

bool file_is_png(const char* fileaddress) {
    unsigned char header[8];
    FILE* file_p = openfile(fileaddress);

    if(isBadError()) {
        LOG_ERR("openfile failed");
        return false;
    }
    size_t ignored = fread(header, 1, 8, file_p);
    fclose(file_p);
    return (bool)png_sig_cmp(header, 0, 8) == 0;
}

/// Takes a filename and creates an image struct full of the png's pixels
/// 
/// Steps involve:
/// Open the file for reading
/// Create necessary libpng structs and populate them
/// Allocate memory to hold the image contents
/// Read the image into the memory
/// Convert the obtained memory into a contiguous format (convert the row pointers from a 2 dimensional array into a single array)
image convert_png_to_image(const char* fileaddress) {
    LOG_INFO("opening png file: ", fileaddress);

    FILE* file_p = openfile(fileaddress);

    if(isBadError()) {
		LOG_ERR("openfile failed");
		return EMPTY_IMAGE;
	}

    unsigned char header[8];
    if (fread(header, 1, 8, file_p) != 8)
    {
        LOG_ERR("Failed to read png header");
        setError(READ_FILE_ERROR);
        return EMPTY_IMAGE;
    }

    if (png_sig_cmp(header, 0, 8))
    {
        LOG_ERR("Not a png file");
        setError(NOT_PNG_OR_JPEG);
        return EMPTY_IMAGE;
    }
    
    /// Prepare and read structs
    LOG_INFO("Creating png_image struct");

    png_byte color_type, bit_depth;

    png_image image_struct;
    image_struct.opaque = NULL; 
    image_struct.version = PNG_IMAGE_VERSION;

    LOG_INFO("creating pnglib read struct...");

    png_structp read_struct = png_create_read_struct(
        PNG_LIBPNG_VER_STRING, NULL, NULL, NULL
    );

    if (!read_struct)
    {
        LOG_ERR("Failed to create png read struct");
        setError(READ_FILE_ERROR);
        fclose(file_p);
        return EMPTY_IMAGE;
    }
    
    LOG_INFO("Creating pnglib info struct...");
    png_infop info_p = png_create_info_struct(read_struct);

    if (!info_p)
    {
        LOG_ERR("Error: png_create_info_struct failed");
        setError(READ_FILE_ERROR);
        fclose(file_p);
        return EMPTY_IMAGE;
    }

    if (setjmp(png_jmpbuf(read_struct)))
    {
        LOG_ERR("Error during init_io");
        png_destroy_read_struct(&read_struct, &info_p, NULL);
        fclose(file_p);
        return (image){ 0, 0, NULL };
    }

    LOG_INFO("Beginning PNG Reading");

    // Start Reading
    png_init_io(read_struct, file_p);
    png_set_sig_bytes(read_struct, 8);

    png_read_info(read_struct, info_p);

    LOG_INFO("Reading image width/height and allocating image space");
    int imageWidth = png_get_image_width(read_struct, info_p);
    int imageHeight = png_get_image_height(read_struct, info_p);
    image output = create_image(imageWidth, imageHeight);

    color_type = png_get_color_type(read_struct, info_p);
    if (color_type != PNG_COLOR_TYPE_RGB && color_type != PNG_COLOR_TYPE_RGBA)
    {
        LOG_ERR("Only RGB/A PNGs are supported for import, format: %d", color_type);
        setError(BAD_ARGUMENT_ERROR);
        fclose(file_p);
        return EMPTY_IMAGE;
    }
    bit_depth = png_get_bit_depth(read_struct, info_p);
    if (bit_depth != 8) {
        LOG_ERR("Only 24bit PNGs are supported, depth: %d", bit_depth * 3);
        setError(BAD_ARGUMENT_ERROR);
        fclose(file_p);
        return EMPTY_IMAGE;
    }

    png_read_update_info(read_struct, info_p);

    if (setjmp(png_jmpbuf(read_struct))) {
        LOG_ERR("Error during early PNG reading");
        setError(READ_FILE_ERROR);
        png_destroy_read_struct(&read_struct, &info_p, NULL);
        fclose(file_p);
        return EMPTY_IMAGE;
    }

    LOG_INFO("Allocating row pointers...");

    LOG_INFO("dimensions: %d x %d", output.width, output.height);

    // Allocate row pointers to be filled
    png_bytep* row_pointers_p = calloc(1, sizeof(png_bytep) * output.height);

    for (int y = 0; y < output.height; ++y)
    {
        row_pointers_p[y] = calloc(1, png_get_rowbytes(read_struct, info_p));
    }

    LOG_INFO("reading the image...");
    
    // Switch to RGB format, and fill the row pointers with values
    image_struct.format = PNG_FORMAT_RGB;
    png_read_image(read_struct, row_pointers_p);

    LOG_INFO("closing image file...");

    // Clean up the file
    fclose(file_p);
    png_destroy_read_struct(&read_struct, &info_p, NULL);
    
    LOG_INFO("putting dereferenced row pointers in custom struct...");

    if (color_type == PNG_COLOR_TYPE_RGB)
    {
        LOG_INFO("Type is RGB");

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
                    x, y
                };
            }
        }
    }

    else if (color_type == PNG_COLOR_TYPE_RGBA)
    {
        LOG_INFO("Type is RGBA");

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
                    x, y,
                    1, 1
                };
            }
        }
    }

    else
    {
        LOG_INFO("color type is not RGBA");
    }

    for (int i = 0; i < output.height; ++i)
        free(row_pointers_p[i]);
        
    free(row_pointers_p);
    
    LOG_INFO("png file converted to image struct.");
    return output;
}


void write_image_to_png(image img, const char* fileaddress)
{
    if (!img.pixels_array_2d || !fileaddress) {
        LOG_ERR("null arguments given to write_image_to_png");
        setError(NULL_ARGUMENT_ERROR);
        return;
    }

    FILE* file_p = fopen(fileaddress, "wb");

    if (!file_p)
    {
        LOG_ERR("File: %s not found", fileaddress);
        setError(READ_FILE_ERROR);
        return;
    }

    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr)
    {
        LOG_ERR("Couldn't create png_struct for writing");
        setError(READ_FILE_ERROR);
        return;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
        LOG_ERR("Couldn't create png_info struct for writing");
        setError(READ_FILE_ERROR);
        return;
    }

    if (setjmp(png_jmpbuf(png_ptr)))
    {
        LOG_ERR("Unknown Failure during png writing");
        setError(READ_FILE_ERROR);
        png_destroy_write_struct(&png_ptr, &info_ptr);
        return;
    }

    png_init_io(png_ptr, file_p);

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
    png_bytep* row_pointers = calloc(img.height, sizeof(png_bytep));

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

    fclose(file_p);

    png_destroy_write_struct(&png_ptr, &info_ptr);
}

void write_bytes_to_png(unsigned char* data, int width, int height, const char* fileaddress)
{
    FILE* file_p = fopen(fileaddress, "wb");

    if (file_p == NULL)
    {
        LOG_ERR("File: %s not found", fileaddress);
        setError(READ_FILE_ERROR);
        return;
    }

    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr)
    {
        LOG_ERR("Couldn't create png_struct for writing");
        setError(READ_FILE_ERROR);
        return;
    }
    png_infop info_ptr = png_create_info_struct(png_ptr);
    
    if (!info_ptr)
    {
        LOG_ERR("Couldn't create png_info struct for writing");
        setError(READ_FILE_ERROR);
        return;
    }

    if (setjmp(png_jmpbuf(png_ptr)))
    {
        LOG_ERR("Unknown Failure during png writing");
        setError(READ_FILE_ERROR);
        png_destroy_write_struct(&png_ptr, &info_ptr);
        return;
    }

    png_init_io(png_ptr, file_p);

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

    fclose(file_p);

    png_destroy_write_struct(&png_ptr, &info_ptr);
}
