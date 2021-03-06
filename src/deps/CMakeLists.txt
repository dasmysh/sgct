# Copyright Linkoping University 2011
# SGCT Project Authors see Authors.txt

cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
project(SGCT_Deps)

function(disable_external_warnings library_name)
    if (MSVC)
        target_compile_options(${library_name} PRIVATE "/W0")
    else ()
        target_compile_options(${library_name} PRIVATE "-w")
    endif ()
endfunction ()


add_subdirectory(glew)

set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Build the GLFW example programs")
set(GLFW_BUILD_TESTS OFF CACHE BOOL "Build the GLFW test programs")
set(GLFW_BUILD_DOCS OFF CACHE BOOL "Build the GLFW documentation")
set(GLFW_INSTALL OFF CACHE BOOL "Generate installation target")
if(SGCT_USE_MSVC_RUNTIMES)
	set(USE_MSVC_RUNTIME_LIBRARY_DLL ON CACHE BOOL "Use MSVC runtime library DLL" FORCE)
else()
	set(USE_MSVC_RUNTIME_LIBRARY_DLL OFF CACHE BOOL "Use MSVC runtime library DLL" FORCE)
endif()
add_subdirectory(glfw)
set_property(TARGET glfw PROPERTY FOLDER "Dependencies")
disable_external_warnings(glfw)

#add_subdirectory(zlib)
add_subdirectory(lpng)
if (TARGET zlibstatic)
    disable_external_warnings(zlibstatic)
endif ()

if(SGCT_TEXT)
	add_subdirectory(freetype) #needs zconf.h generated by lpng->zlib
endif()
add_subdirectory(libjpeg-turbo)
add_subdirectory(tinyxml2)
add_subdirectory(vrpn)

