cmake_minimum_required(VERSION 3.13.4)

project("vec" C)

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

message("including math.h")
target_link_libraries(vec PUBLIC m)
target_link_libraries(vec_tests PUBLIC m)

message("linking nanosvg")
    target_include_directories(vec PUBLIC "../../nanosvg/install/include/nanosvg")
    target_include_directories(vec_tests PUBLIC "../../nanosvg/install/include/nanosvg")

message("linking zlib")
    target_include_directories(vec PUBLIC "../../zlib/install/include")
    target_include_directories(vec_tests PUBLIC "../../zlib/install/include")

    if(WIN32)
        find_library(ZLIB_LIB NAMES zlibstatic libzlibstatic PATHS "../../zlib/install/lib" NO_DEFAULT_PATH REQUIRED)
        set(ZLIB_LIB "../../zlib/install/lib/libzlibstatic.a")

        get_filename_component(REAL_ZLIB_LIB "${ZLIB_LIB}" REALPATH)

        target_link_libraries(vec PUBLIC "${REAL_ZLIB_LIB}")
        target_link_libraries(vec_tests PRIVATE "${REAL_ZLIB_LIB}")
    else()
        set(ZLIB_LIB "../../zlib/install/lib/libz.a")
        find_library(ZLIB_LIB NAMES zlib z "../../zlib/install/lib" NO_DEFAULT_PATH REQUIRED)
        target_link_libraries(vec PUBLIC "libz.a") #dont ask I have no idea why it only works this way. probably differences between gcc and mingw
        target_link_libraries(vec_tests PUBLIC "libz.a")
    endif()

message("linking libpng")
set(PNG_LIB "../../libpng/png-install/lib/libpng16.a")

target_include_directories(vec PUBLIC "../../libpng/png-install/include")
target_include_directories(vec_tests PUBLIC "../../libpng/png-install/include")

get_filename_component(REAL_PNG_LIB "${PNG_LIB}" REALPATH)

target_link_libraries(vec PUBLIC "${REAL_PNG_LIB}")
target_link_libraries(vec_tests PRIVATE "${REAL_PNG_LIB}")

target_link_libraries(vec PUBLIC "pthread")
target_link_libraries(vec_tests PRIVATE "pthread")

message("linking libjpg-turbo")
    target_include_directories(vec PUBLIC "../../libjpeg/install/include/lib")
    target_include_directories(vec_tests PUBLIC "../../libjpeg/install/include/lib")
    find_library(JPEGTURBO_LIB NAMES turbojpeg libturbojpeg PATHS "../../libjpeg/install/lib" NO_DEFAULT_PATH REQUIRED)
    target_link_libraries(vec PUBLIC ${JPEGTURBO_LIB})
    target_link_libraries(vec_tests PUBLIC ${JPEGTURBO_LIB})

target_include_directories(vec INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src)
set_target_properties(vec PROPERTIES PUBLIC_HEADER "./src/entrypoint.h")
install(FILES "../template.svg" DESTINATION bin)
file(COPY "../template.svg" DESTINATION "./bin")

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
    ${PNG_LIB}
    DESTINATION lib
    RENAME libpng16${LIB_EXTENSION}
)

install(
    FILES
    ${JPEGTURBO_LIB}
    DESTINATION lib
    RENAME libturbojpeg${LIB_EXTENSION}
)


