
# OBJECTIVE
Convert a raster image into a scalable vector graphic and back.

# TASKS

- boundary detection along sharp colour differences
    ok so

    each variance group above a threshold becomes an svg point

    the point has a single colour

    form the first closed path around the border of the image and fill if the points have the same colour. if they dont, fill white

    if a point has the same colour and is adjacent, form a path between the points

    set the colour of the closed path as a single colour fill

- convert back to raster and submit it to discord

- deploy to azure web apps for containers

- upload the project to conan

<br>
<br>

# RESOURCES
