
# OBJECTIVE
Convert a raster image into a scalable vector graphic and back.

# TASKS

- fix weird picasso output
    - fix overlap of shapes

    - fix start of path closing to end of path diagonally across the shape

- BUGS
    hangs if no image given to !vectorize command

    bubble_sort caused access violation (but not reproducible)

    restrict chunk_size to 0 and smallest of the two area variables

    restrict threshold to between 0 and 440

- deploy to azure apps for containers

- switch debug() and LOG_INFO() off if release environment variable detected

- fix bobsweep

<br>
<br>

# RESOURCES
