
# OBJECTIVE
Convert a raster image into a scalable vector graphic and back.

# TASKS

- boundary detection along sharp colour differences

    - iterate through the shapes:
        set the colour of the closed path as a single colour fill

        //add new path to the SVG image      <-\
                                             |
        //convert boundary linked list to this | SVG path

    - wind back all linked lists before freeing
        using wind back functions if duplicates exist

    -- change the first path's coordinate to have real significance

    - connect up the first path's starting coordinate once finished looping

    - collect first path. if there are none connecting, remove it

    -- handle edge case where there is only one point in a shape

    - fill shape attributes

    - store SVG paths to a file

- convert svg to bmp and submit it to discord

- deploy to azure web apps for containers

- upload the project to conan

<br>
<br>

# RESOURCES
