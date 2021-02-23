#include "prune.h"

#include "chunkmap.h"
#include "image.h"

bool chunk_is_adjacent(pixelchunk& current, pixelchunk& subject) {
    int abs_x_diff = abs(subject.location.x - current.location.x);
    int abs_y_diff = abs(subject.location.y - current.location.y);
    bool xisclose = abs_x_diff <= 1;
    bool yisclose = abs_y_diff <= 1;
    return xisclose && yisclose;
}

void prune_boundary(std::list<std::shared_ptr<pixelchunk>> boundary) {
    if (boundary.size() < 2)
        return;
        
    std::list<std::shared_ptr<pixelchunk>>::iterator current = boundary.begin();
    std::list<std::shared_ptr<pixelchunk>>::iterator previous = current++;
    while (current != boundary.end())
    {
        if (!chunk_is_adjacent(**current, **previous))
        {
            boundary.erase(current++);
        }
        else
            ++current;
    }
}
