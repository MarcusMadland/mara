# Project Info
cmake_minimum_required(VERSION 3.18 FATAL_ERROR)
cmake_policy(VERSION 3.18)
project(asset-manager)
enable_language(C)
enable_language(CXX)

# Makefile generators on apple need this flag to compile mixed objective/c++
if(APPLE AND NOT XCODE)
	set(CMAKE_CXX_FLAGS "-ObjC++")
endif()

if(MSVC AND (MSVC_VERSION GREATER_EQUAL 1914))
	add_compile_options("/Zc:__cplusplus")
endif()

# CMake Settings
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_SUPPRESS_REGENERATION true)
set(DCMAKE_GENERATOR_PLATFORM "x64")
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin)
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin)
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)
if(NOT CMAKE_DEBUG_POSTFIX)
  set(CMAKE_DEBUG_POSTFIX d)
endif()

# Sources
file(GLOB_RECURSE FILE_SOURCES RELATIVE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h
)

# Dependencies ================================================
add_subdirectory(../../ ${CMAKE_BINARY_DIR}/mengine) # mengine
# =============================================================

# FBX SDK
IF(WIN32)
    set(FBX_ROOT "C:/Program Files/Autodesk/FBX/FBX SDK/2020.0.1")

    set(INCLUDE_DIR
        ${INCLUDE_DIR}
        ${FBX_ROOT}/include
    )

    set(LINK_LIBS
        ${FBX_ROOT}/lib/vs2017/x64/debug/libfbxsdk.dll
    )

#ENDIF(WIN32)
ELSEIF(APPLE)
    set(FBX_ROOT /Applications/Autodesk/FBX\ SDK/2013.3)

    set(INCLUDE_DIR
        ${INCLUDE_DIR}
        ${FBX_ROOT}/include
    )

    set(LINK_LIBS
        ${FBX_ROOT}/lib/gcc4/ub/libfbxsdk-2013.3-static.a
        /System/Library/Frameworks/Carbon.framework
    )
ENDIF(WIN32)
#

# Add engine executable
add_executable(
    ${PROJECT_NAME}
    "${FILE_SOURCES}"
)

# Link to engine
target_link_libraries(
    ${PROJECT_NAME}
    mengine
    ${LINK_LIBS}
)

target_include_directories(
    ${PROJECT_NAME} 
    PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include/
    ${INCLUDE_DIR}
)

# Change output dir to bin
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
# Change working directory to bin
if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
endif()