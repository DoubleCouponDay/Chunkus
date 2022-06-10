cmake_minimum_required(VERSION 3.13.4)

project("vec" "C")

set(PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB headersglob 
    "${PROJECT_SOURCE_DIR}/src/*.h"  
    "${PROJECT_SOURCE_DIR}/src/utility/*.h" 
    "${PROJECT_SOURCE_DIR}/src/nsvg/*.h" 
    "${PROJECT_SOURCE_DIR}/src/imagefile/*.h")

file(GLOB sourceglob
    "${PROJECT_SOURCE_DIR}/src/imagefile/*.c"
    "${PROJECT_SOURCE_DIR}/src/*.c"
    "${PROJECT_SOURCE_DIR}/src/utility/*.c"
    "${PROJECT_SOURCE_DIR}/src/nsvg/*.c")

file(GLOB testglob
        "${PROJECT_SOURCE_DIR}/test/*.c")
    
set(TEST_SRCS ${testglob} ${sourceglob})

add_executable(vec_tests ${TEST_SRCS})

if(BUILD_SHARED_LIBS)
    message("Building vec as shared library")
    add_library(vec SHARED ${sourceglob} ${headersglob})
    add_compile_definitions(vec VEC_EXPORTDLL)
else()
    message("Build vec as a static library")
    add_library(vec STATIC ${sourceglob} ${headersglob})
    add_compile_definitions(vec VEC_NOEXPORTDLL)
endif()

target_include_directories(vec INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src)

if (DEFINED CONAN_CMAKE_DIR)
    include(${CONAN_CMAKE_DIR}/conanbuildinfo.cmake)
else()
    include(${CMAKE_CURRENT_SOURCE_DIR}/../build/conanbuildinfo.cmake)
endif(DEFINED CONAN_CMAKE_DIR)
conan_basic_setup(TARGETS)

target_link_libraries(vec CONAN_PKG::libpng CONAN_PKG::nanosvg CONAN_PKG::libjpeg)
target_link_libraries(vec_tests CONAN_PKG::libpng CONAN_PKG::nanosvg CONAN_PKG::libjpeg)


set_target_properties(vec PROPERTIES PUBLIC_HEADER "./src/entrypoint.h")

install(
  TARGETS vec
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION bin
  RUNTIME DESTINATION bin
)

install(FILES "../template.svg" DESTINATION bin)

file(COPY "../template.svg" DESTINATION "./bin")
