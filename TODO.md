
# OBJECTIVE
Convert a raster image into a scalable vector graphic and back.

# BUGS
    hangs if no image given to !vectorize command

    bubble_sort caused access violation (but not reproducible)

# TASKS

- select the next sorted border chunk based on which is closed to the outer edge

- fix chunkmap_to_png not drawing all of a chunk's pixels

- fix bottom edges not being added to boundary

- fix heap corruption in dcdfiller when threshold: 1
    //doesnt happen when you run the same image, same params without the bot??????????????

- restrict chunk_size to 0 and smallest of the two area variables

- restrict threshold to between 0 and 440

- deploy to azure apps for containers

- switch debug() and LOG_INFO() off if release environment variable detected

- fix bobsweep

<br>
<br>

# RESOURCES
