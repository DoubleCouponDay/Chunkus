cd ../
git clone https://github.com/madler/zlib.git
git clone https://github.com/glennrp/libpng.git
git clone https://github.com/memononen/nanosvg.git
git clone https://github.com/FreeGLUTProject/freeglut.git
git clone https://github.com/sammycage/lunasvg.git

echo "Building zlib..."
cd zlib
git checkout v1.2.12
sudo cmake -B build -G "Unix Makefiles" -D CMAKE_INSTALL_PREFIX="install"
sudo cmake --build build -j4
sudo cmake --install build

echo "Building libpng..."
cd ../libpng
git checkout v1.6.35
sudo cmake -B build -G "Unix Makefiles" -D ZLIB_ROOT="../zlib/install"
sudo cmake --build build -j4
sudo cmake --install build --prefix png-install

echo "Building nanosvg..."
cd ../nanosvg
git checkout 3bcdf2f3cdc1bf9197c2dce81368bfc6f99205a7
sudo cmake -B build -G "Unix Makefiles"
sudo cmake --build build -j4
sudo cmake --install build --prefix install

echo "Building freeglut..."
cd ../freeglut
git checkout v3.2.2
sudo apt install libxi-dev -y
sudo cmake -B build -G "Unix Makefiles"
sudo cmake --build build -j4
sudo cmake --install build --prefix install

echo "Building lunasvg..."
cd ../lunasvg
git checkout v2.3.1
sudo cmake -B build -G "Unix Makefiles"
sudo cmake --build build -j4
sudo cmake --install build --prefix install

echo "building Vectorizer..."
cd ../Vectorizer
mkdir build
sudo cmake -B build -G "Unix Makefiles" -D CMAKE_INSTALL_PREFIX=install -D CMAKE_BUILD_TYPE=Debug
sudo cmake --build build -j4
sudo cmake --install build --prefix install