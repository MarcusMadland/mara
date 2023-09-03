# Project Info
cmake_minimum_required(VERSION 3.18 FATAL_ERROR)
cmake_policy(VERSION 3.18)
project(mengine)
enable_language(C)
enable_language(CXX)

# CMake Settings
set(CMAKE_SUPPRESS_REGENERATION true)
set(DCMAKE_GENERATOR_PLATFORM "x64")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Sources
file(GLOB_RECURSE FILE_SOURCES RELATIVE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/source/**.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/**.hpp
)

# Solution Filters
foreach(source IN LISTS FILE_SOURCES)
    get_filename_component(source_path "${source}" PATH)
    string(REPLACE "/" "\\" source_path_msvc "${source_path}")
    string(REPLACE "source" "" source_path_final "${source_path_msvc}")
    source_group("${source_path_final}" FILES "${source}")
endforeach()

# Functions ===================================================
function(mengine_add_executable targetProject targetSources)
	xwin_add_executable(
	    ${targetProject}
	    "${targetSources}"
	)
endfunction()
# =============================================================

# Finalize Library
add_library(
    ${PROJECT_NAME}
    "${FILE_SOURCES}"
    )
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/source/include)

# Dependencies ================================================
add_subdirectory(mapp ${CMAKE_BINARY_DIR}/mapp) # mapp
add_subdirectory(mcore ${CMAKE_BINARY_DIR}/mcore) # mcore
add_subdirectory(mrender ${CMAKE_BINARY_DIR}/mrender) # mrender
target_link_libraries(
    ${PROJECT_NAME}
    mapp
    mcore
    mrender
)
set_target_properties(mapp PROPERTIES FOLDER "mapp ")
set_target_properties(mcore PROPERTIES FOLDER "mcore ")
set_target_properties(mrender PROPERTIES FOLDER "mrender ")
# =============================================================