
# OBJECTIVE
Convert a raster image into a scalable vector graphic and back.

# TASKS

- /*DONE*/ Create Local Structures for common SVG structures (eg. struct Rect {})

- /*DONE*/ install xmake and get the build working

- /*DONE*/ install conan and add jpeg, png libraries

- create an API surface for basic SVG elements

    - \<rect\>

    - \<ellipse\>

    - \<g\>

    - \<line\>

    - \<curve\>\

- Create functions that can convert known SVG elements to a matrix of pixels.  It has to know the colour of an element and plot the colour of each pixel.

- Create a file parser that utilizes those functions in a very specific order. It spits out a 2D array of Pixels.

- Create a bitmap creator that can generate the file header and data segments.

- upload the project to conan

<br>
<br>

- Example flow would be:
    - Convert input file to local structures (.svg)
    - Order Local structures correctly
    - Draw Structures via specific functions onto a Bitmap
    - Write Bitmap into new File (.bmp)