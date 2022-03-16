# CMakeList.txt : CMake project for Vectorizer_GUI, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

cmake_policy(SET CMP0091 NEW)

project("Vectorizer_GUI")

set(VC_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

file(GLOB_RECURSE VC_SRCS "${VC_SRC_DIR}/*.cpp")
file(GLOB_RECURSE VC_HDRS "${VC_SRC_DIR}/*.h")

add_executable(Vectorizer_GUI ${VC_SRCS})

install(TARGETS Vectorizer_GUI
		DESTINATION bin)

set_target_properties(Vectorizer_GUI
	PROPERTIES
		CXX_STANDARD 17
		CXX_STANDARD_REQUIRED YES
		CXX_EXTENSIONS NO)
		
find_package(OpenGL REQUIRED)
find_package(GLUT QUIET)

if (NOT GLUT_FOUND)
	message("Using freeglut github repository for glut")

	execute_process(COMMAND git submodule update --init -- freeglut
					WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

	add_subdirectory(freeglut)

	
	target_include_directories(Vectorizer_GUI PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/freeglut/include")
	target_link_libraries(Vectorizer_GUI PRIVATE freeglut)
else()
	message("Found Freeglut installation: dirs: ${GLUT_INCLUDE_DIRS} libs: ${GLUT_LIBRARIES}")
	target_include_directories(Vectorizer_GUI PRIVATE ${GLUT_INCLUDE_DIRS})
	target_link_libraries(Vectorizer_GUI PRIVATE ${GLUT_LIBRARIES})
endif()


target_link_libraries(Vectorizer_GUI PRIVATE ${OPENGL_LIBRARIES})