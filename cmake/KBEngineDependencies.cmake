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
