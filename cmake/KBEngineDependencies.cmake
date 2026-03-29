if(KBE_USING_VCPKG)
  find_package(fmt CONFIG QUIET)
endif()

if(TARGET fmt::fmt)
  message(STATUS "KBEngine: using package-provided fmt target")
else()
  add_library(fmt STATIC
    "${KBE_SOURCE_DIR}/lib/dependencies/fmt/src/format.cc"
    "${KBE_SOURCE_DIR}/lib/dependencies/fmt/src/os.cc"
  )

  add_library(fmt::fmt ALIAS fmt)

  target_compile_features(fmt PUBLIC cxx_std_17)
  target_include_directories(fmt
    PUBLIC
      "${KBE_SOURCE_DIR}/lib/dependencies/fmt/include"
  )

  message(STATUS "KBEngine: using vendored fmt sources")
endif()

add_library(tinyxml STATIC
  "${KBE_SOURCE_DIR}/lib/dependencies/tinyxml/tinystr.cpp"
  "${KBE_SOURCE_DIR}/lib/dependencies/tinyxml/tinyxml.cpp"
  "${KBE_SOURCE_DIR}/lib/dependencies/tinyxml/tinyxmlerror.cpp"
  "${KBE_SOURCE_DIR}/lib/dependencies/tinyxml/tinyxmlparser.cpp"
)

add_library(KBEngine::tinyxml ALIAS tinyxml)

target_compile_features(tinyxml PUBLIC cxx_std_17)
target_include_directories(tinyxml
  PUBLIC
    "${KBE_SOURCE_DIR}/lib/dependencies/tinyxml"
)

find_package(ZLIB REQUIRED)

if(TARGET ZLIB::ZLIB)
  message(STATUS "Using system zlib target: ZLIB::ZLIB")
endif()

find_package(CURL REQUIRED)

if(TARGET CURL::libcurl)
  message(STATUS "Using system CURL target: CURL::libcurl")
endif()

if(KBE_USE_OPENSSL)
  find_package(OpenSSL REQUIRED COMPONENTS Crypto SSL)

  if(TARGET OpenSSL::Crypto)
    message(STATUS "Using system OpenSSL targets: OpenSSL::Crypto/OpenSSL::SSL")
  endif()
endif()

if(BUILD_TESTING AND KBE_ENABLE_TESTING)
  if(KBE_USING_VCPKG)
    find_package(GTest CONFIG QUIET)
  endif()

  if(TARGET GTest::gtest_main)
    message(STATUS "KBEngine: using package-provided GTest targets")
  else()
    include(FetchContent)

    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

    FetchContent_Declare(
      googletest
      URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.tar.gz
      DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )

    FetchContent_MakeAvailable(googletest)
    message(STATUS "KBEngine: using FetchContent googletest fallback")
  endif()
endif()
