
# OBJECTIVE
Convert a raster image into a scalable vector graphic and back.

# TASKS

- boundary detection along sharp colour differences

    bin variance

    install hashmap library

    iterate through chunkmap

    store the first chunk in a new shape

    use rgb as a 3d point and then compare it with adjacent chunks

    if the 3d point is outside a sphere with magnitude of shape_colour_threshhold
        current pixelchunk is a boundary

    else
        current pixelchunk is not a boundary

        store the pixelchunk in the pixelshape with matching colour
            use hashmaps to store pixelchunks inside pixelshapes

            lookup inside each shapes map for the adjacent chunk

            if found, add the current chunkto that shape and stop looping

    set the colour of the closed path as a single colour fill

- convert svg to bmp and submit it to discord

- deploy to azure web apps for containers

- upload the project to conan

<br>
<br>

# RESOURCES
