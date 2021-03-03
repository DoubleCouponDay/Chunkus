# Rasterizer
the project objective

convert jpg, png files to svg files!

Currently, only png is supported.

it has a Rust component and a C component.

# Getting started

You will need to create a file in the `discord-v` folder called `secrettoken.rs`. A template is given in the wiki. 
<br>

# Building
  
## Requirements

It should work on windows and linux. For windows, install visual studio community edition and the C++ desktop development workload.  

Install Rust lang so that you can use the `cargo` tool to work with the discord-v folder as a rust project.  

Install Xmake.io so that you can build the C code in the root directory.  

Install Conan.io. it can only be run from cmd, not powershell. Windows defender antivirus may nuke your conan.exe so you will have to add the folder in the exclusions.
    create conanfile.txt
    
    add [requires] section
        library names followed by /version
    
    add [generators] section
        cmake
        
    cd into build directory
    
    conan install ..
  
---

## Building the C code
<br>

The C building uses a lua based build tool called Xmake, https://xmake.io/  
First configure the xmake build tool with:  
    
    xmake f -m debug -y

This gets xmake to find the correct compiler and linker, and allows conan to download the dependencies  
It also configures xmake for debug building
You can configure it to produce release builds with:

    xmake f -m release

You must add an environment variable called `conanpath` and make its value the absolute path to your `.conan` folder. Usually this is found in your user folder.

Now you should be able to run xmake and successfully build the C binaries  
Simply call:
    
    xmake


Xmake will build 2 executables, and a static library.
They can most likely be found in xmake's default debug build folder
- Xmake's default build folder on Windows: `Vectorizer/build/x64/windows/debug/`
- Xmake's default build folder on Linux: `Vectorizer/build/x86_64/linux/debug/`  

The static library (either **staticvectorizer.lib** or **libstaticvectorizer.a**) is required for the rust component, however if the static library does in fact lie in the default build directory, the rust build script should copy it automatically.

The C code builds to `/build/windows/x64/`.
  
---
  
## Building the Rust code

Now to build the Rust Component  

The rust component links to the C code, which in turn depends on libpng (which depends on zlib)

Now in the `discord-v` folder, run:

    cargo build

To link against the release build define the environment variable `releasebuild` as true

in windows:

    $env:releasebuild = "true" //powershell

    set releasebuild=true //cmd

    then log back in

in linux:

    you need to update your `~/.pam_environment` file to have this persist

This sets the environment variable for this single terminal instance (the variable is lost with the terminal)

It should succeed
If it doesn't, you or I have done something wrong


The rust part builds to `/discord-v/target/debug/`.

# Running

Commands to use the bot:
### Vectorize: Goes through all attachments of the command message, executes the algorithm on them and returns the output  
`!v/!vectorize` with an attachment or url eg.  

    !v https://cdn.discordapp.com/attachments/787470274261549056/807847299752394773/ginormous.png  

You should receive a message with `output.svg` and a preview png attached
  
### Params: Sets the parameters to use with the algorithm, first item is chunk_size, second item is threshold  
Chunk Size is a reverse scale for the image, higher number improve speed while reducing quality (and losing information)  
Threshold is a number between 0 and 441.67 (The square root of 255^2 * 3 (vector math))  
- The threshold determines how easily another colour is considered a separate shape  
- A Threshold of 0 means any color that is not EXACTLY the same will be considered separate  
- A threshold of 441.67 means the only color values considered different are rgb(0,0,0) and rgb(255,255,255) (white and black)  

`!p/!params [chunksize] [threshold]` eg. 

    !params 2 50  
You should receive a confirmation message telling you what you set the parameters to  
  
### Set Algorithm: Sets which algorithm is used for shape identification  
Currently only values of 0 and 1 are supported  
- Value 0 means linked-list aggregation algorithm  
- Value 1 means image-sweep algorithm  

`!algo/!set_algorithm [algorithm_num]` eg.  
    
    !algo 0  
You should receive a confirmation message telling you which algorithm number you set it to  

# Package manager

conan.io

# Libraries used

Libraries the C code depends on:
- libpng
    - zlib

Ex-dependencies:
- libjpg  
- nanosvg  

# Tests

## C Tests

The C code contains a test suite (based on MUnit)
This is in Xmake as its own target
Run the Xmake tool with target **tests** to easily run the executable

    xmake -b tests
    xmake r tests

Note:  
    The test executable may require undocumented arguments, such as relative paths to images, and as such may be harder to run from Xmake directly  
    The test executable can be found in the Xmake build folder (eg. **./build/windows/x64/debug** on windows) and can be run manually from there  

# Deployment
    build C code, then Rust code, then run `sudo docker build` on a Linux machine.

    one the image is built, deploy it to your docker hub registration.