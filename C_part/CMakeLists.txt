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

message("including math.h")
target_link_libraries(vec PUBLIC m)
target_link_libraries(vec_tests PUBLIC m)

message("linking nanosvg")
    target_include_directories(vec PUBLIC "../../nanosvg/install/include/nanosvg")
    target_include_directories(vec_tests PUBLIC "../../nanosvg/install/include/nanosvg")

message("linking zlib")
target_include_directories(vec PUBLIC "../../zlib/install/include")
target_include_directories(vec_tests PUBLIC "../../zlib/install/include")

if (WIN32)
    set(ZLIB_LIB "../../zlib/install/lib/libzlibstatic.a")
    set(ZLIB_NAME "libzlibstatic.a")
    find_library(ZLIB_LIB NAMES zlibstatic libzlibstatic PATHS "../../zlib/install/lib" NO_DEFAULT_PATH REQUIRED)
else()
    set(ZLIB_NAME "libz.a")

endif()

target_link_libraries(vec PUBLIC ${ZLIB_NAME})
target_link_libraries(vec_tests PUBLIC ${ZLIB_NAME})

message("linking libpng")
    set(PNG_LIB "../../libpng/png-install/lib/libpng16.a")

    target_include_directories(vec PUBLIC "../../libpng/png-install/include")
    target_include_directories(vec_tests PUBLIC "../../libpng/png-install/include")

    find_library(LIBPNG_LIB NAMES png libpng libpng16 PATHS "../../libpng/png-install/lib" REQUIRED) #the windows build only produces libpng16, not libpng
    target_link_libraries(vec PUBLIC ${LIBPNG_LIB})
    target_link_libraries(vec_tests PUBLIC ${LIBPNG_LIB})

message("linking turbojpeg...")
    if(WIN32)
        target_include_directories(vec PUBLIC "../../libjpeg/install/include/lib")
        target_include_directories(vec_tests PUBLIC "../../libjpeg/install/include/lib")
        find_library(TURBOJPEG_LIB NAMES turbojpeg libturbojpeg PATHS "../../libjpeg/install/lib" NO_DEFAULT_PATH REQUIRED)
        target_link_libraries(vec PUBLIC ${TURBOJPEG_LIB})
        target_link_libraries(vec_tests PRIVATE ${TURBOJPEG_LIB})
    else()
        target_include_directories(vec PUBLIC "../../libjpeg/install/lib64")
        target_include_directories(vec_tests PUBLIC "../../libjpeg/install/lib64")        
        set(TURBOJPEG_LIB "../../libjpeg/install/lib64/libturbojpeg.a")
        get_filename_component(REAL_TURBOJPEG_LIB ${TURBOJPEG_LIB} REALPATH)
        target_link_libraries(vec PUBLIC ${REAL_TURBOJPEG_LIB})
        target_link_libraries(vec_tests PRIVATE ${REAL_TURBOJPEG_LIB})
    endif()

target_include_directories(vec INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src)
set_target_properties(vec PROPERTIES PUBLIC_HEADER "./src/entrypoint.h")

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
    ${PNG_LIB}
    DESTINATION lib
    RENAME libpng16${LIB_EXTENSION}
)

install(
    FILES
    ${PNG_LIB}
    DESTINATION lib
    RENAME libturbjpeg${LIB_EXTENSION}
)

install(FILES "../template.svg" DESTINATION bin)
file(COPY "../template.svg" DESTINATION "./bin")
