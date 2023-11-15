# Chunkus #

Converts PNG and JPEG image files to Scalable Vector Graphic (SVG) image files.

The main algorithm is written in C. It is linked with a discord bot written in Rust. The "vec_step" debugger is written in C++ which can be used for debugging the svg output.

This was a group project written by Samuel, Joshua, Matthew.

## Requirements

WARNING: Please note that SIMD instructions are used and Intel Microprocessors without the MMX extension will not work.

Set the `CHUNKUS` environment variable to the value of your discord bot's secret token.

### Installing C requirements on Windows

On Windows:

+ Install Visual Studio Community Edition
+ Add the MSVC C++ Build Tools Individual Component
+ Add the Windows SDK Individual Component
+ Install Git using Chocolatey

    choco install git

+ Install Cmake using Chocolatey

    choco install cmake --installargs 'ADD_CMAKE_TO_PATH=System'

+ Set the `CMAKE_C_COMPILER` to this:

    C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.37.32822\bin\Hostx64\x64\cl.exe

    the path may vary depending on your MSVC version.

### Installing C requirements on Linux

On Debian Linux:

    sudo apt update -y

    sudo apt install git cmake build-essential

On Arch Linux:

    sudo snap install --classic cmake

### Installing Rust requirements on Windows

if you are on windows:

    + Download and run the rustup installer.

    + run the following commands:
        rustup toolchain install stable-msvc

        rustup default stable-msvc

### Installing Rust Requirements on Linux

On Linux, run the follow commands:

    curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh

    rustup default stable

    sudo apt install build-essential libssl-dev

### Installing OpenGL on Linux

On linux, run the following to install OpenGL:
 
    sudo apt install mesa-common-dev libxi-dev libx11-dev libgl1-mesa-dev libglu1-mesa-dev -y

On Windows, OpenGL should already be available.

## Building the C part

Use a terminal to execute the `build.bat` / `build.sh` depending on your operating system type. It must be executed from the root directory.

linux:

    ./build.sh

windows:

    .\build.bat

The C code builds to `C_part\build`. It has to build all the dependencies and link them which can take about a minute. For a faster build time, run `debug.bat|sh` instead.

## Building the Rust part 


The rust component links to the C code, which depends on libpng, which depends on zlib.

In the `Rust_part` folder, run:

    cargo build

The rust part builds to `/Rust_part/target/{BUILD_MODE}/` where BUILD_MODE = (debug|release)

## Running Chunkus

`cargo run --bin trampoline`

## Commands 

### Vec: 
Goes through all attachments of the command message, executes the algorithm on them and returns the output  

`!vec [attachment or URL]`  

    !vec https://cdn.discordapp.com/attachments/787470274261549056/807847299752394773/ginormous.png  

You will receive a message with `output.svg.xz` compressed file and a preview png attached.
  
### Params: 
Sets the parameters to use with the algorithm.

`!params [chunk size] [thresholds] [colours]`

    @Chunkus !params 1 20 256

- [chunk size]

Chunk size is amount of width and height in the algorithm's smallest image unit. An image is broken up into chunks, where a higher number improves speed while reducing quality (and losing information).

Increase this if reducing the output image size would increase the speed of image processing.

- [thresholds]

Thresholds is an int between 0 and 441. It is the number of threshold layers to be taking from an exponential curve.
one threshold layer is the magnitude of the difference between colours with 3 components (RGB). 
you can use the pythagorean theorem in 3 dimensions to calculate this by:
    
    mag3 = sqrt(255^2 * 3)

- A threshold determines how easily another colour is considered a separate shape  
- A Threshold of 0 means any color that is not EXACTLY the same will be considered separate  
- A threshold of 441.67 means the only color values considered different are rgb(0,0,0) and rgb(255,255,255) (white and black).

Use this if you want increase the accuracy of the output image.

- [colours]

The number of colours that are available for an output image. 

Use this if reducing the colour complexity would increase the speed of image processing.

## C Tests
---

The C code contains a test suite `vec_test.exe` which can run all C tests or just one, when given a test name. The test names can be found in `C_part/tests/main.c`.

`vec_test.exe` is designed to be executed from the root folder.

    install\bin\vec_test read_png

You can easily debug the C tests with Visual Studio Code, on Linux. 

    + run the `.\build.bat` script at least once then run `.\debug.bat` to generate debugging symbols.

    + In the `Run and Debug` side menu, select the launch task `Linux: Launch Tests`, which can be configured in the `.vscode/launch.json file`. 

## Rust Tests
---

Rust tests have been pretty much abandoned because a test cant send a discord chat message which triggers itself.

The Rust tests must be run in series.

```
cargo test -- --test-threads 1
```

## Running and Deployment
---
you can run the bot on your computer or inside a docker container.

You will need to provide a docker-compose.yml file that fills the `CHUNKUS` environment variable with your discord bot token.

build C code, then Rust code, then run `sudo docker build`. once the image is built, push it to dockerhub.

Fill in the blanks in the dockercompose.yml file to pass the environment variables into the container.

`sudo docker-compose up --build --detach`

## Debugging the C Algorithm
---

You can use the `vec_step` GUI to step through frames on a specified iteration count of the vectorizer algorithm.

Open the Desktop App by running this:

    ./build/bin/vec_step ./build/bin/test.png

## Debugging on Windows

Ensure the Desktop Development for C++ package is installed through VS Installer.

Build in CMake with the Visual Studio Generator.

Open the `Chunkus` solution in visual studio, from the `build` folder.

In VS, set the startup project to `vec`.

In the Debugging settings, set the Current Working Directory to `${project}../../`. This allows running tests from the root folder.

Press F5 to start debugging.

## FAQ

### Why do we need .a library files from our Windows build instead of our Linux Build?

Our current Rust build script will not link with .dll files as we are static linking.

### Why did we initially choose MingW for our Build Generator?

Visual Studio Code is our primary IDE and it can only debug C/C++ if you build with MingW.

### Why are we using MSBuild instead of MingW on Windows?

We could potentially be using OpenCL in the future. Its CmakeLists.txt only works with MSBuild.

### Why can't I see printfs in the console?

Running `vec_text` will smother calls to `printf`` but you can still use `LOG_INFO()`.
