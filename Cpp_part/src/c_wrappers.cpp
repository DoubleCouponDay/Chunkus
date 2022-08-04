#include "c_wrappers.h"

algorithm_data get_algorithm_data()
{
    algorithm_data data;
    data.width = 100;
    data.height = 100;
    return data;
}

visual_algorithm_data generate_visuals(algorithm_data data)
{
    visual_algorithm_data visual_data;
    visual_data.data = std::move(data);

    for (int i = 0; i < visual_data.data.groups.size(); i++)
    {
        auto& group = visual_data.data.groups[i];
        auto& texture = visual_data.textures.emplace_back();
        texture.texture = WomboTexture{ Texture8{ Colors::Black8, group.width(), group.height() } };
        texture.position = { group.lower.x, group.lower.y };

        for (int j = 0; j < group.pixels.size(); j++)
        {
            auto& pixel = group.pixels[j];
            texture.texture.setPixel(pixel.x, pixel.y, Convert32to8(group.color));
        }
    }
}