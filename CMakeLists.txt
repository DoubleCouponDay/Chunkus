cmake_minimum_required(VERSION 3.13.4)

cmake_policy(SET CMP0079 NEW)
cmake_policy(SET CMP0074 NEW)

project("Vectorizer" C CXX)


add_subdirectory(C_part)
add_subdirectory(Cpp_part)

