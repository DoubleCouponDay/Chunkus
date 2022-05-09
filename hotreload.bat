Rem Runs the necessary cmake commands to compile and install the vec target. Expected to be called from the install folder with the build folder at ../build
cmake --build ../build -t vec
cmake --install ../build --prefix ./