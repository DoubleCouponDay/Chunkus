cmake_minimum_required(VERSION 2.8.12...2.8.12)

cmake_policy(SET CMP0048 NEW) #manage version numbers in project() calls
cmake_policy(SET CMP0074 NEW) #use <PackageName>_ROOT variables with find_package() calls
cmake_policy(SET CMP0079 NEW) #link with targets in other directories
cmake_policy(SET CMP0091 NEW) #use abstraction runtime flags with MSVC

if(MSVC)
    add_compile_options(/GS) #check for buffer overruns
endif()

project("vec" "C")
message("creating vec.lib and vec_test.exe builds...")
file(GLOB headersglob 
    "${PROJECT_SOURCE_DIR}/src/*.h"  
    "${PROJECT_SOURCE_DIR}/src/utility/*.h" 
    "${PROJECT_SOURCE_DIR}/src/algorithm/*.h" 
    "${PROJECT_SOURCE_DIR}/src/imagefile/*.h")

file(GLOB sourceglob
    "${PROJECT_SOURCE_DIR}/src/imagefile/*.c"
    "${PROJECT_SOURCE_DIR}/src/*.c"
    "${PROJECT_SOURCE_DIR}/src/utility/*.c"
    "${PROJECT_SOURCE_DIR}/src/algorithm/*.c")

file(GLOB testglob 
    "${PROJECT_SOURCE_DIR}/test/*.c")
    
set(TEST_SRCS ${testglob} ${sourceglob})

# Set Processor Architecture directive
if(MSVC)
    execute_process(COMMAND "echo %PROCESSOR_ARCHITECTURE%" OUTPUT_VARIABLE cpu_arch)
    message("CPU Architecture: ${cpu_arch}")
else()
    execute_process(COMMAND "uname -m" OUTPUT_VARIABLE cpu_arch)
    message("CPU Architecture: ${cpu_arch}")
endif()

if(simd_support)
    set(SIMD_SUPPORT 1)

else()
    unset(SIMD_SUPPORT)
endif()

add_executable(vec_test ${TEST_SRCS})

message("Building vec as a static library...")
add_library(vec STATIC ${sourceglob} ${headersglob})
add_compile_definitions(vec VEC_NOEXPORTDLL)
    
# Include directories v
target_include_directories(vec PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_include_directories(vec_test PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")

target_include_directories(vec       PUBLIC "../../nanosvg/install/include/nanosvg")
target_include_directories(vec_test PUBLIC "../../nanosvg/install/include/nanosvg")

target_include_directories(vec       PUBLIC "../../zlib/install/include")
target_include_directories(vec_test PUBLIC "../../zlib/install/include")

find_path(RPI_PATH NAMES libpng16.a "../../libpng/png-install/lib/aarch64-linux-gnu" NO_CACHE NO_DEFAULT_PATH)
message("RPI_PATH: ${RPI_PATH}")

if(RPI_PATH STREQUAL "RPI_PATH-NOTFOUND")
    set(LIBPNG_PATH "../../libpng/png-install")
    set(LIBPNG_INCLUDEPATH "${LIBPNG_PATH}/include")
else()
    message("aarch64-linux-gnu libpng install detected")
    set(LIBPNG_PATH "../../libpng/png-install/lib/aarch64-linux-gnu")
    set(LIBPNG_INCLUDEPATH "../../libpng/png-install/include/libpng16")
endif()

message("LIBPNG_PATH: ${LIBPNG_PATH}")
message("LIBPNG_INCLUDEPATH: ${LIBPNG_INCLUDEPATH}")

target_include_directories(vec       PUBLIC "${LIBPNG_INCLUDEPATH}")
target_include_directories(vec_test PUBLIC "${LIBPNG_INCLUDEPATH}")

set(TURBOJPEG_PATH "../../libjpeg/install")
set(LIBJPEG_INCLUDEPATH "${TURBOJPEG_PATH}/include")
target_include_directories(vec       PUBLIC "${LIBJPEG_INCLUDEPATH}")
target_include_directories(vec_test PUBLIC "${LIBJPEG_INCLUDEPATH}")

# Libraries v
# Most dependent first (most dependencies)

message("linking libpng...")
	message("png_path: ${LIBPNG_PATH}")
    message("using png .lib")

    if(MSVC)
        find_library(LIBPNG_LIB NAMES libpng16_static.lib PATHS "${LIBPNG_PATH}/lib" NO_DEFAULT_PATH REQUIRED NO_CACHE) #the windows build only produces libpng16, not libpng
    else()
        find_library(LIBPNG_LIB NAMES libpng16.lib libpng16.a png16.a libpng.a png.a libpng16.dll.a PATHS "${LIBPNG_PATH}/lib" NO_DEFAULT_PATH REQUIRED NO_CACHE) 
    endif()

    find_file(LIBPNG_HEADER NAMES png.h PATHS "${LIBPNG_INCLUDEPATH}" NO_DEFAULT_PATH REQUIRED NO_CACHE)
    message("Png at: ${LIBPNG_LIB}")
    message("Png header at: ${LIBPNG_HEADER}")
    target_link_libraries(vec PUBLIC ${LIBPNG_LIB})
    target_link_libraries(vec_test PUBLIC ${LIBPNG_LIB})

message("linking turbojpeg...")
    message("jpeg_path: ${TURBOJPEG_PATH}")

    message("using jpg .lib")
    find_library(TURBOJPEG_LIB NAMES turbojpeg.lib turbojpeg.a libturbojpeg.a PATHS "${TURBOJPEG_PATH}/lib" "${TURBOJPEG_PATH}/lib32" "${TURBOJPEG_PATH}/lib64" NO_DEFAULT_PATH REQUIRED)
    find_file(TURBOJPEG_HEADER NAMES turbojpeg.h PATHS "${LIBJPEG_INCLUDEPATH}" NO_DEFAULT_PATH REQUIRED)
    message("Turbojpeg at: ${TURBOJPEG_LIB}")
    message("turbojpeg header at: ${TURBOJPEG_HEADER}")
    target_link_libraries(vec PUBLIC ${TURBOJPEG_LIB})
    target_link_libraries(vec_test PUBLIC ${TURBOJPEG_LIB})

    if(MSVC)
        message("using turbojpeg.dll for windows tests")
        find_file(TURBOJPEG_LIBtoo NAMES turbojpeg.dll libturbojpeg.dll PATHS "${TURBOJPEG_PATH}/bin" "${TURBOJPEG_PATH}/lib32" "${TURBOJPEG_PATH}/lib64" NO_DEFAULT_PATH REQUIRED)
        install(FILES ${TURBOJPEG_LIBtoo} DESTINATION bin)
        install(FILES ${TURBOJPEG_LIBtoo} DESTINATION "bin/../../")
    endif()

message("linking zlib...")
    find_library(ZLIB_LIB NAMES zlib.lib libzlib.a zlib.a libz.a libzlibstatic.a zlibstatic.a PATHS "../../zlib/install/lib" NO_DEFAULT_PATH REQUIRED)
    
    target_link_libraries(vec       PUBLIC  ${ZLIB_LIB})
    target_link_libraries(vec_test PRIVATE ${ZLIB_LIB})
    message("Zlib at: ${ZLIB_LIB}")

	if(MSVC)
        message("using zlib.dll for windows tests")
		find_file(ZLIB_LIBtoo NAMES zlib.dll libzlib.dll PATHS "../../zlib/install/bin" NO_DEFAULT_PATH REQUIRED)
        install(FILES ${ZLIB_LIBtoo} DESTINATION bin)
        install(FILES ${ZLIB_LIBtoo} DESTINATION "bin/../../")
    endif()

if(NOT WIN32)
    message("linking math...")
    target_link_libraries(vec       PUBLIC m)
    target_link_libraries(vec_test PUBLIC m)
endif()

if (NOT MSVC)
    message("linking pthread...")
    target_link_libraries(vec       PUBLIC  pthread)
    target_link_libraries(vec_test PRIVATE pthread)
endif()

if(MSVC)
    target_compile_options(vec PUBLIC "/experimental:preprocessor")
    message("Compiling with MSVC preprocessor flag")
endif()

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
    TARGETS vec_test
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION bin
    RUNTIME DESTINATION bin
)

if (MSVC)
    set(LIB_PREFIX "")
    set(LIB_EXTENSION ".lib") # Setting a variable in case we ever need to change it
else()
    set(LIB_PREFIX "lib")
    set(LIB_EXTENSION ".a")
endif()

install(
    FILES
    ${ZLIB_LIB}
    DESTINATION lib
    RENAME ${LIB_PREFIX}zlib${LIB_EXTENSION}
)

install(
    FILES
    ${LIBPNG_LIB}
    DESTINATION lib
    RENAME ${LIB_PREFIX}png16${LIB_EXTENSION}
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
    RENAME ${LIB_PREFIX}turbojpeg${LIB_EXTENSION}
)


install(
    FILES
    ${TURBOJPEG_HEADER}
    DESTINATION include
)

install(FILES "../template.svg" DESTINATION bin)
install(FILES "test/test.png" DESTINATION bin)
install(FILES "test/test2.png" DESTINATION bin)
install(FILES "test/test3.png" DESTINATION bin)
install(FILES "test/test.jpeg" DESTINATION bin)
