#designed for linux build
FROM rust:1.63.0

# copy C source and Rust source
WORKDIR /

# set environment variables
ENV PATH=$PATH:/Chunkus/Rust_part/target/release

RUN apt-get update
RUN apt-get install cmake -y
RUN apt-get install build-essential -y
RUN apt-get install mesa-common-dev -y
RUN apt-get install libxi-dev -y
RUN apt-get install libx11-dev -y
RUN apt-get install libgl1-mesa-dev -y
RUN apt-get install libglu1-mesa-dev -y

RUN rustup target add x86_64-unknown-linux-musl && \
    rustup default stable

# initialize build tools

RUN git clone https://github.com/madler/zlib.git && \
    git clone https://github.com/glennrp/libpng.git && \ 
    git clone https://github.com/memononen/nanosvg.git && \ 
    git clone https://github.com/FreeGLUTProject/freeglut.git && \ 
    git clone https://github.com/sammycage/lunasvg.git && \ 
    git clone https://github.com/winlibs/libjpeg.git

RUN echo "Building zlib..."
WORKDIR /zlib
RUN git checkout v1.2.12
RUN cmake -B build -G "Unix Makefiles" -D CMAKE_INSTALL_PREFIX="install"
RUN cmake --build build -j4
RUN cmake --install build

RUN echo "Building libpng..."
WORKDIR /libpng
RUN git checkout v1.6.35
RUN cmake -B build -G "Unix Makefiles" -D ZLIB_ROOT="../zlib/install"
RUN cmake --build build -j4
RUN cmake --install build --prefix png-install

RUN echo "Building nanosvg..."
WORKDIR /nanosvg
RUN git checkout 3bcdf2f3cdc1bf9197c2dce81368bfc6f99205a7
RUN cmake -B build -G "Unix Makefiles"
RUN cmake --build build -j4
RUN cmake --install build --prefix install

RUN echo "Building freeglut..."
WORKDIR /freeglut
RUN git checkout v3.2.2
RUN cmake -B build -G "Unix Makefiles"
RUN cmake --build build -j4
RUN cmake --install build --prefix install

RUN echo "Building lunasvg..."
WORKDIR /lunasvg
RUN git checkout v2.3.1
RUN cmake -B build -G "Unix Makefiles"
RUN cmake --build build -j4
RUN cmake --install build --prefix install

RUN echo "Building libjpeg-turbo..."
WORKDIR /libjpeg
RUN git checkout libjpeg-turbo-2.1.0
RUN cmake -B build -G "Unix Makefiles"
RUN cmake --build build -j4
RUN cmake --install build --prefix install

COPY ./ /Chunkus

RUN echo "building Chunkus..."
WORKDIR /Chunkus
RUN rm -rf build
RUN mkdir build
RUN cmake -B build -G "Unix Makefiles" -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ -D CMAKE_INSTALL_PREFIX="install" -D CMAKE_BUILD_TYPE="Debug" -D CMAKE_C_COMPILER="gcc"
RUN cmake --build build -j4
RUN cmake --install build --prefix install

#build Rust code
WORKDIR /Chunkus/Rust_part/
RUN cargo build -vv --release

#create entrypoint of container
CMD ["./target/release/trampoline", "false"]
