cmake_minimum_required(VERSION 3.8)

option(SUPERBUILD "Perform a superbuild (or not)" OFF)

project(my-application LANGUAGES CXX)



if (SUPERBUILD)
  if (EMSCRIPTEN)
    set(THIRD_PARTY_BUILD_DIR_NAME build-em)
  else ()
    set(THIRD_PARTY_BUILD_DIR_NAME build)
  endif ()
  include(third-party/CMakeLists.txt)
  include(superbuild.cmake)
  return()
endif ()

find_package(SDL2 REQUIRED CONFIG CMAKE_FIND_ROOT_PATH_BOTH)
find_package(bgfx REQUIRED CONFIG CMAKE_FIND_ROOT_PATH_BOTH)
find_package(imgui.cmake REQUIRED CONFIG CMAKE_FIND_ROOT_PATH_BOTH)

file(GLOB_RECURSE source_list RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
	"source/**.cpp" 
	"source/**.hpp"
	"source/**.h"
    "mapp/**.cpp" 
    "mapp/**.hpp"
	"mapp/**.h"
    "mcore/**.cpp" 
    "mcore/**.hpp"
    "mcore/**.h"
    "mrender/**.cpp" 
    "mrender/**.hpp"
    "mrender/**.h")
add_executable(${PROJECT_NAME} ${source_list})
target_include_directories(${PROJECT_NAME} PUBLIC 
	source/include/ 
	mapp/include/ 
	mcore/include/ 
	mrender/include/)

 if (true)
foreach(source IN LISTS source_list)
    get_filename_component(source_path "${source}" PATH)
    string(REPLACE "/" "\\" source_path_msvc "${source_path}")
    string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "" source_path_msvc_clean "${source_path_msvc}")
    source_group("${source_path_msvc_clean}" FILES "${source}")
endforeach()
endif()



target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_11)
target_link_libraries(
  ${PROJECT_NAME} PRIVATE SDL2::SDL2-static SDL2::SDL2main bgfx::bgfx bgfx::bx
                          imgui.cmake::imgui.cmake)
target_link_options(
  ${PROJECT_NAME} PRIVATE $<$<BOOL:${EMSCRIPTEN}>:-sMAX_WEBGL_VERSION=2
  -sALLOW_MEMORY_GROWTH=1 --preload-file=shader/build-em/v_simple.bin
  --preload-file=shader/build-em/f_simple.bin>)
target_compile_definitions(
  ${PROJECT_NAME} PRIVATE $<$<BOOL:${EMSCRIPTEN}>:USE_SDL=2>)

if (EMSCRIPTEN)
  set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".html")
  add_custom_command(
    TARGET ${PROJECT_NAME}
    PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
            $<TARGET_FILE_DIR:${PROJECT_NAME}>/shader/build-em
    COMMAND
      ${CMAKE_COMMAND} -E copy_if_different
      ${CMAKE_CURRENT_SOURCE_DIR}/shader/build-em/v_simple.bin
      $<TARGET_FILE_DIR:${PROJECT_NAME}>/shader/build-em
    COMMAND
      ${CMAKE_COMMAND} -E copy_if_different
      ${CMAKE_CURRENT_SOURCE_DIR}/shader/build-em/f_simple.bin
      $<TARGET_FILE_DIR:${PROJECT_NAME}>/shader/build-em
    VERBATIM)
endif ()

set_target_properties(
  ${PROJECT_NAME}
  # required for project when using visual studio generator
  PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

if (WIN32)
  # copy the SDL2.dll to the same folder as the executable
  add_custom_command(
    TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:SDL2::SDL2>
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
    VERBATIM)
endif ()