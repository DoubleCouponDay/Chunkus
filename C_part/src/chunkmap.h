#pragma once

#include <vector>
#include <list>
#include <memory>

#include "image.h"


struct chunkshape; //allows circular reference

struct coordinate
{
    int x;
    int y;
};

struct pixelchunk
{
    pixelchunk() = default;
    pixelchunk(coordinate coord) : location(coord) {}
    pixelchunk(pixel average_colour, coordinate location) : average_colour(average_colour), location(location) {}


    pixel average_colour;
    coordinate location;
    std::shared_ptr<chunkshape> shape_in;
    std::shared_ptr<chunkshape> boundary_in;
};

struct chunkshape {
    std::list<std::shared_ptr<pixelchunk>> chunks;
    std::list<std::shared_ptr<pixelchunk>> boundaries;
    pixel colour;
};

struct chunkmap
{
    chunkmap();
    chunkmap(const image& image, int chunk_size);
    chunkmap(chunkmap &&other) noexcept : groups(std::move(other.groups)), shape_list(std::move(other.shape_list)) { other.groups.clear(); other.shape_list.clear(); }

    chunkmap(const chunkmap& other) = delete;

    inline chunkmap& operator=(const chunkmap& other) = delete;
    chunkmap& operator=(chunkmap &&other);

    std::vector<std::vector<std::shared_ptr<pixelchunk>>> groups;
    std::list<std::shared_ptr<chunkshape>> shape_list;

    inline size_t width() const { return groups.size(); }
    inline size_t height() const { return (groups.empty() ? 0 : groups.front().size()); }

    const std::shared_ptr<pixelchunk>& get(int x, int y) const { return groups[x][y]; }
    std::shared_ptr<pixelchunk>& get(int x, int y) { return groups[x][y]; }

    const std::shared_ptr<pixelchunk>& get1D(int i) const { return groups[i % width()][i / width()]; }
    std::shared_ptr<pixelchunk>& get1D(int i) { return groups[i % width()][i / width()]; }

    bool chunks_to_file(char* file) const;
    bool shapes_to_file(char* file) const;
    bool borders_to_file(char* file) const;

private:
    void iterate_pixels(const image& other, size_t chunk_size);
};

typedef struct
{
    char* file_path;
    int chunk_size;
    float shape_colour_threshhold;
    int num_colours;
} vectorize_options;
