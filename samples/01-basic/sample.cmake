cmake_minimum_required(VERSION 3.18 FATAL_ERROR)

# Project Info
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
# Set the source and binary directories explicitly
set(SAMPLE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/samples/01-basic)
set(SAMPLE_BINARY_DIR ${CMAKE_BINARY_DIR}/samples/01-basic)

# Specify the source files for your sample
#file(GLOB_RECURSE SAMPLE_SOURCE_FILES RELATIVE
#    ${SAMPLE_SOURCE_DIR}
#    ${SAMPLE_SOURCE_DIR}/src/*.cpp
#    ${SAMPLE_SOURCE_DIR}/include/*.h
#)
set(SAMPLE_SOURCE_FILES ${SAMPLE_SOURCE_DIR}/src/main.cpp)

# Add executable for your sample
add_executable(
    01-basic
    "${SAMPLE_SOURCE_FILES}"
)

# Link to the engine
target_link_libraries(
    01-basic
    mara
)

target_include_directories(01-basic PUBLIC ${SAMPLE_SOURCE_DIR}/include/)


# Change output directory for executable
set_target_properties(01-basic PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${SAMPLE_BINARY_DIR}/bin
)

# Change output directory for libraries (if needed)
set_target_properties(01-basic PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${SAMPLE_BINARY_DIR}/lib
)


# Change working directory to bin (for MSVC)
if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    set_property(TARGET 01-basic PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${SAMPLE_BINARY_DIR})
endif()
