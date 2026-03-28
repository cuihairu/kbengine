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
  "${CMAKE_CURRENT_SOURCE_DIR}/smoke/test_common_memorystream.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/smoke/test_common_ssl.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/smoke/test_common_rsa.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/smoke/test_common_kbekey.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/smoke/test_common_tasks.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/smoke/test_common_version.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/smoke/test_common_blowfish.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/smoke/test_common_smartpointer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/smoke/test_thread_primitives.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/smoke/test_thread_concurrency.cpp"
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
    KBEngine::common_timer
    KBEngine::common_md5
    KBEngine::common_ssl
    KBEngine::common_rsa
    KBEngine::common_kbekey
    KBEngine::common_tasks
    KBEngine::common_version
    KBEngine::common_blowfish
    KBEngine::common_smartpointer
    KBEngine::thread_primitives
    KBEngine::thread_concurrency
)

gtest_discover_tests(kbe_smoke_tests)
