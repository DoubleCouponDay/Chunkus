echo on

rmdir build /s /q
rmdir install /s /q

echo "Building Vectorizer..."

cmake -B build -G "Ninja" -D CMAKE_C_COMPILER="gcc" -D CMAKE_CXX_COMPILER="g++" -D CMAKE_INSTALL_PREFIX="install" -D CMAKE_BUILD_TYPE="Debug"
cmake --build build -j4
cmake --install build --prefix install