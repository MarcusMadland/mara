cmake_minimum_required(VERSION 3.10.2) # version supported by android studio
project(mara)
cmake_policy(SET CMP0054 NEW)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "cmake")
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Makefile generators on apple need this flag to compile mixed objective/c++
if(APPLE AND NOT XCODE)
	set(CMAKE_CXX_FLAGS "-ObjC++")
endif()

if(MSVC AND (MSVC_VERSION GREATER_EQUAL 1914))
	add_compile_options("/Zc:__cplusplus")
endif()

include(CMakeDependentOption)

# Options
option(MARA_CONFIG_IMGUI "Build mara with imgui library" ON)
option(MARA_CONFIG_WITH_SAMPLES "Build mara with samples" OFF)
option(GRAPHICS_CONFIG_MULTITHREADED "Build graphics with multithreaded configuration" ON)

# Variables
set(GRAPHICS_OPENGL_VERSION "" CACHE STRING "Specify minimum opengl version")
set(GRAPHICS_OPENGLES_VERSION "" CACHE STRING "Specify minimum OpenGL ES version")
set(GRAPHICS_LIBRARY_TYPE "STATIC" CACHE STRING "Linking type for library")
set_property(CACHE GRAPHICS_LIBRARY_TYPE PROPERTY STRINGS "STATIC" "SHARED")
set(GRAPHICS_CONFIG_DEFAULT_MAX_ENCODERS "" CACHE STRING "Specify default maximum encoder count (multithreaded only)")
set(GRAPHICS_CONFIG_MAX_DRAW_CALLS "" CACHE STRING "Specify maximum draw calls")
set(GRAPHICS_CONFIG_MAX_VIEWS "" CACHE STRING "Specify maximum view count (should be a power of two)")
set(GRAPHICS_CONFIG_MAX_FRAME_BUFFERS "" CACHE STRING "Specify maximum frame buffers")
set(GRAPHICS_CONFIG_MAX_VERTEX_LAYOUTS "" CACHE STRING "Specify maximum vertex layouts")
set(GRAPHICS_CONFIG_MAX_VERTEX_BUFFERS "" CACHE STRING "Specify maximum vertex buffer count")
set(GRAPHICS_CONFIG_MAX_DYNAMIC_VERTEX_BUFFERS "" CACHE STRING "Specify maximum dynamic vertex buffer count")
set(GRAPHICS_CONFIG_MAX_INDEX_BUFFERS "" CACHE STRING "Specify maximum index buffer count")
set(GRAPHICS_CONFIG_MAX_DYNAMIC_INDEX_BUFFERS "" CACHE STRING "Specify maximum dynamic index buffer count")
set(GRAPHICS_CONFIG_MAX_TEXTURES "" CACHE STRING "Specify maximum texture count")
set(GRAPHICS_CONFIG_MAX_TEXTURE_SAMPLERS "" CACHE STRING "Specify maximum texture samplers")
set(GRAPHICS_CONFIG_MAX_SHADERS "" CACHE STRING "Specify shader count")
set(GRAPHICS_CONFIG_SORT_KEY_NUM_BITS_PROGRAM "" CACHE STRING "Specify maximum program count as 2^(program bits)")
set(GRAPHICS_CMAKE_USER_SCRIPT "" CACHE STRING "Path to custom cmake script to include")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Sources
file(
    GLOB_RECURSE 
    FILE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/mara/src/**.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mara/src/**.h
    ${CMAKE_CURRENT_SOURCE_DIR}/mara/include/**.h

if (MARA_CONFIG_IMGUI)
    ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/imgui/**.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/imgui/**.h
endif()
)

# Solution Filters
foreach(_source IN ITEMS ${FILE_SOURCES})
	if (IS_ABSOLUTE "${_source}")
            file(RELATIVE_PATH _source_rel "${CMAKE_CURRENT_SOURCE_DIR}" "${_source}")
        else()
            set(_source_rel "${_source}")
        endif()
        get_filename_component(_source_path "${_source_rel}" PATH)
        string(REPLACE "/" "\\" _source_path_msvc "${_source_path}")
        source_group("${_source_path_msvc}" FILES "${_source}")
endforeach()

# Finalize Library
add_library(
    ${PROJECT_NAME}
    "${FILE_SOURCES}"
    )
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/mara/include)

if (MARA_CONFIG_IMGUI)
   target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty)
endif()

# graphics
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/graphics)
target_link_libraries(
    ${PROJECT_NAME}
    graphics
)

# samples
if (MARA_CONFIG_WITH_SAMPLES)
	include(${CMAKE_CURRENT_SOURCE_DIR}/samples/00-empty/sample.cmake)
	include(${CMAKE_CURRENT_SOURCE_DIR}/samples/01-basic/sample.cmake)
endif()


