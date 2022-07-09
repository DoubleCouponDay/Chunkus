cd ../
git clone https://github.com/madler/zlib.git
git clone https://github.com/glennrp/libpng.git
git clone https://github.com/sammycage/lunasvg.git
git clone https://github.com/FreeGLUTProject/freeglut.git
git clone https://github.com/memononen/nanosvg.git

:: Build zlib
cd zlib
git checkout --detach 21767c654d31d2dccdde4330529775c6c5fd5389

cmake -B build -G "MinGW Makefiles" -D CMAKE_INSTALL_PREFIX="install"
cmake --build build -j4
cmake --install build

:: build libpng
cd ../libpng
git checkout libpng16

cmake -B build -G "MinGW Makefiles" -D ZLIB_ROOT="../zlib/install"
cmake --build build -j4
cmake --install build --prefix png-install

:: build nanosvg
cd ../nanosvg

cmake -B build -G "MinGW Makefiles"
cmake --build build -j4
cmake --install build --prefix install

:: --------------------------------
:: Vectorizer build
:: --------------------------------
cd ../Vectorizer

cmake -B build -G "MinGW Makefiles" -D CMAKE_INSTALL_PREFIX="install" -D CMAKE_BUILD_TYPE="Debug"
cmake --build build -j4
cmake --install build --prefix install