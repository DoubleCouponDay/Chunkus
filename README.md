# Rasterizer
the project objective

convert jpg, png files to svg files!

it has a Rust component and a C component.

You can use this program as a discord bot by clicking the link below. Fill in the last request parameter with your discord guild id.

    https://discord.com/api/oauth2/authorize?client_id=690684027019067393&scope=bot&permissions=50176&guild_id=
    
# build tool 

Use Xmake as the C compiler. run the following command to get started:

    xmake -y

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
