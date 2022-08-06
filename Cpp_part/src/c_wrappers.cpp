#include "c_wrappers.h"

algorithm_data get_algorithm_data()
{
    algorithm_data data;
    data.width = 100;
    data.height = 100;
    pixel_group group;
    group.color = Colors::Green32;
    group.lower = { 0, 0 };
    group.upper = { 50, 50 };
    group.pixels.push_back({ 25, 25 });
    group.pixels.push_back({ 25, 26 });
    group.pixels.push_back({ 26, 25 });
    group.pixels.push_back({ 26, 26 });
    group.pixels.push_back({ 27, 25 });
    group.pixels.push_back({ 27, 26 });
    data.groups.push_back(group);
    group.color = Colors::Red32;
    group.lower = { 50, 50 };
    group.upper = { 100, 100 };
    group.pixels.clear();
    group.pixels.push_back({ 75, 75 });
    group.pixels.push_back({ 75, 76 });
    group.pixels.push_back({ 76, 75 });
    group.pixels.push_back({ 76, 76 });
    group.pixels.push_back({ 77, 75 });
    group.pixels.push_back({ 77, 76 });
    data.groups.push_back(group);
    return data;
}

visual_algorithm_data generate_visuals(algorithm_data data)
{
    visual_algorithm_data visual_data;
    visual_data.data = std::move(data);
    visual_data.complete_texture = WomboTexture(Texture8{ Colors::Black8, visual_data.data.width, visual_data.data.height });

    for (int i = 0; i < visual_data.data.groups.size(); i++)
    {
        auto& group = visual_data.data.groups[i];
        auto& texture = visual_data.textures.emplace_back();
        auto color8 = Convert32to8(group.color);
        texture.texture = WomboTexture{ Texture8{ Colors::Black8, group.width(), group.height() }, true };
        texture.position = { group.lower.x, group.lower.y };

        for (int j = 0; j < group.pixels.size(); j++)
        {
            auto& pixel = group.pixels[j];
            texture.texture.setPixel(pixel.x - texture.position.x, pixel.y - texture.position.y, color8);
            visual_data.complete_texture.setPixel(pixel.x, pixel.y, color8);
        }
    }

    return visual_data;
}