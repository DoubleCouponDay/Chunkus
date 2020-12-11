# Rasterizer
the project objective

convert jpg, png files to svg files!

it has a Rust component and a C component.

# Getting started

You can use this program as a discord bot by clicking the link below. Fill in the last request parameter with your discord guild id.

    https://discord.com/api/oauth2/authorize?client_id=690684027019067393&scope=bot&permissions=51200&guild_id=

You will need to create a file in the `discord-v` folder called `secrettoken.rs`. A template is given in the wiki. 
    
# build tool 

Use Xmake to build the rust part. run the following command to get started:

    xmake -y

Then to build the rust part, the vectorizer_library.a/.lib file built from xmake must be moved to a folder discord-v/vec-lib/

# package manager

conan.io

# libraries used

libjpg, libpng, nanosvg

# test runner

run the xmake tool to test the C component.

    xmake -b tests
    xmake r tests

# Building 
it should work on windows and linux

install rust and xmake

    xmake f -m debug
