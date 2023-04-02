echo on

echo "building Chunkus..."
cd ../Chunkus
cmake -B build -G "Visual Studio 17 2022" -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ -D CMAKE_INSTALL_PREFIX="install" -D CMAKE_BUILD_TYPE="Debug" -D CMAKE_C_COMPILER="gcc" -D CMAKE_EXPORT_COMPILE_COMMANDS="ON"
cmake --build build -j4 --config Debug
cmake --install build --prefix install

cd ./Rust_part
cargo build
cd ../
