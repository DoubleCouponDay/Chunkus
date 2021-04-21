#include "pngfile.h"

#include <png.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <stdexcept>
#include <memory>

#include "../image.h"
#include "utility/error.h"
#include "utility/logger.h"

/// Takes a filename (assumed to be a png file), and creates an image struct full of the png's pixels
/// 
/// Steps involve:
/// Open the file for reading
/// Create necessary libpng structs and populate them
/// Allocate memory to hold the image contents
/// Read the image into the memory
/// Convert the obtained memory into a contiguous format (convert the row pointers from a 2 dimensional array into a single array)
image convert_png_to_image(char* fileaddress) {
    LOG_INFO("Converting '%s' to png file...", fileaddress);

    if (fileaddress == NULL) {
        LOG_ERR("Fileaddress was null");
        setError(NULL_ARGUMENT_ERROR);
        return image();
    }

    FILE* wtf = fopen(fileaddress, "rb");

    /// Open File
    FILE* file_p = nullptr;
    if (fopen_s(&file_p, fileaddress, "rb"))
        throw std::invalid_argument((std::string("Could not open file: '") + fileaddress + "'").c_str());

    /// Verify File
    LOG_INFO("Checking if file is PNG type");

    unsigned char header[8];
    fread(header, 1, 8, file_p);
    if (png_sig_cmp(header, 0, 8))
    {
        LOG_INFO("File \'%s\' was not recognised as a PNG file", fileaddress);
        setError(NOT_PNG);
        return image();
    }
    
    /// Prepare and read structs
    LOG_INFO("Creating png_image struct");

    png_byte color_type, bit_depth;

    // Return struct
    png_image image_struct;
    image_struct.opaque = NULL; 
    image_struct.version = PNG_IMAGE_VERSION;

    LOG_INFO("creating pnglib read struct...");

    png_structp read_struct = png_create_read_struct(
        PNG_LIBPNG_VER_STRING, NULL, NULL, NULL
    );

    if (!read_struct)
    {
        LOG_INFO("Failed to create png read struct");
        setError(READ_FILE_ERROR);
        return image();
    }
    
    LOG_INFO("Creating pnglib info struct...");
    png_infop info = png_create_info_struct(read_struct);

    if (!info)
    {
        LOG_INFO("Error: png_create_info_struct failed");
        setError(READ_FILE_ERROR);
        return image();
    }

    if (setjmp(png_jmpbuf(read_struct)))
    {
        LOG_INFO("Error during init_io");
        png_destroy_read_struct(&read_struct, &info, NULL);
        return image();
    }

    LOG_INFO("Beginning PNG Reading ");

    // Start Reading
    png_init_io(read_struct, file_p);
    png_set_sig_bytes(read_struct, 8);

    png_read_info(read_struct, info);

    LOG_INFO("Reading image width/height and allocating image space");
    image output{png_get_image_width(read_struct, info), png_get_image_height(read_struct, info)};

    color_type = png_get_color_type(read_struct, info);
    if (color_type != PNG_COLOR_TYPE_RGB && color_type != PNG_COLOR_TYPE_RGBA)
    {
        LOG_INFO("Only RGB/A PNGs are supported for import, format: %d", color_type);
        setError(NOT_PNG);
        return image();
    }
    bit_depth = png_get_bit_depth(read_struct, info);
    if (bit_depth != 8) {
        LOG_INFO("Only 24bpp PNGs are supported, depth: %d ", bit_depth * 3);
        setError(NOT_PNG);
        return image();
    }

    png_read_update_info(read_struct, info);

    if (setjmp(png_jmpbuf(read_struct)))
    {
        LOG_INFO("Error during early PNG reading ");
        setError(READ_FILE_ERROR);
        png_destroy_read_struct(&read_struct, &info, NULL);
        return image();
    }

    LOG_INFO("Allocating row pointers...");

    LOG_INFO("dimensions: %d x %d ", output.width(), output.height());

    // Allocate row pointers to be filled
    std::vector<std::vector<png_byte>> data(output.height(), std::vector<png_byte>(png_get_rowbytes(read_struct, info)));

    LOG_INFO("reading the image...");
    
    // Switch to RGB format, and fill the row pointers with values
    {
        std::vector<png_bytep> unsafe_data(output.height());
        for (int i = 0; i < unsafe_data.size(); ++i)
            unsafe_data[i] = data[i].data();
        image_struct.format = PNG_FORMAT_RGB;
        png_read_image(read_struct, unsafe_data.data());
    }

    LOG_INFO("closing image file...");

    // Clean up the file
    fclose(file_p);
    png_destroy_read_struct(&read_struct, &info, NULL);
    
    LOG_INFO("putting dereferenced row pointers in custom struct...");

    if (color_type == PNG_COLOR_TYPE_RGB)
    {
        LOG_INFO("Type is RGB");
        for (size_t y = 0; y < output.height(); ++y)
        {
            for (size_t x = 0; x < output.width(); ++x)
            {
                output.set(x, y, pixel{data[y][x * 3 + 0], data[y][x * 3 + 1], data[y][x * 3 + 2] });
            }
        }
    }
    else if (color_type == PNG_COLOR_TYPE_RGBA)
    {
        LOG_INFO("Type is RGBA");
        for (size_t y = 0; y < output.height(); ++y)
        {
            for (size_t x = 0; x < output.width(); ++x)
            {
                output.set(x, y, pixel{ data[y][x * 4 + 0], data[y][x * 4 + 1], data[y][x * 4 + 2] });
            }
        }
    }

    else
    {
        throw std::invalid_argument("png file was not a supported color type");
    }
    
    LOG_INFO("png file converted to image struct.");
    return output;
}


void write_image_to_png(const image& img, char* fileaddress)
{
    if (img.pixels.empty() || !fileaddress) {
        LOG_ERR("null arguments given to write_image_to_png");
        setError(NULL_ARGUMENT_ERROR);
        return;
    }

    FILE* fp = fopen(fileaddress, "wb");

    if (!fp)
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

    png_init_io(png_ptr, fp);

    // Output 8bit RGBA
    png_set_IHDR(
        png_ptr,
        info_ptr,
        img.width(), 
        img.height(),
        8,
        PNG_COLOR_TYPE_RGB,
        PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_DEFAULT,
        PNG_FILTER_TYPE_DEFAULT
    );

    png_write_info(png_ptr, info_ptr);

    // Popular some row_pointers
    png_bytep* row_pointers = new png_bytep[img.height()];

    for (int y = 0; y < img.height(); ++y)
    {
        row_pointers[y] = new png_byte[img.width() * 3];
        for (int x = 0; x < img.width(); ++x)
        {
            row_pointers[y][x * 3 + 0] = img.get(x, y).R;
            row_pointers[y][x * 3 + 1] = img.get(x, y).G;
            row_pointers[y][x * 3 + 2] = img.get(x, y).B;
        }
    }

    png_write_image(png_ptr, row_pointers);
    png_write_end(png_ptr, NULL);

    // Clean up
    for (int y = 0; y < img.height(); ++y)
    {
        delete [] row_pointers[y];
    }
    delete [] row_pointers;

    fclose(fp);

    png_destroy_write_struct(&png_ptr, &info_ptr);
}

void write_bytes_to_png(unsigned char* data, int width, int height, char* fileaddress)
{
    FILE* fp = fopen(fileaddress, "wb");

    if (!fp)
    {
        LOG_INFO("File: %s not found", fileaddress);
        setError(READ_FILE_ERROR);
        return;
    }

    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr)
    {
        LOG_INFO("Couldn't create png_struct for writing");
        setError(READ_FILE_ERROR);
        return;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
        LOG_INFO("Couldn't create png_info struct for writing");
        setError(READ_FILE_ERROR);
        return;
    }

    if (setjmp(png_jmpbuf(png_ptr)))
    {
        LOG_INFO("Unknown Failure during png writing");
        setError(READ_FILE_ERROR);
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
    png_bytep *row_pointers = new png_bytep[height];
    for (int y = 0; y < height; ++y)
    {
        row_pointers[y] = new png_byte[width * 3];
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
        delete[] row_pointers[y];
    }
    delete[] row_pointers;

    fclose(fp);

    png_destroy_write_struct(&png_ptr, &info_ptr);
}

void iterate_through_shape(const std::list<std::shared_ptr<pixelchunk>>& chunks, image& img)
{
    for (auto& chunk : chunks)
    {
        img.set(chunk->location.x, chunk->location.y, chunk->average_colour);
    }
}

void write_chunkmap_to_png(const chunkmap& map, char* fileaddress) {
    LOG_INFO("Writing chunkmap to '%s'", fileaddress);
    if (map.width() < 1 || map.height() < 1)
    {
        LOG_ERR("Can not write 0 dimension chunkmap to file");
        setError(ASSUMPTION_WRONG);
        return;
    }

    image img = image{map.width(), map.height()};

    for (auto& shape : map.shape_list)
    {
        iterate_through_shape(shape->chunks, img);
    }

    write_image_to_png(img, fileaddress);
}
