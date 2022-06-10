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


message("Using nanosvg that should be in ../../nanosvg")
add_subdirectory(../../nanosvg ./nanosvg)
target_include_directories(vec PRIVATE $<TARGET_PROPERTY:nanosvg,INTERFACE_INCLUDE_DIRECTORIES>)
target_link_libraries(vec PRIVATE nanosvg)
target_include_directories(vec_tests PRIVATE $<TARGET_PROPERTY:nanosvg,INTERFACE_INCLUDE_DIRECTORIES>)
target_link_libraries(vec_tests PRIVATE nanosvg)

message("Using zlib that should be in ../../zlib")
add_subdirectory(../../zlib ./zlib)
target_include_directories(vec PRIVATE $<TARGET_PROPERTY:zlib,INTERFACE_INCLUDE_DIRECTORIES>)
target_link_libraries(vec PRIVATE zlib)
target_include_directories(vec_tests PRIVATE $<TARGET_PROPERTY:zlib,INTERFACE_INCLUDE_DIRECTORIES>)
target_link_libraries(vec_tests PRIVATE zlib)

message("Using libpng that should be in ../../libpng")
add_subdirectory(../../libpng ./libpng)
target_include_directories(vec PRIVATE $<TARGET_PROPERTY:libpng,INTERFACE_INCLUDE_DIRECTORIES>)
target_link_libraries(vec PRIVATE libpng)
target_include_directories(vec_tests PRIVATE $<TARGET_PROPERTY:libpng,INTERFACE_INCLUDE_DIRECTORIES>)
target_link_libraries(vec_tests PRIVATE libpng)

target_include_directories(vec INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src)

set_target_properties(vec PROPERTIES PUBLIC_HEADER "./src/entrypoint.h")

install(
  TARGETS vec
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION bin
  RUNTIME DESTINATION bin
)

install(FILES "../template.svg" DESTINATION bin)

file(COPY "../template.svg" DESTINATION "./bin")
