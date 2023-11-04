cmake_minimum_required(VERSION 2.8.12...2.8.12)

cmake_policy(SET CMP0048 NEW) #manage version numbers in project() calls
cmake_policy(SET CMP0074 NEW) #use <PackageName>_ROOT variables with find_package() calls
cmake_policy(SET CMP0079 NEW) #link with targets in other directories
cmake_policy(SET CMP0091 NEW) #use abstraction runtime flags with MSVC

project("Chunkus" C CXX)

add_subdirectory(C_part)
add_subdirectory(Cpp_part)
