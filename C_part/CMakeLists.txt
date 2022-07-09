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
    target_include_directories(vec PUBLIC "../../nanosvg/install/include/nanosvg")
    target_include_directories(vec_tests PUBLIC "../../nanosvg/install/include/nanosvg")

message("Using libpng that should be in ../../libpng (and installed to ../../libpng/png-install)")

    target_include_directories(vec PUBLIC "../../libpng/png-install/include")
    target_include_directories(vec_tests PUBLIC "../../libpng/png-install/include")

    find_library(LIBPNG_LIB NAMES png libpng PATHS "../../libpng/png-install/lib" REQUIRED)
    target_link_libraries(vec PUBLIC ${LIBPNG_LIB} m) #link with linbpng and math.h
    target_link_libraries(vec_tests PUBLIC ${LIBPNG_LIB} m)

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

if (WIN32)
    # Eventually make these not hard-coded to pre-built windows dlls
    set(ZLIB_SHARED_LIB "../../zlib/install/lib/libzlibstatic.a")
    set(PNG_SHARED_LIB "../../libpng/png-install/lib/libpng16.a") #the windows build only produces libpng16, not libpng

    install(
        FILES
        ${ZLIB_SHARED_LIB}
        ${PNG_SHARED_LIB}
        DESTINATION lib
    )

else()
    set(ZLIB_SHARED_LIB "../../zlib/install/lib/libz.a")
    set(PNG_SHARED_LIB "../../libpng/png-install/lib/libpng16.a") #match windows in order to reduce complexity while rust linking
    install(
        FILES
        ${ZLIB_SHARED_LIB}
        ${PNG_SHARED_LIB}
        DESTINATION lib
    )

endif()

install(FILES "../template.svg" DESTINATION bin)
file(COPY "../template.svg" DESTINATION "./bin")
