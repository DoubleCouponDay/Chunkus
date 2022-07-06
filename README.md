# Vectorizer
converts png files to svg files!

This was a group project by Samuel, Joshua, Matthew.

## Building Requirements

it has a Rust component and a C component.

Set the `VECTORIZER`, `TRAMPOLINE` and `CHANNELID` environment variables to the value of your two discord bot's secret tokens.

If you are on windows:

    + install chocolatey

    + use choco to install mingw

    + add the choco path to the `PATH` environment variable

        `C:\ProgramData\chocolatey\bin`

Install Rust lang so that you can use the `cargo` tool to work with the rust part.

```
    rustup target add x86_64-unknown-linux-musl
```

Install docker and docker compose (for the release build)

Install python3 and python3-pip (for installing conan)

    it can only be run from cmd, not powershell. Windows defender antivirus may nuke your conan.exe so you will have to add the folder in the exclusions.

```
    apt-get install python3
    apt-get install python3-pip
    python3 -m pip install --upgrade pip
    python3 -m pip install conan
```

Set the `CONAN` environment variable to the directory containing your conan packages. on windows it's: `C:\Users\(YOU))\.conan\data`. On linux it's: ~/.conan/data

disclaimer: conan no longer works with python2 and pip2 as it is using python3 syntax. If you didn't install conan from the correct place, your build will fail.

  
---

# Building the C Code

On linux install the following:

	sudo apt-get install mesa-common-dev

Clone these repos into folders adjacent to the root folder:

    https://github.com/rockcarry/ffjpeg

        git checkout --detach 3dddf985fac209db78f3e2189f8285de80f8992b

    https://github.com/madler/zlib

        git checkout --detach 21767c654d31d2dccdde4330529775c6c5fd5389

        cmake -B build -G "MinGW Makefiles" -D CMAKE_INSTALL_PREFIX="install" 
        cmake --build build -j4
        cmake --install build

    https://github.com/glennrp/libpng

        git checkout libpng16

        # Ensure zlib is built and installed first

        cmake -B build -G "MinGW Makefiles" -D ZLIB_ROOT="../zlib/install"
        cmake --build build -j4
        cmake --install build --prefix png-install

    https://github.com/sammycage/lunasvg

        git checkout --detach e612abda858b53160041381a23422cd2b4f42fbd

    https://github.com/FreeGLUTProject/freeglut

        git checkout --detach e3aa3d74f3c6a93b26fd66f81152d9c55506a6c6

    https://github.com/memononen/nanosvg

        cmake -B build -G "MinGW Makefiles"
        cmake --build build -j4
        cmake --install build --prefix install

Cmake will know where to find these projects when linking. (WE HOPE)

Also have an placeholder.bmp in the binary folder (wherever you build or install) if you want a placeholder image for non-existant images

The C code builds to `C_part\build`

From the root folder, run the following commands:

```
    cd C_part
    mkdir build
    cd build
```

continue with cross-platform instructions using a terminal with administrator privileges:

```
    cmake -B build -G "MinGW Makefiles"
    cmake --build build -j4
    cmake --install build --prefix install
```

The C code is now built into the build folder, and the final product has been installed into the install folder.

---

You can easily build all components by executing the build script. It must be executed from the root directory. (Not currently working)

  
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

The C code contains a test suite (based on MUnit). Run the `test` binary created in `C_part/build`. an single test name can be taken as argument, otherwise it runs all tests.

## Rust Tests

The Rust tests must be run in series.

```
cargo test -- --test-threads 1
```

## Running and Deployment
you can run the bot on your computer or inside a docker container.

build C code, then Rust code, then run `sudo docker build` on a Linux machine. once the image is built, deploy it to your docker hub registration.

Create a `releasebuild` environment variable and set it to true.

to use docker-compose on your computer, You will need to install OPENSSL 1.1.1

Fill in the blanks in the dockercompose.yml file to pass the environment variables into the container.

`sudo docker-compose up --build --detach`

## Debugging the C Algorithm

You can use the Vectorizer_GUI to pause frame on a current iteration of the vectorizer algorithm.

First you must build using cmake from the root directory, then open the Desktop App by running this:

    ./build/bin/Vectorizer_GUI
