cmake_minimum_required(VERSION 2.8.12)

cmake_policy(SET CMP0079 NEW)
cmake_policy(SET CMP0074 NEW)

project("Chunkus" C CXX)

add_subdirectory(C_part)
add_subdirectory(Cpp_part)
