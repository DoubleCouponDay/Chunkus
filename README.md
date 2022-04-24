# Vectorizer
converts png files to svg files!

This was a group project created and maintained by Samuel, Joshua, Matthew.

## Building Requirements

it has a Rust component and a C component.

Set the `VECTORIZER` and `TRAMPOLINE` environment variables to the value of your two discord bot's secret tokens.

use this template for the `secrettoken.rs` file that you must create, in the `Rust_part/src` folder.

```
    pub fn gettoken() -> &'static str { }

    pub fn getwatchertoken() -> &'static str { }

    pub fn getchannelid() -> u64 { }
```

For windows there are some extra steps:

+ install visual studio 2019

+ add msbuild.exe to your PATH environment variable

    `C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\msbuild.exe`


Install Rust lang so that you can use the `cargo` tool to work with the discord-v folder as a rust project.  

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

## Building the C code
<br>

To build on windows, clone freeglut from https://github.com/FreeGLUTProject/freeglut into Cpp_part/freeglut.
On linux just make sure you have the freeglut package

Also have an placeholder.bmp in the binary folder (wherever you build or install) if you want a placeholder image for non-existant images

The C code builds to `C_part\build`

From the root folder, run the following commands:

```
    cd C_part
    mkdir build
    cd build
```

If on linux, run this line:

```
conan profile new default --detect
conan profile update settings.compiler.libcxx=libstdc++11 default
```

Then continue with cross-platform instructions:

```
    conan install ../ --build=libpng --build=zlib
    cd ../
    cmake -B build
    cmake --build build
```

The C code is now build into `/build/bin/vec.lib`

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
You should receive a confirmation message telling you what you set the parameters to  
  
### Set Algorithm: Sets which algorithm is used for shape identification  
Currently only values of 0 and 1 are supported  
- Value 0 means linked-list aggregation algorithm  
- Value 1 means image-sweep algorithm  

`!va or !vectorizeralgorithm [algorithm_num]`

## C Tests

The C code contains a test suite (based on MUnit). Run the `test` binary created in `C_part/build`. an single test name can be taken as argument, otherwise it runs all tests.

## Rust Tests

Because there are end to end tests which require access to the bot token, the Rust tests must be run in series.

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

First you must run cmake install, after building:

    cmake --build build
    cmake --install build --prefix build

Open the Desktop App by running this:

    /build/bin/Vectorizer_GUI
