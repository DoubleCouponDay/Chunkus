
if [ $# -eq 0 ]; then
    echo "Usage: ./build.sh <buildtype>"
    echo "buildtype can be Debug or Release"
    exit 1
fi

buildtype=$1
echo "building type: $buildtype"

cd ../
git clone https://github.com/madler/zlib.git
git clone https://github.com/glennrp/libpng.git
git clone https://github.com/memononen/nanosvg.git
git clone https://github.com/FreeGLUTProject/freeglut.git
git clone https://github.com/sammycage/lunasvg.git
git clone https://github.com/winlibs/libjpeg.git
git clone https://github.com/KhronosGroup/OpenCL-SDK.git

echo "Building zlib..."
cd zlib
git checkout v1.2.12
cmake -B build -G "Unix Makefiles" -D CMAKE_INSTALL_PREFIX="install"
cmake --build build -j4 --config $buildtype
cmake --install build

echo "Building libpng..."
cd ../libpng
git checkout v1.6.35
cmake -B build -G "Unix Makefiles" -D ZLIB_ROOT="../zlib/install"
cmake --build build -j4 --config $buildtype
cmake --install build --prefix png-install

echo "Building nanosvg..."
cd ../nanosvg
git checkout 3bcdf2f3cdc1bf9197c2dce81368bfc6f99205a7
cmake -B build -G "Unix Makefiles"
cmake --build build -j4 --config $buildtype
cmake --install build --prefix install

echo "Building freeglut..."
cd ../freeglut
git checkout v3.2.2
cmake -B build -G "Unix Makefiles"
cmake --build build -j4 --config $buildtype
cmake --install build --prefix install

echo "Building lunasvg..."
cd ../lunasvg
git checkout v2.3.1
cmake -B build -G "Unix Makefiles"
cmake --build build -j4 --config $buildtype
cmake --install build --prefix install

echo "Building libjpeg-turbo..."
cd ../libjpeg
git checkout libjpeg-turbo-2.1.0
cmake -B build -G "Unix Makefiles"
cmake --build build -j4 --config $buildtype
cmake --install build --prefix install

# echo "Building opencl..."
# cd ../OpenCL-SDK
# git checkout v2022.09.30
# cmake -G "Unix Makefiles" -D CMAKE_INSTALL_PREFIX=install -B build -S ./
# cmake --build build -j4 --config $buildtype --target install -- /m /v:minimal
# cmake --install build --prefix install

echo "building Chunkus..."
cd ../Chunkus
cmake -B build -G "Unix Makefiles" -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ -D CMAKE_INSTALL_PREFIX="install" -D CMAKE_BUILD_TYPE="$buildtype" -D CMAKE_C_COMPILER="gcc" -D CMAKE_EXPORT_COMPILE_COMMANDS="ON"
cmake --build build -j4 --config $buildtype
cmake --install build --prefix install

cd ./Rust_part
cargo build
cd ../
