include(GoogleTest)

add_executable(kbe_smoke_tests
  "${CMAKE_CURRENT_SOURCE_DIR}/smoke/test_fmt_bootstrap.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/smoke/test_tinyxml_bootstrap.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/smoke/test_zlib_bootstrap.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/smoke/test_common_base64_sha1.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/smoke/test_common_strutil.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/smoke/test_common_timestamp.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/smoke/test_common_timer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/smoke/test_common_md5.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/smoke/test_common_objectpool.cpp"
)

set_target_properties(kbe_smoke_tests PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests/bin"
)

target_link_libraries(kbe_smoke_tests
  PRIVATE
    GTest::gtest_main
    KBEngine::build_config
    KBEngine::third_party
    KBEngine::common_core
    KBEngine::common_strutil
    KBEngine::common_timestamp
    KBEngine::common_timer
    KBEngine::common_md5
)

gtest_discover_tests(kbe_smoke_tests)
