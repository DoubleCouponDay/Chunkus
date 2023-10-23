cmake_minimum_required(VERSION 2.8.12)

cmake_policy(SET CMP0048 NEW) #manage version numbers in project() calls
cmake_policy(SET CMP0074 NEW) #use <PackageName>_ROOT variables with find_package() calls
cmake_policy(SET CMP0079 NEW) #link with targets in other directories
cmake_policy(SET CMP0091 NEW) #use abstraction runtime flags with MSVC

project("vec_step" CXX)
message("creating vec_step.exe build...")
set(VC_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

file(GLOB_RECURSE VC_SRCS "${VC_SRC_DIR}/*.cpp")
file(GLOB_RECURSE VC_HDRS "${VC_SRC_DIR}/*.h")

add_executable(vec_step ${VC_SRCS})

set_target_properties(vec_step
	PROPERTIES
		CXX_STANDARD 17
		CXX_STANDARD_REQUIRED YES
		CXX_EXTENSIONS NO)

set(LIBPNG_PATH "../../libpng/png-install")
set(LIBPNG_INCLUDEPATH "${LIBPNG_PATH}/include")
set(TURBOJPEG_PATH "../../libjpeg/install")
set(LIBJPEG_INCLUDEPATH "${TURBOJPEG_PATH}/include")

set(OpenGL_GL_PREFERENCE "LEGACY")

find_package(OpenGL REQUIRED)

set(CMAKE_PREFIX_PATH "../../freeglut/install")
find_package(FreeGLUT REQUIRED)

if (NOT FreeGLUT_FOUND)
	message("Couldn't find a built GLUT installation, please install freeglut alongside this repo")
else()
	message("Found Freeglut installation.")
	target_link_libraries(vec_step PUBLIC FreeGLUT::freeglut)
	target_include_directories(vec_step PRIVATE $<TARGET_PROPERTY:FreeGLUT::freeglut,INTERFACE_INCLUDE_DIRECTORIES>)
	install(FILES $<TARGET_RUNTIME_DLLS:vec_step> DESTINATION bin)
endif()

target_link_libraries(vec_step PUBLIC ${OPENGL_LIBRARIES})

find_package(LUNASVG QUIET)

if (NOT LUNASVG_FOUND)
	message("linking lunasvg...")

	if (BUILD_SHARED_LIBS)
		set(BUILD_SHARED_LIBS ON CACHE BOOL "Force Build Shared Libs on" FORCE)
	endif()
	add_subdirectory(../../lunasvg ./lunasvg)

	target_include_directories(vec_step PRIVATE $<TARGET_PROPERTY:lunasvg,INTERFACE_INCLUDE_DIRECTORIES>)
	target_link_libraries(vec_step PUBLIC lunasvg)
else()
	message("Using installed lunasvg...")
	target_include_directories(vec_step PRIVATE ${LUNASVG_INCLUDE_DIRS})
	target_link_libraries(vec_step PUBLIC ${LUNASVG_LIBRARIES})
endif()

if (TARGET vec)
	message("Found vec")
	if(NOT VEC_HOTRELOAD)
		target_link_libraries(vec_step PUBLIC vec)
	endif()
	target_include_directories(vec_step PRIVATE $<TARGET_PROPERTY:vec,INTERFACE_INCLUDE_DIRECTORIES>)

	add_dependencies(vec_step vec)
else()
	message("Did not find the vec target! This should not happen! Make sure the C_part subdirectory is added first in the root CMakeLists.txt")
	return()
endif()

set_property(TARGET vec_step PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

if (UNIX)
	target_link_libraries(vec_step PUBLIC dl)
endif(UNIX)

message("installing libpng...")
	message("png_path: ${LIBPNG_PATH}")
	target_include_directories(vec PUBLIC "${LIBPNG_INCLUDEPATH}")

	message("png_path: ${LIBPNG_PATH}")
    message("using png .lib")
    find_library(LIBPNG_LIB NAMES libpng16_static.lib PATHS "${LIBPNG_PATH}/lib" NO_DEFAULT_PATH REQUIRED NO_CACHE)
    find_file(LIBPNG_HEADER NAMES png.h PATHS "${LIBPNG_INCLUDEPATH}" NO_DEFAULT_PATH REQUIRED NO_CACHE)
    message("Png at: ${LIBPNG_LIB}")
    message("Png header at: ${LIBPNG_HEADER}")
    target_link_libraries(vec PUBLIC ${LIBPNG_LIB})
    target_link_libraries(vec_test PUBLIC ${LIBPNG_LIB})

    message("Png at: ${LIBPNG_LIB}")
    message("Png header at: ${LIBPNG_HEADER}")

message("installing turbojpeg...")
    message("jpeg_path: ${TURBOJPEG_PATH}")
	target_include_directories(vec PUBLIC "${LIBJPEG_INCLUDEPATH}")

	if(NOT MSVC)
		message("using jpg .lib")
		find_library(TURBOJPEG_LIBtoo NAMES turbojpeg.lib turbojpeg.a libturbojpeg.a PATHS "${TURBOJPEG_PATH}/lib" "${TURBOJPEG_PATH}/lib32" "${TURBOJPEG_PATH}/lib64" NO_DEFAULT_PATH REQUIRED)
		find_file(TURBOJPEG_HEADER NAMES turbojpeg.h PATHS "${LIBJPEG_INCLUDEPATH}" NO_DEFAULT_PATH REQUIRED)
	endif()

    message("Turbojpeg at: ${TURBOJPEG_LIBtoo}")
    message("turbojpeg header at: ${TURBOJPEG_HEADER}")

message("installing zlib...")
	find_file(TURBOJPEG_HEADER NAMES turbojpeg.h PATHS "${LIBJPEG_INCLUDEPATH}" NO_DEFAULT_PATH REQUIRED)
	install(FILES ${TURBOJPEG_HEADER} DESTINATION bin)
		
	if(NOT MSVC)
		find_library(ZLIB_LIBtoo NAMES zlib.lib libzlib.a zlib.a libz.a libzlibstatic.a zlibstatic.a PATHS "../../zlib/install/lib" NO_DEFAULT_PATH REQUIRED)
		target_link_libraries(vec_test PRIVATE ${ZLIB_LIBtoo})
	endif()
    message("Zlib at: ${ZLIB_LIBtoo}")

message("Installing vec_step...")

install(TARGETS vec_step DESTINATION bin)
