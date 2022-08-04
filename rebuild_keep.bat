echo "Rebuilding without removing..."

cmake --build build -j4
cmake --install build --prefix install

