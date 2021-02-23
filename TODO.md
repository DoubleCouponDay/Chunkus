
# OBJECTIVE
Convert a raster image into a scalable vector graphic and back.

# BUGS
    hangs if no image given to !vectorize command

    bubble_sort caused access violation (but not reproducible)

# TASKS

- BEFORE SCHOOL STARTS

    - clear log file after an invocation

    - override panic::set_hook and write error to log file

    - write another bot that posts error message from log
        if success chat message not detected after timeout

    - make other bot restart the bot

    - deploy to azure apps for containers

- select the next sorted border chunk based on which is closed to the outer edge

- fix chunkmap_to_png not drawing all of a chunk's pixels

- fix heap corruption in dcdfiller when threshold: 1
    //doesnt happen when you run the same image, same params without the bot??????????????

- restrict chunk_size to 0 and smallest of the two area variables

- restrict threshold to between 0 and 440

- fix bobsweep

- remove logs from free null checks

<br>
<br>

# RESOURCES
