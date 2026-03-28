include(GoogleTest)

add_executable(kbe_smoke_tests
  "${CMAKE_CURRENT_SOURCE_DIR}/smoke/test_fmt_bootstrap.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/smoke/test_tinyxml_bootstrap.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/smoke/test_zlib_bootstrap.cpp"
)

set_target_properties(kbe_smoke_tests PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests/bin"
)

target_link_libraries(kbe_smoke_tests
  PRIVATE
    GTest::gtest_main
    KBEngine::build_config
    KBEngine::third_party
)

gtest_discover_tests(kbe_smoke_tests)
