
# OBJECTIVE
Convert a raster image into a scalable vector graphic and back.

# TASKS

- /*DONE*/ Create Local Structures for common SVG structures (eg. struct Rect {})

- /*DONE*/ install xmake and get the build working

- /*DONE*/ install conan and add jpeg, png libraries

- create an API surface for basic SVG elements

    - \<ellipse\>

    - \<curve\>\

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

- https://en.wikipedia.org/wiki/Raster_graphics

- https://en.wikipedia.org/wiki/Image_tracing

- https://en.wikipedia.org/wiki/Portable_Network_Graphics

- https://en.wikipedia.org/wiki/JPEG

- http://www.libpng.org/pub/png/libpng-manual.txt

- https://en.wikipedia.org/wiki/Image_tracing

- https://www.w3.org/TR/SVG2/shapes.html

- https://code.visualstudio.com/docs/cpp/c-cpp-properties-schema-reference