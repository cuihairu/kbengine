add_library(kbe_third_party INTERFACE)
add_library(KBEngine::third_party ALIAS kbe_third_party)

target_link_libraries(kbe_third_party INTERFACE
  fmt::fmt
  KBEngine::dependency_tmxparser
  KBEngine::tinyxml
  ZLIB::ZLIB
)
