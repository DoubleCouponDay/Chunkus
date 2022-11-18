# CMakeList.txt : CMake project for vec_step, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.13.4)

cmake_policy(SET CMP0091 NEW)

project("vec_step" CXX)

set(VC_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

file(GLOB_RECURSE VC_SRCS "${VC_SRC_DIR}/*.cpp")
file(GLOB_RECURSE VC_HDRS "${VC_SRC_DIR}/*.h")

add_executable(vec_step ${VC_SRCS})

install(TARGETS vec_step
		DESTINATION bin)

set_target_properties(vec_step
	PROPERTIES
		CXX_STANDARD 17
		CXX_STANDARD_REQUIRED YES
		CXX_EXTENSIONS NO)

if (BUILD_SHARED_LIBS)
	add_compile_definitions(vec_step PUBLIC VECG_INTEROP)
endif()

set(OpenGL_GL_PREFERENCE "LEGACY")

find_package(OpenGL REQUIRED)

set(CMAKE_PREFIX_PATH "../../freeglut/install")
find_package(FreeGLUT REQUIRED)

if (NOT FreeGLUT_FOUND)
	message("Couldn't find a built GLUT installation, please install freeglut alongside this repo")
else()
	message("Found Freeglut installation.")
	target_link_libraries(vec_step PRIVATE FreeGLUT::freeglut)
	target_include_directories(vec_step PRIVATE $<TARGET_PROPERTY:FreeGLUT::freeglut,INTERFACE_INCLUDE_DIRECTORIES>)
	install(FILES $<TARGET_RUNTIME_DLLS:vec_step> DESTINATION bin)
endif()

target_link_libraries(vec_step PRIVATE ${OPENGL_LIBRARIES})

find_package(LUNASVG QUIET)

if (NOT LUNASVG_FOUND)
	message("linking lunasvg...")

	if (BUILD_SHARED_LIBS)
		set(BUILD_SHARED_LIBS ON CACHE BOOL "Force Build Shared Libs on" FORCE)
	endif()
	add_subdirectory(../../lunasvg ./lunasvg)

	target_include_directories(vec_step PRIVATE $<TARGET_PROPERTY:lunasvg,INTERFACE_INCLUDE_DIRECTORIES>)
	target_link_libraries(vec_step PRIVATE lunasvg)
else()
	message("Using installed lunasvg...")
	target_include_directories(vec_step PRIVATE ${LUNASVG_INCLUDE_DIRS})
	target_link_libraries(vec_step PRIVATE ${LUNASVG_LIBRARIES})
endif()

if (TARGET vec)
	message("Found vec")
	if(NOT VEC_HOTRELOAD)
		target_link_libraries(vec_step PRIVATE vec)
	endif()
	target_include_directories(vec_step PRIVATE $<TARGET_PROPERTY:vec,INTERFACE_INCLUDE_DIRECTORIES>)

	add_dependencies(vec_step vec)
else()
	message("Did not find the vec target! This should not happen! Make sure the C_part subdirectory is added first in the root CMakeLists.txt")
	return()
endif()

set_property(TARGET vec_step PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

if (UNIX)
	target_link_libraries(vec_step PRIVATE dl)
endif(UNIX)
