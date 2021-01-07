# Rasterizer
the project objective

convert jpg, png files to svg files!

it has a Rust component and a C component.

# Getting started

You can use this program as a discord bot by clicking the link below. Fill in the last request parameter with your discord guild id.

    https://discord.com/api/oauth2/authorize?client_id=690684027019067393&scope=bot&permissions=51200&guild_id=

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

This gets xmake to find the correct compiler and linker, and allows conan to download the dependancies  

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
This means you will need to copy and rename:  
 - a libpng static library into `discord-v` as `libpng.lib` on windows and probably like `liblibpng.a` on linux  
 - a zlib static library into `discord-v` as `zlib.lib` on windows and most likely `libzlib.a` on linux  
See note for help on obtaining **libpng** and **zlib** static libraries

Now in the `discord-v` folder, run:

    cargo build

It should succeed
If it doesn't, you or I have done something wrong

Note on obtaining **libpng** and **zlib** library files:  
    If you have built the C component, this means Conan has already downloaded **libpng** and **zlib** itself  
    Therefore you should find a file similar to **libpng16.lib** or **libpng.a** wherever conan has installed those files  
    I found my (windows machine) **libpng16.lib** and **zlib.lib** in  
    - `C:\Users\<username>\.conan\data\libpng\1.6.37\_\_\package\c6eb1796d6a9d59338fa618b78b90859da126b06\lib\`  
    - `C:\Users\<username>\.conan\data\zlib\1.2.11\_\_\package\3fb49604f9c2f729b85ba3115852006824e72cab\lib\`  

The rust part builds to `/discord-v/target/debug/`.

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
