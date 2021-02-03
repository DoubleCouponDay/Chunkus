#include "../mapping.h"

uint64_t chunk_hash(const void *chunk, uint64_t seed0, uint64_t seed1) {
    pixelchunk* item = chunk;
    return hashmap_sip(item, sizeof(item), seed0, seed1);
}

int chunk_compare(const void *chunk_a, const void *chunk_b, void *udata) {
    pixelchunk* ca = chunk_a;
    pixelchunk* cb = chunk_b;

    if(ca->location.x == cb->location.x &&
        ca->location.y == cb->location.y) {
        return 0;
    }
    
    else if(ca->location.x < cb->location.x ||
        ca->location.y < cb->location.y) {
        return 1;
    }

    else {
        return -1;
    }
}
