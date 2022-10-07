# Chunkus #
converts PNG and JPEG image files to Scalable Vector Graphic (SVG) image files.

The main algorithm is written in C. It is linked with a discord bot written in Rust. The "vec_step" debugger is written in C++ which can be used for debugging the svg output.

This was a group project written by Samuel, Joshua, Matthew.

## Building Requirements
---

Set the `CHUNKUS` environment variable to the value of your discord bot's secret token.

### Installing GCC

    + install chocolatey

    + use choco to install mingw

    + add the choco path to the `PATH` environment variable

        `C:\ProgramData\chocolatey\bin`

### Installing Rust

run the follow command:

    curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh

if you are on windows:

+ install vc++ build tools from a vs installer
    rustup toolchain install stable-x86_64-pc-windows-gnu

+ run the following command:
	
        rustup default stable-x86_64-pc-windows-gnu

If you are on linux:

    rustup default stable

### Installing OpenGL

On linux run the following to install OpenGL:

    sudo apt update -y

    sudo apt install git cmake build-essential mesa-common-dev libxi-dev libx11-dev libgl1-mesa-dev libglu1-mesa-dev -y

On Windows, OpenGL should already be available.

### Rust installation on windows
	

	
### Rust installation on linux
		
    sudo apt install build-essential libssl-dev -y

### Installing Docker

Run the following command:

    curl -sSL https://get.docker.com | sh

## Building the C Code
---


Use a terminal to execute the `build.bat` / `build.sh` depending on your operating system type. It must be executed from the root directory.

linux:

    ./build.sh

windows:

    .\build.bat

The C code builds to `C_part\build`.

## Building the Rust code 
---

The rust component links to the C code, which depends on libpng, which depends on zlib.

In the `Rust_part` folder, run:

    cargo build

The rust part builds to `/Rust_part/target/debug/`.

## Running Chunkus
---

`cargo run --bin trampoline`

## Commands 
---
## Vec: 
Goes through all attachments of the command message, executes the algorithm on them and returns the output  

`!vec [attachment or URL]`  

    !vec https://cdn.discordapp.com/attachments/787470274261549056/807847299752394773/ginormous.png  

You should receive a message with `output.svg.xz` compressed file and a preview png attached.
  
## Params: 
Sets the parameters to use with the algorithm.

`!params [chunksize] [thresholds] [olours]` eg. 

    @Chunkus !params 1 20 256

You should receive a confirmation message telling you what you set the parameters to.

### Chunk Size 
Chunk size is amount of width and height in the algorithm's smallest image unit. An image is broken up into chunks, where a higher number improves speed while reducing quality (and losing information). 

### Thresholds
Thresholds is an int between 0 and 441. It is the number of threshold layers to be taking from an exponential curve.
one threshold layer is the magnitude of the difference between colours with 3 components (RGB). 
you can use the pythagorean theorem in 3 dimensions to calculate this by:
    
    mag3 = sqrt(255^2 * 3)

- A threshold determines how easily another colour is considered a separate shape  
- A Threshold of 0 means any color that is not EXACTLY the same will be considered separate  
- A threshold of 441.67 means the only color values considered different are rgb(0,0,0) and rgb(255,255,255) (white and black)  


## C Tests
---

The C code contains a test suite `vec_test.exe` which can run all C tests or just one, when given a test name. The test names can be found in `C_part/tests/main.c`.

Requirements: 

+ administrative shell

+ current work directory (CWD) set to `.\install\bin`

You can easily debug the tests with VSCode if it was opened with Administrive privileges. in the `Run and Debug` side menu, select the launch task `(Windows) Launch Tests`, or the Linux equivalent if required. It will run the build task automatically before running the tests.

## Rust Tests
---

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
