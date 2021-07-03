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

disclaimer: conan no longer works with python2 and pip2 as it has python3 only syntax. If you didn't install conan from the correct place, your build will fail.
  
---

## Building the C code
<br>

The C code builds to `C_part\build`

```
    cd C_part/build
    conan install ../
    conan profile update settings.compiler.libcxx-libstc++11 default
    cmake ../
    make
```
  
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

The C code contains a test suite (based on MUnit)

# Deployment
    
    build C code, then Rust code, then run `sudo docker build` on a Linux machine. once the image is built, deploy it to your docker hub registration.

    

# Operation
    you can run the bot on your computer or inside a docker container.

# Running on your computer
    Set the VECTORIZER and TRAMPOLINE environment variables to the value of your discord bot's secret token.
    This will allow trampoline to start the bots.

# Running as a container
    You will need to install OPENSSL 1.1.1

    Fill in the blanks in the dockercompose.yml file and run it.

    `sudo docker-compose up --build --detach`

    
