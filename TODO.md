
# OBJECTIVE
Convert a raster image into a scalable vector graphic and back.

# TASKS

- /*DONE*/ Create Local Structures for common SVG structures (eg. struct Rect {})

- /*DONE*/ install xmake and get the build working

- /*DONE*/ install conan and add jpeg, png libraries

- create an API surface for enough basic svg elements that can process a bitmap

- Create functions that can convert a matrix of pixels to an svg file.
    - input decompression using libpng, libjpeg

    - area under the curve algorithm for colouring in

    - iterative collection of non linear shapes, 
    
    - boundary detection along sharp colour differences

    - gradient detection 

- write discord bot adapter in rust, upload to heroku

- upload the project to conan

<br>
<br>

# RESOURCES
