cmake_minimum_required(VERSION 3.21)

project(KBEngine LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(KBEngineOptions)

if(MSVC)
  add_compile_options(/utf-8)
endif()

set(KBE_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(KBE_SOURCE_DIR "${KBE_ROOT_DIR}/kbe/src")
set(KBE_LIB_OUTPUT_DIR "${KBE_SOURCE_DIR}/libs")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${KBE_LIB_OUTPUT_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${KBE_LIB_OUTPUT_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${KBE_ROOT_DIR}/kbe/bin/server")

include(KBEngineDependencies)

add_subdirectory(kbe/src)
