
# OBJECTIVE
Convert a raster image into a scalable vector graphic and back.

# TASKS

- boundary detection along sharp colour differences
    - iterate through chunkmap

        store the first chunk in a new shape

        iterate through the adjacent chunks:
        
            use rgb as a 3d point and then compare it with adjacent chunks

            if the 3d point is outside a sphere with magnitude of shape_colour_threshold
                current pixelchunk is a boundary

            else
                current pixelchunk is not a boundary

            store the pixelchunk in the adjacent pixelshape's shape with matching colour
                use hashmaps to store pixelchunks inside pixelshapes

                lookup inside each shapes hashmap for the adjacent chunk

                if found, add the current chunk to that shape and finish lookup

            store whether chunk was boundary or not, in a linked list

    - iterate through the shapes:
        set the colour of the closed path as a single colour fill

        add new path to the SVG image      <-\
                                             |
        convert boundary linked list to this | SVG path

    - store SVG paths to a file

- convert svg to bmp and submit it to discord

- deploy to azure web apps for containers

- upload the project to conan

<br>
<br>

# RESOURCES
