# Chunkus #
converts png and jpeg to svg.

The main algorithm is written in C and is linked to the discord bot in Rust. 

An optional debugger `vec_step` is written in C++ and is for using an index to step to a specified iteration of the Vectorizer algorithm.

This was a group project by Samuel, Joshua, Matthew.

## Building Requirements
---

Set the `CHUNKUS` environment variable to the value of your discord bot's secret token.

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

If you are on linux:

    rustup default stable
	
### Rust installation on windows
	
install vc++ build tools from a vs installer

	
### Rust installation on linux
		
    sudo apt install build-essential libssl-dev -y	

Install docker and docker compose for testing the production build.

## Building the C Code
---

Ensure that git is installed.

On linux install the following:

	sudo apt-get install mesa-common-dev

Use a terminal with Administrator privileges to execute the `build.bat` / `build.sh` depending on your operating system. It must be executed from the root directory.

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

Cargo must find bot.exe so add to the PATH environment variable with the Rust build location

`(REPO CLONE FOLDER)/Rust_part/target/debug`


## Commands 
---
### Vectorize: 
Goes through all attachments of the command message, executes the algorithm on them and returns the output  

`@[bots name] [attachment or URL]`  

    @Chunkus https://cdn.discordapp.com/attachments/787470274261549056/807847299752394773/ginormous.png  

You should receive a message with `output.svg` and a preview png attached
  
### Params: 
Sets the parameters to use with the algorithm

`@[bots name] !params [chunksize] [thresholds]` eg. 

    @Chunkus !params 2 50  

You should receive a confirmation message telling you what you set the parameters to.

### Chunk Size 
Chunk size is amount of width and height in the algorithm's smallest image unit. An image is broken up into chunks, where a higher number improves speed while reducing quality (and losing information). 

### Thresholds
Thresholds is a float between 0 and 441. It is the number of threshold layers to be taking from an exponential curve.
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

build C code, then Rust code, then run `sudo docker build` on a Linux machine. once the image is built, deploy it to your docker hub registration.

to use docker-compose on your computer, You will need to install OPENSSL 1.1.1

Fill in the blanks in the dockercompose.yml file to pass the environment variables into the container.

`sudo docker-compose up --build --detach`

## Debugging the C Algorithm
---

You can use `vec_step` GUI to step through frames on a specified iteration count of the vectorizer algorithm.

First you must build using cmake from the root directory, then open the Desktop App by running this:

    ./build/bin/vec_step
