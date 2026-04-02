cmake_minimum_required(VERSION 3.20)

project(jwsmtp LANGUAGES CXX)

include(CMakePackageConfigHelpers)

add_library(jwsmtp STATIC
  base64.cpp
  compat.cpp
  mailer.cpp
)

add_library(jwsmtp::jwsmtp ALIAS jwsmtp)

target_compile_features(jwsmtp PUBLIC cxx_std_17)
target_include_directories(jwsmtp
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

if(WIN32)
  target_link_libraries(jwsmtp PUBLIC ws2_32)
endif()

install(
  TARGETS jwsmtp
  EXPORT jwsmtpTargets
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

install(
  FILES
    base64.h
    compat.h
    jwsmtp.h
    mailer.h
  DESTINATION include
)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/jwsmtpConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/jwsmtpConfig.cmake"
  INSTALL_DESTINATION share/jwsmtp
)

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/jwsmtpConfigVersion.cmake"
  VERSION 1.32.0
  COMPATIBILITY AnyNewerVersion
)

install(
  EXPORT jwsmtpTargets
  NAMESPACE jwsmtp::
  DESTINATION share/jwsmtp
)

install(
  FILES
    "${CMAKE_CURRENT_BINARY_DIR}/jwsmtpConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/jwsmtpConfigVersion.cmake"
  DESTINATION share/jwsmtp
)
