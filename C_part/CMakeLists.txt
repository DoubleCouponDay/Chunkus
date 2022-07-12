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

message("linking zlib")
    target_include_directories(vec PUBLIC "../../zlib/install/include")
    target_include_directories(vec_tests PUBLIC "../../zlib/install/include")

    if (WIN32)
        find_library(ZLIB_LIB NAMES zlibstatic libzlibstatic PATHS "../../zlib/install/lib" NO_DEFAULT_PATH REQUIRED)
    else()
        find_library(ZLIB_LIB NAMES zlib z "../../zlib/install/lib" NO_DEFAULT_PATH REQUIRED)
    endif()
    target_link_libraries(vec PUBLIC ${ZLIB_LIB})
    target_link_libraries(vec_tests PUBLIC ${ZLIB_LIB})

message("Using libpng that should be in ../../libpng (and installed to ../../libpng/png-install)")

    target_include_directories(vec PUBLIC "../../libpng/png-install/include")
    target_include_directories(vec_tests PUBLIC "../../libpng/png-install/include")

    find_library(LIBPNG_LIB NAMES png libpng libpng16 PATHS "../../libpng/png-install/lib" REQUIRED)
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

install(FILES ${COMBINED_LIB} DESTINATION lib)

if (WIN32)
    # Eventually make these not hard-coded to pre-built windows dlls
    set(ZLIB_LIB "../../zlib/install/lib/libzlibstatic.a")
    set(PNG_LIB "../../libpng/png-install/lib/libpng16.a") #the windows build only produces libpng16, not libpng

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
        
else()
    set(ZLIB_LIB "../../zlib/install/lib/libz.a")
    set(PNG_LIB "../../libpng/png-install/lib/libpng16.a") #match windows in order to reduce complexity while rust linking
    
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

endif()

install(FILES "../template.svg" DESTINATION bin)
file(COPY "../template.svg" DESTINATION "./bin")
