#pragma once

#include <vector>

typedef unsigned char byte;

struct pixel
{
    pixel() : R(0), G(0), B(0) {}
    pixel(byte R, byte G, byte B) : R(R), G(G), B(B) {}
    pixel(const pixel& other) : R(other.R), G(other.G), B(other.B) {}

    inline pixel &operator=(const pixel& other) { R = other.R; G = other.G; B = other.B; return *this; }

    byte R, G, B;

    bool is_similar_to(const pixel& other, float threshold);
};

struct pixelInt
{
    pixelInt() : R(0), G(0), B(0) {}
    pixelInt(int r, int g, int b) : R(r), G(g), B(b) {}
    pixelInt(const pixel& other) : R(other.R), G(other.G), B(other.B) {}

    int R, G, B;

    inline pixelInt& operator=(const pixelInt& other) { R = other.R; G = other.G; B = other.B; return *this; }
    inline pixelInt& operator+=(const pixel& p) { R += p.R; G += p.G; B += p.B; return *this; }

    bool is_similar_to(const pixelInt& other, float threshold);
};

struct pixelD
{
    pixelD() : R(0.0), G(0.0), B(0.0) {}
    pixelD(float R, float G, float B) : R(R), G(G), B(B) {}
    pixelD(double R, double G, double B) : R(R), G(G), B(B) {}
    pixelD(const pixelD& p) : R(p.R), G(p.G), B(p.B) {}
    pixelD(const pixel& p) : R((double)p.R / 255.0), G((double)p.G / 255.0), B((double)p.B / 255.0) {}

    double R, G, B;

    inline operator pixel() { return { (byte)(R * 255.0), (byte)(G * 255.0), (byte)(B * 255.0) }; }

    static pixelD lerp(pixelD a, pixelD b, float t);
};

// RGB floating point color struct
// Values to be stored as normalized values (0-1)
struct pixelF
{
    pixelF() : R(0.f), G(0.f), B(0.f) {}
    pixelF(float R, float G, float B) : R(R), G(G), B(B) {}
    pixelF(const pixelF& other) : R(other.R), G(other.G), B(other.G) {}
    pixelF(const pixel& p) : R((float)p.R / 255.f), G((float)p.G / 255.f), B((float)p.B / 255.f) {}
    pixelF(const pixelD& p) : R((float)p.R), G((float)p.G), B((float)p.B) {}

    float R, G, B;

    inline operator pixel() { return { (byte)(R * 255.f), (byte)(G * 255.f), (byte)(B * 255.f) }; }
    inline operator pixelD() { return { R, G, B }; }

    inline pixelF& operator+=(const pixel& p) { R += p.R; G += p.G; B += p.B; return *this; }

    static pixelF lerp(pixelF a, pixelF b, float t);
};

struct image
{
    image() : pixels() {}
    image(size_t width, size_t height) : pixels(width, std::vector<pixel>(height)) {}
    image(image&& other) noexcept : pixels(std::move(other.pixels)) {}

    image(const image& other) = delete;

    inline image& operator=(const image& other) = delete;
    inline image& operator=(image &&other) noexcept { pixels = std::move(other.pixels); other.pixels.clear(); return *this; }

    std::vector<std::vector<pixel>> pixels;

    inline size_t width() const noexcept { return pixels.size(); }
    inline size_t height() const noexcept { return (pixels.empty() ? 0 : pixels.front().size()); }

    const pixel& get(int x, int y) const
    {
        return pixels[x][y];
    }

    pixel& get(int x, int y)
    {
        return pixels[x][y];
    }

    inline void set(int x, int y, pixel pix) { pixels[x][y] = pix; }

    bool to_png(char *file) const;
};
