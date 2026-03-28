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

if(BUILD_TESTING AND KBE_ENABLE_TESTING)
  include(FetchContent)

  set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
  )

  FetchContent_MakeAvailable(googletest)
endif()
