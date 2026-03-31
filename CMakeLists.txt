cmake_minimum_required(VERSION 3.21)

project(KBEngine LANGUAGES C CXX)

include(CTest)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC AND NOT DEFINED CMAKE_MSVC_RUNTIME_LIBRARY)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

if(MSVC)
  add_compile_definitions(KBE_CMAKE_MANAGED_MSVC_RUNTIME=1)
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(KBE_USING_VCPKG OFF)
if(DEFINED CMAKE_TOOLCHAIN_FILE AND CMAKE_TOOLCHAIN_FILE MATCHES "vcpkg[\\\\/]scripts[\\\\/]buildsystems[\\\\/]vcpkg\\.cmake$")
  set(KBE_USING_VCPKG ON)
elseif(DEFINED VCPKG_TARGET_TRIPLET)
  set(KBE_USING_VCPKG ON)
endif()

include(KBEngineOptions)

if(KBE_USING_VCPKG)
  message(STATUS "KBEngine: vcpkg toolchain detected")
endif()

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

if(BUILD_TESTING AND KBE_ENABLE_TESTING)
  add_subdirectory(tests)
endif()
