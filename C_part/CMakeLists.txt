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

set(LIBPNG_PATH "../../libpng/png-install")
target_include_directories(vec       PUBLIC "${LIBPNG_PATH}/include")
target_include_directories(vec_tests PUBLIC "${LIBPNG_PATH}/include")

set(TURBOJPEG_PATH "../../libjpeg/install")
target_include_directories(vec       PUBLIC "${TURBOJPEG_PATH}/include")
target_include_directories(vec_tests PUBLIC "${TURBOJPEG_PATH}/include")

# Libraries v
# Most dependent first (most dependencies)

message("linking libpng...")
    find_library(LIBPNG_LIB NAMES libpng16.a png16.a libpng.a png.a PATHS "${LIBPNG_PATH}/lib" NO_DEFAULT_PATH REQUIRED) #the windows build only produces libpng16, not libpng
    find_file(LIBPNG_HEADER NAMES png.h PATHS "${LIBPNG_PATH}/include" NO_DEFAULT_PATH REQUIRED)

    target_link_libraries(vec PUBLIC ${LIBPNG_LIB})
    target_link_libraries(vec_tests PUBLIC ${LIBPNG_LIB})
    message("Png at: ${LIBPNG_LIB}")
    message("Png header at: ${LIBPNG_HEADER}")

message("linking turbojpeg...")
    find_library(TURBOJPEG_LIB NAMES turbojpeg.a libturbojpeg.a PATHS "${TURBOJPEG_PATH}/lib" "${TURBOJPEG_PATH}/lib64" NO_DEFAULT_PATH REQUIRED)
    find_file(TURBOJPEG_HEADER NAMES turbojpeg.h PATHS "${TURBOJPEG_PATH}/include" NO_DEFAULT_PATH REQUIRED)
    
    target_link_libraries(vec       PUBLIC  ${TURBOJPEG_LIB})
    target_link_libraries(vec_tests PRIVATE ${TURBOJPEG_LIB})
    message("Turbojpeg at: ${TURBOJPEG_LIB}")
    message("turbojpeg header at: ${TURBOJPEG_HEADER}")

message("linking zlib...")
    find_library(ZLIB_LIB NAMES libzlib.a zlib.a libz.a libzlibstatic.a zlibstatic.a PATHS "../../zlib/install/lib" NO_DEFAULT_PATH REQUIRED)
    
    target_link_libraries(vec       PUBLIC  ${ZLIB_LIB})
    target_link_libraries(vec_tests PRIVATE ${ZLIB_LIB})
    message("Zlib at: ${ZLIB_LIB}")


if (NOT WIN32)
    message("linking math...")
    target_link_libraries(vec       PUBLIC m)
    target_link_libraries(vec_tests PUBLIC m)
endif()

message("linking pthread...")
    target_link_libraries(vec       PUBLIC  pthread)
    target_link_libraries(vec_tests PRIVATE pthread)

# Public header v

set_target_properties(vec PROPERTIES PUBLIC_HEADER "./src/entrypoint.h")

# Install files v

install(
  TARGETS vec
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION bin
  RUNTIME DESTINATION bin
  PUBLIC_HEADER DESTINATION include
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
    ${LIBPNG_HEADER}
    DESTINATION include
)

install(
    FILES
    ${TURBOJPEG_LIB}
    DESTINATION lib
    RENAME libturbojpeg${LIB_EXTENSION}
)

install(
    FILES
    ${TURBOJPEG_HEADER}
    DESTINATION include
)

install(FILES "../template.svg" DESTINATION bin)
install(FILES "test/test.png" DESTINATION bin)
install(FILES "test/test2.png" DESTINATION bin)
install(FILES "test/test.jpeg" DESTINATION bin)
