cmake_minimum_required(VERSION 3.13.4)

project("Vectorizer")

cmake_policy(SET CMP0079 NEW)

set(BUILD_SHARED_LIBS ON)

add_subdirectory(C_part)
add_subdirectory(Cpp_part)
