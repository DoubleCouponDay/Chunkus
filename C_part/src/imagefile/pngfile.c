#include <png.h>
#include <stdio.h>
#include <stdlib.h>

#include "pngfile.h"
#include "../image.h"
#include "../utility/error.h"
#include "../../test/debug.h"

/// Takes a filename (assumed to be a png file), and creates an image struct full of the png's pixels
/// 
/// Steps involve:
/// Open the file for reading
/// Create necessary libpng structs and populate them
/// Allocate memory to hold the image contents
/// Read the image into the memory
/// Convert the obtained memory into a contiguous format (convert the row pointers from a 2 dimensional array into a single array)
image convert_png_to_image(char* fileaddress) {
    DEBUG("converting png to image struct...\n");
    DEBUG("opening image file...\n");

    if (fileaddress == NULL) {
        DEBUG("fileaddress not given\n");
        setError(NULL_ARGUMENT_ERROR);
        return (image){NULL, 0, 0};
    }

    /// Open File
    FILE* file_p = fopen(fileaddress, "rb");

    if (!file_p)
    {
        DEBUG("Could not open file '%s' for reading\n", fileaddress);
        setError(ASSUMPTION_WRONG);
        return (image){NULL, 0, 0};
    }

    /// Verify File
    DEBUG("Checking if file is PNG type\n");

    unsigned char header[8];
    fread(header, 1, 8, file_p);
    if (png_sig_cmp(header, 0, 8))
    {
        DEBUG("File \'%s\' was not recognised as a PNG file\n", fileaddress);
        setError(NOT_PNG);
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
        setError(READ_FILE_ERROR);
        return (image){NULL, 0, 0};
    }
    
    DEBUG("Creating pnglib info struct...\n");
    png_infop info = png_create_info_struct(read_struct);

    if (!info)
    {
        DEBUG("Error: png_create_info_struct failed\n");
        setError(READ_FILE_ERROR);
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
        setError(NOT_PNG);
        return (image){NULL, 0, 0};
    }
    bit_depth = png_get_bit_depth(read_struct, info);
    if (bit_depth != 8) {
        DEBUG("Only 24bpp PNGs are supported, depth: %d \n", bit_depth * 3);
        setError(NOT_PNG);
        return (image){NULL, 0, 0};
    }

    png_read_update_info(read_struct, info);

    if (setjmp(png_jmpbuf(read_struct)))
    {
        DEBUG("Error during early PNG reading \n");
        setError(READ_FILE_ERROR);
        png_destroy_read_struct(read_struct, info, NULL);
        return (image){NULL, 0, 0};
    }

    DEBUG("Allocating row pointers...\n");

    DEBUG("dimensions: %d x %d \n", output.width, output.height);

    // Allocate row pointers to be filled
    png_bytep* row_pointers_p = calloc(1, sizeof(png_bytep) * output.height);

    for (int y = 0; y < output.height; ++y)
    {
        row_pointers_p[y] = calloc(1, png_get_rowbytes(read_struct, info));
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
                    x, y,
                    1, 1
                };
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
                    x, y,
                    1, 1
                };
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


void write_image_to_png(image img, char* fileaddress)
{
    if (!img.pixels_array_2d || !fileaddress) {
        DEBUG("null arguments given to write_image_to_png\n");
        setError(NULL_ARGUMENT_ERROR);
        return;
    }

    FILE* fp = fopen(fileaddress, "wb");

    if (!fp)
    {
        DEBUG("File: %s not found\n", fileaddress);
        setError(READ_FILE_ERROR);
        return;
    }

    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr)
    {
        DEBUG("Couldn't create png_struct for writing\n");
        setError(READ_FILE_ERROR);
        return;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
        DEBUG("Couldn't create png_info struct for writing\n");
        setError(READ_FILE_ERROR);
        return;
    }

    if (setjmp(png_jmpbuf(png_ptr)))
    {
        DEBUG("Unknown Failure during png writing\n");
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
        DEBUG("File: %s not found\n", fileaddress);
        setError(READ_FILE_ERROR);
        return;
    }

    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr)
    {
        DEBUG("Couldn't create png_struct for writing\n");
        setError(READ_FILE_ERROR);
        return;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
        DEBUG("Couldn't create png_info struct for writing\n");
        setError(READ_FILE_ERROR);
        return;
    }

    if (setjmp(png_jmpbuf(png_ptr)))
    {
        DEBUG("Unknown Failure during png writing\n");
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
            pixelchunk* chunk = list->chunk_p;    
            png_hashies_iter* stuff = udata;
            colourmap* map = stuff->map;
        
        if (chunk->location.x < 0 || chunk->location.y < 0 || chunk->location.x >= map->width || chunk->location.y >= map->height)
        {
            DEBUG("Error: chunk has waaaaay off coordinate\n");
        }

        else {
            colour avg = convert_pixel_to_colour(chunk->average_colour);
            map->colours[chunk->location.x + map->width * chunk->location.y] = avg;
        }
        current = current->next;
    }    
}

void write_chunkmap_to_png(chunkmap* map, char* fileaddress) {
    if (map->map_width < 1 || map->map_height < 1)
    {
        DEBUG("can not write 0 dimension chunkmap to file\n");
        setError(ASSUMPTION_WRONG);
        return;
    }

    colour* colours = calloc(map->map_width * map->map_height, sizeof(colour));

    colourmap intermediate = {
        colours,
        map->map_width,
        map->map_height
    };

    DEBUG("now iterating chunkshapes in chunkmap with dims %d x %d\n", map->map_width, map->map_height);
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
    DEBUG("iterated %d shapes in chunkmap\n", shape_count);
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
