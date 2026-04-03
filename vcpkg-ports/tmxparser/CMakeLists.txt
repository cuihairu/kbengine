cmake_minimum_required(VERSION 3.20)

project(tmxparser LANGUAGES CXX)

include(CMakePackageConfigHelpers)

find_package(tinyxml2 CONFIG REQUIRED)
find_package(ZLIB REQUIRED)

add_library(tmxparser STATIC
  TmxEllipse.cpp
  TmxImage.cpp
  TmxImageLayer.cpp
  TmxLayer.cpp
  TmxMap.cpp
  TmxObject.cpp
  TmxObjectGroup.cpp
  TmxPolygon.cpp
  TmxPolyline.cpp
  TmxPropertySet.cpp
  TmxTile.cpp
  TmxTileset.cpp
  TmxUtil.cpp
  base64.cpp
)

add_library(tmxparser::tmxparser ALIAS tmxparser)

target_compile_features(tmxparser PUBLIC cxx_std_17)
target_include_directories(tmxparser
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
    $<INSTALL_INTERFACE:include/tmxparser>
)
target_link_libraries(tmxparser
  PUBLIC
    tinyxml2::tinyxml2
    ZLIB::ZLIB
)

install(
  TARGETS tmxparser
  EXPORT tmxparserTargets
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

install(
  FILES
    Tmx.h
    TmxEllipse.h
    TmxImage.h
    TmxImageLayer.h
    TmxLayer.h
    TmxMap.h
    TmxMapTile.h
    TmxObject.h
    TmxObjectGroup.h
    TmxPoint.h
    TmxPolygon.h
    TmxPolyline.h
    TmxPropertySet.h
    TmxTile.h
    TmxTileset.h
    TmxUtil.h
    base64.h
  DESTINATION include/tmxparser
)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/tmxparserConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/tmxparserConfig.cmake"
  INSTALL_DESTINATION share/tmxparser
)

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/tmxparserConfigVersion.cmake"
  VERSION 0.0.0
  COMPATIBILITY AnyNewerVersion
)

install(
  EXPORT tmxparserTargets
  NAMESPACE tmxparser::
  DESTINATION share/tmxparser
)

install(
  FILES
    "${CMAKE_CURRENT_BINARY_DIR}/tmxparserConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/tmxparserConfigVersion.cmake"
  DESTINATION share/tmxparser
)
