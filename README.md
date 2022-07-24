# Vectorizer
converts png files to svg files!

This was a group project by Samuel, Joshua, Matthew.

## Building Requirements

it has a C component, a CPP component, and a Rust component.

Set the `VECTORIZER`, `TRAMPOLINE` and `CHANNELID` environment variables to the value of your two discord bot's secret tokens.

If you are on windows:

    + install chocolatey

    + use choco to install mingw

    + add the choco path to the `PATH` environment variable

        `C:\ProgramData\chocolatey\bin`

Install Rust lang:

    curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh

if you are on windows:

    rustup toolchain install stable-x86_64-pc-windows-gnu
	
	rustup default stable-x86_64-pc-windows-gnu

else

    rustup default stable
	
### Rust installation on windows
	
install vc++ build tools from a vs installer

	
### Rust installation on linux
		
    sudo apt install build-essential libssl-dev -y	
		


Install docker and docker compose for testing the production build.

---

# Building the C Code

On linux install the following:

	sudo apt-get install mesa-common-dev

Clone these repos into folders adjacent to the root folder:

    https://github.com/madler/zlib

        pinned commit: v1.2.12

        cmake -B build -G "MinGW Makefiles" -D CMAKE_INSTALL_PREFIX="install" 
        cmake --build build -j4
        cmake --install build

    https://github.com/glennrp/libpng

        pinned commit: v1.6.35

        # Ensure zlib is built and installed first

    https://github.com/sammycage/lunasvg

        pinned commit: v2.3.1

    https://github.com/FreeGLUTProject/freeglut

        pinned commit: v3.2.2

    https://github.com/memononen/nanosvg

        pinned commit: 3bcdf2f3cdc1bf9197c2dce81368bfc6f99205a7

    https://github.com/winlibs/libjpeg
        
        pinned commit: libjpeg-turbo-2.1.0

Also have an placeholder.bmp in the binary folder (wherever you build or install) if you want a placeholder image for non-existant images

The C code builds to `C_part\build`

Continue with cross-platform instructions using a terminal with administrator privileges.
Execute the .SH or .BAT build script depending on your operating system. It must be executed from the root directory.

linux

    ./build.sh

windows

    .\build.bat

  
## Building the Rust code 

The rust component links to the C code, which depends on libpng, which depends on zlib.

In the `discord-v` folder, run:

    cargo build

This sets the environment variable for this single terminal instance (the variable is lost with the terminal)

It should succeed
If it doesn't, you or I have done something wrong


The rust part builds to `/discord-v/target/debug/`.

## Running

`cargo run --bin trampoline`

Cargo must find bot.exe so add to the PATH environment variable with the Rust build location

`(REPO CLONE FOLDER)/Rust_part/target/debug`

Commands to use the bot:
### Vectorize: Goes through all attachments of the command message, executes the algorithm on them and returns the output  
`!v or !vectorize` with an attachment or url eg.  

    !v https://cdn.discordapp.com/attachments/787470274261549056/807847299752394773/ginormous.png  

You should receive a message with `output.svg` and a preview png attached
  
### Params: Sets the parameters to use with the algorithm, first item is chunk_size, second item is threshold  
Chunk Size is a reverse scale for the image, higher number improve speed while reducing quality (and losing information)  
Threshold is a number between 0 and 441.67 (The square root of 255^2 * 3 (vector math))  
- The threshold determines how easily another colour is considered a separate shape  
- A Threshold of 0 means any color that is not EXACTLY the same will be considered separate  
- A threshold of 441.67 means the only color values considered different are rgb(0,0,0) and rgb(255,255,255) (white and black)  

`!vp or !vectorizerparams [chunksize] [threshold]` eg. 

    !params 2 50  
You should receive a confirmation message telling you what you set the parameters to.

## C Tests

The C code contains a test suite (based on MUnit). `vec_tests.exe` can run all C tests or just one, when given a test name. The test names can be found in `C_part/tests/main.c`. The test executable requires administrative privileges to allocate buffers.

You can easily debug the tests in VSCode if it was opened with Administrive privileges. in the `Run and Debug` side menu, select the launch task `(Windows) Launch Tests`, or the Linux equivalent if required. It will run the build task automatically before running the tests.

## Rust Tests

The Rust tests must be run in series.

```
cargo test -- --test-threads 1
```

## Running and Deployment
you can run the bot on your computer or inside a docker container.

build C code, then Rust code, then run `sudo docker build` on a Linux machine. once the image is built, deploy it to your docker hub registration.

to use docker-compose on your computer, You will need to install OPENSSL 1.1.1

Fill in the blanks in the dockercompose.yml file to pass the environment variables into the container.

`sudo docker-compose up --build --detach`

## Debugging the C Algorithm

You can use the Vectorizer_GUI to pause frame on a current iteration of the vectorizer algorithm.

First you must build using cmake from the root directory, then open the Desktop App by running this:

    ./build/bin/Vectorizer_GUI
