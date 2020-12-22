# Rasterizer
the project objective

convert jpg, png files to svg files!

it has a Rust component and a C component.

# Getting started

You can use this program as a discord bot by clicking the link below. Fill in the last request parameter with your discord guild id.

    https://discord.com/api/oauth2/authorize?client_id=690684027019067393&scope=bot&permissions=51200&guild_id=

You will need to create a file in the `discord-v` folder called `secrettoken.rs`. A template is given in the wiki. 
    
# Building

It should work on windows and linux. For windows, install visual studio community edition and the C++ desktop development workload.

Install Rust lang so that you can use the `cargo` tool to work with the discord-v folder as a rust project.

Install Xmake.io so that you can build the C code in the root directory.

Run the following command to get started:

    xmake f -m -y debug
    
    xmake -b staticvectorizer

accept all prompts whewn downloading packages.

Now cd into the `discord-v` directory and run:

    cargo build

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


    xmake f -m debug
