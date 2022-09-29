echo on

cd ../
git clone https://github.com/madler/zlib.git
git clone https://github.com/glennrp/libpng.git
git clone https://github.com/memononen/nanosvg.git
git clone https://github.com/FreeGLUTProject/freeglut.git
git clone https://github.com/sammycage/lunasvg.git
git clone https://github.com/winlibs/libjpeg.git

echo "Building zlib..."
cd zlib
git checkout v1.2.12
cmake -B build -G "MinGW Makefiles" -D CMAKE_INSTALL_PREFIX="install"
cmake --build build -j4
cmake --install build

echo "Building libpng..."
cd ../libpng
git checkout v1.6.35
cmake -B build -G "MinGW Makefiles" -D ZLIB_ROOT="../zlib/install"
cmake --build build -j4
cmake --install build --prefix png-install

echo "Building nanosvg..."
cd ../nanosvg
git checkout 3bcdf2f3cdc1bf9197c2dce81368bfc6f99205a7
cmake -B build -G "MinGW Makefiles"
cmake --build build -j4
cmake --install build --prefix install

echo "Building freeglut..."
cd ../freeglut
git checkout v3.2.2
cmake -B build -G "MinGW Makefiles"
cmake --build build -j4
cmake --install build --prefix install

echo "Building lunasvg..."
cd ../lunasvg
git checkout v2.3.1
cmake -B build -G "MinGW Makefiles" -D CMAKE_INSTALL_PREFIX="install"
cmake --build build -j4
cmake --install build --prefix install

echo "Building libjpg-turbo..."
cd ../libjpeg
git checkout libjpeg-turbo-2.1.0
cmake -B build -G "MinGW Makefiles"
cmake --build build -j4
cmake --install build --prefix install

echo "building Chunkus..."
cd ../Chunkus
cmake -B build -G "Ninja" -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ -D CMAKE_INSTALL_PREFIX="install" -D CMAKE_BUILD_TYPE="Debug" -D CMAKE_C_COMPILER="gcc" -D CMAKE_EXPORT_COMPILE_COMMANDS="ON"
cmake --build build -j4
cmake --install build --prefix install

cd ./Rust_part
cargo build
cd ../Chunkus
