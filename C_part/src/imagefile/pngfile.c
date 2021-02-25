#include <png.h>
#include <stdio.h>
#include <stdlib.h>

#include "pngfile.h"
#include "../image.h"
#include "../utility/error.h"
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
    LOG_INFO("converting png to image struct...");
    LOG_INFO("opening image file...");

    if (fileaddress == NULL) {
        LOG_ERR("fileaddress not given");
        setError(NULL_ARGUMENT_ERROR);
        return (image){NULL, 0, 0};
    }

    /// Open File
    FILE* file_p = fopen(fileaddress, "rb");

    if (!file_p)
    {
        LOG_ERR("Could not open file '%s' for reading", fileaddress);
        setError(ASSUMPTION_WRONG);
        return (image){NULL, 0, 0};
    }

    /// Verify File
    LOG_INFO("Checking if file is PNG type");

    unsigned char header[8];
    fread(header, 1, 8, file_p);
    if (png_sig_cmp(header, 0, 8))
    {
        LOG_ERR("File \'%s\' was not recognised as a PNG file", fileaddress);
        setError(NOT_PNG);
        return (image){NULL, 0, 0};
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
        LOG_ERR("Failed to create png read struct");
        setError(READ_FILE_ERROR);
        return (image){NULL, 0, 0};
    }
    
    LOG_INFO("Creating pnglib info struct...");
    png_infop info = png_create_info_struct(read_struct);

    if (!info)
    {
        LOG_ERR("Error: png_create_info_struct failed");
        setError(READ_FILE_ERROR);
        return (image) { NULL, 0, 0 };
    }

    if (setjmp(png_jmpbuf(read_struct)))
    {
        LOG_ERR("Error during init_io");
        png_destroy_read_struct(read_struct, info, NULL);
        return (image) { NULL, 0, 0 };
    }

    LOG_INFO("Beginning PNG Reading");

    // Start Reading
    png_init_io(read_struct, file_p);
    png_set_sig_bytes(read_struct, 8);

    png_read_info(read_struct, info);

    LOG_INFO("Reading image width/height and allocating image space");
    image output = create_image(png_get_image_width(read_struct, info), png_get_image_height(read_struct, info));

    color_type = png_get_color_type(read_struct, info);
    if (color_type != PNG_COLOR_TYPE_RGB && color_type != PNG_COLOR_TYPE_RGBA)
    {
        LOG_ERR("Only RGB/A PNGs are supported for import, format: %d", color_type);
        setError(NOT_PNG);
        return (image){NULL, 0, 0};
    }
    bit_depth = png_get_bit_depth(read_struct, info);
    if (bit_depth != 8) {
        LOG_ERR("Only 24bpp PNGs are supported, depth: %d", bit_depth * 3);
        setError(NOT_PNG);
        return (image){NULL, 0, 0};
    }

    png_read_update_info(read_struct, info);

    if (setjmp(png_jmpbuf(read_struct)))
    {
        LOG_ERR("Error during early PNG reading");
        setError(READ_FILE_ERROR);
        png_destroy_read_struct(read_struct, info, NULL);
        return (image){NULL, 0, 0};
    }

    LOG_INFO("Allocating row pointers...");

    LOG_INFO("dimensions: %d x %d", output.width, output.height);

    // Allocate row pointers to be filled
    png_bytep* row_pointers_p = calloc(1, sizeof(png_bytep) * output.height);

    for (int y = 0; y < output.height; ++y)
    {
        row_pointers_p[y] = calloc(1, png_get_rowbytes(read_struct, info));
    }

    LOG_INFO("reading the image...");
    
    // Switch to RGB format, and fill the row pointers with values
    image_struct.format = PNG_FORMAT_RGB;
    png_read_image(read_struct, row_pointers_p);

    LOG_INFO("closing image file...");

    // Clean up the file
    fclose(file_p);
    png_destroy_read_struct(&read_struct, &info, NULL);
    
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
                    x, y,
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


void write_image_to_png(image img, char* fileaddress)
{
    if (!img.pixels_array_2d || !fileaddress) {
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

    fclose(fp);

    png_destroy_write_struct(&png_ptr, &info_ptr);
}

void write_bytes_to_png(unsigned char* data, int width, int height, char* fileaddress)
{
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

void iterate_through_shape(pixelchunk_list* list, png_hashies_iter* udata)
{
    pixelchunk_list* current = list;

    while(current != NULL) {
            pixelchunk* chunk = current->chunk_p;
            png_hashies_iter* stuff = udata;
            colourmap* map = stuff->map;
        
        if (chunk->location.x < 0 || chunk->location.y < 0 || chunk->location.x >= map->width || chunk->location.y >= map->height)
        {
            LOG_INFO("Error: chunk has waaaaay off coordinate");
        }

        else {
            colour avg = convert_pixel_to_colour(chunk->average_colour);
            int index = (int)chunk->location.x + map->width * (int)chunk->location.y;
            map->colours[index] = avg;
        }
        current = current->next;
    }    
}

void write_chunkmap_to_png(chunkmap* map, char* fileaddress) {
    if (map->map_width < 1 || map->map_height < 1)
    {
        LOG_ERR("can not write 0 dimension chunkmap to file");
        setError(ASSUMPTION_WRONG);
        return;
    }

    colour* colours = calloc(map->map_width * map->map_height, sizeof(colour));

    colourmap intermediate = {
        colours,
        map->map_width,
        map->map_height
    };

    LOG_INFO("now iterating chunkshapes in chunkmap with dims %d x %d", map->map_width, map->map_height);
    chunkshape* current = map->shape_list;
    int shape_count = 0;

    while (current)
    {
        shape_count++;

        png_hashies_iter stuff = {
            &intermediate
        };
        iterate_through_shape(current->chunks, &stuff);
        current = current->next;
    }
    LOG_INFO("iterated %d shapes in chunkmap", shape_count);
    image output_img = create_image(intermediate.width, intermediate.height);
    
    for (int x = 0; x < intermediate.width; ++x)
    {
        for (int y = 0; y < intermediate.height; ++y)
        {
            colour* bob = &intermediate.colours[x + intermediate.width * y];
            pixel* img_pix = &(output_img.pixels_array_2d[x][y]);
            img_pix->r = bob->r;
            img_pix->g = bob->g;
            img_pix->b = bob->b;
        }
    }
    write_image_to_png(output_img, fileaddress);
}
