rmdir build /s /q
rmdir install /s /q

echo "Building Vectorizer..."

cmake -B build -G "MinGW Makefiles" -D CMAKE_INSTALL_PREFIX="install" -D CMAKE_BUILD_TYPE="Debug"
cmake --build build -j4
cmake --install build --prefix install