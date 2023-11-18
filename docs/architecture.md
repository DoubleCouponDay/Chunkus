# Chunkus Architecture

## C Process

- Accept 2 arguments to the C Process: chunksize, colours.

    Chunksize is a measure of length of a square proportioning one chunk. Each chunk will have their own threshold intensity value calculated.

    Colours measures the amount of colours that are available for an output image.

- Divide the image into square chunks.

- For each chunk, calculate the mean magnitude of the RGB pixels in the chunk. Each value is a dimension.

- Store each mean in a reduced 2D array of chunks where xy coords can be used.

- Use the mean as the threshold value for that chunk.

- Split vectorizing into 8 directions. Each direction specifies each pixels pair for comparision. Each direction will be processed in a separate thread known as a "Split".

- For each split, perform boundary comparisons between each pixel pair along the corresponding direction line.

- Aggregate all splits together using bitwise OR operations (SIMD where hardware supports it) on 64 bit slices of rows.

- Pathfind the aggregate for boundaries. Do not pathfind a pixel that has already been used.

- Write paths to an svg file.
