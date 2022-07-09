cd ../
git clone https://github.com/madler/zlib.git
git clone https://github.com/glennrp/libpng.git
git clone https://github.com/memononen/nanosvg.git
git clone https://github.com/FreeGLUTProject/freeglut.git
git clone https://github.com/sammycage/lunasvg.git

:: Build zlib
cd zlib
git checkout v1.2.12

cmake -B build -G "MinGW Makefiles" -D CMAKE_INSTALL_PREFIX="install"
cmake --build build -j4
cmake --install build

:: build libpng
cd ../libpng
git checkout v1.6.35

cmake -B build -G "MinGW Makefiles" -D ZLIB_ROOT="../zlib/install"
cmake --build build -j4
cmake --install build --prefix png-install

:: build nanosvg
cd ../nanosvg
git checkout 3bcdf2f3cdc1bf9197c2dce81368bfc6f99205a7

cmake -B build -G "MinGW Makefiles"
cmake --build build -j4
cmake --install build --prefix install

:: build freeglut
cd ../freeglut
git checkout v3.2.2

cmake -B build -G "MinGW Makefiles"
cmake --build build -j4
cmake --install build --prefix install

:: build lunasvg
cd ../lunasvg
git checkout v2.3.1

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