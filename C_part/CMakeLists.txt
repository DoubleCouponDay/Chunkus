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
    
# Include directories v
target_include_directories(vec PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_include_directories(vec       PUBLIC "../../nanosvg/install/include/nanosvg")
target_include_directories(vec_tests PUBLIC "../../nanosvg/install/include/nanosvg")

target_include_directories(vec       PUBLIC "../../zlib/install/include")
target_include_directories(vec_tests PUBLIC "../../zlib/install/include")

target_include_directories(vec       PUBLIC "../../libpng/png-install/include")
target_include_directories(vec_tests PUBLIC "../../libpng/png-install/include")

if (WIN32)
    target_include_directories(vec       PUBLIC "../../libjpeg/install/include/lib")
    target_include_directories(vec_tests PUBLIC "../../libjpeg/install/include/lib")
else()
    target_include_directories(vec       PUBLIC "../../libjpeg/install/lib64")
    target_include_directories(vec_tests PUBLIC "../../libjpeg/install/lib64")        
endif()

# Libraries v
# Most dependant first (most dependancies)

message("linking libpng...")
    find_library(LIBPNG_LIB NAMES libpng16.a png16.a libpng.a png.a libpng16.lib png16.lib libpng.lib png.lib png libpng libpng16 png16 PATHS "../../libpng/png-install/lib" REQUIRED) #the windows build only produces libpng16, not libpng

    target_link_libraries(vec PUBLIC ${LIBPNG_LIB})
    target_link_libraries(vec_tests PUBLIC ${LIBPNG_LIB})
    message("Png at: ${LIBPNG_LIB}")

message("linking turbojpeg...")
    find_library(TURBOJPEG_LIB NAMES turbojpeg.a libturbojpeg.a turbojpeg.lib libturbojpeg.lib turbojpeg libturbojpeg PATHS "../../libjpeg/install/lib" NO_DEFAULT_PATH REQUIRED)
    target_link_libraries(vec       PUBLIC  ${TURBOJPEG_LIB})
    target_link_libraries(vec_tests PRIVATE ${TURBOJPEG_LIB})
    message("Turbojpeg at: ${TURBOJPEG_LIB}")

message("linking zlib...")
    find_library(ZLIB_LIB NAMES libzlib.a zlib.a libz.a libzlibstatic.a zlibstatic.a libzlib.lib libzlibstatic.lib libz.lib zlib.lib zlibstatic.lib z.lib zlib z libz zlibstatic PATHS "../../zlib/install/lib" NO_DEFAULT_PATH REQUIRED)
    
    target_link_libraries(vec       PUBLIC  ${ZLIB_LIB})
    target_link_libraries(vec_tests PRIVATE ${ZLIB_LIB})
    message("Zlib at: ${ZLIB_LIB}")


if (NOT WIN32)
    message("linking math...")
    target_link_libraries(vec       PUBLIC m)
    target_link_libraries(vec_tests PUBLIC m)
endif()

# Public header v

set_target_properties(vec PROPERTIES PUBLIC_HEADER "./src/entrypoint.h")

# Install files v

install(
  TARGETS vec
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION bin
  RUNTIME DESTINATION bin
)

install(
    TARGETS vec_tests
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION bin
    RUNTIME DESTINATION bin
)

set(LIB_EXTENSION ".a") # Setting a variable in case we ever need to change it

install(
    FILES
    ${ZLIB_LIB}
    DESTINATION lib
    RENAME libzlib${LIB_EXTENSION}
)

install(
    FILES
    ${LIBPNG_LIB}
    DESTINATION lib
    RENAME libpng16${LIB_EXTENSION}
)

install(
    FILES
    ${TURBOJPEG_LIB}
    DESTINATION lib
    RENAME libturbjpeg${LIB_EXTENSION}
)

install(FILES "../template.svg" DESTINATION bin)
install(FILES "../test.png" DESTINATION bin)
