add_subdirectory(dependencies)

add_library(kbe_build_config INTERFACE)
add_library(KBEngine::build_config ALIAS kbe_build_config)

target_compile_features(kbe_build_config INTERFACE cxx_std_17)
target_compile_definitions(kbe_build_config INTERFACE ${KBE_COMMON_DEFINITIONS})
target_include_directories(kbe_build_config INTERFACE
  "${KBE_SOURCE_DIR}"
  "${KBE_SOURCE_DIR}/lib"
  "${KBE_SOURCE_DIR}/server"
  "${KBE_SOURCE_DIR}/lib/dependencies"
  "${KBE_SOURCE_DIR}/lib/dependencies/fmt/include"
  "${KBE_SOURCE_DIR}/lib/dependencies/tinyxml"
  "${KBE_SOURCE_DIR}/lib/dependencies/zlib"
  "${KBE_SOURCE_DIR}/lib/dependencies/curl/include"
)

if(KBE_USE_LOG4CXX)
  target_include_directories(kbe_build_config INTERFACE
    "${KBE_SOURCE_DIR}/lib/dependencies/log4cxx/src/main/include"
  )
endif()

add_custom_target(kbe-bootstrap
  COMMENT "Initial KBEngine CMake bootstrap target"
)

add_library(kbe_common_core STATIC
  "${KBE_SOURCE_DIR}/lib/common/base64.cpp"
  "${KBE_SOURCE_DIR}/lib/common/sha1.cpp"
)

add_library(KBEngine::common_core ALIAS kbe_common_core)

target_link_libraries(kbe_common_core
  PUBLIC
    KBEngine::build_config
)

target_include_directories(kbe_common_core
  PUBLIC
    "${KBE_SOURCE_DIR}/lib"
)

add_dependencies(kbe-bootstrap kbe_common_core)

add_library(kbe_common_strutil STATIC
  "${KBE_SOURCE_DIR}/lib/common/strutil_core.cpp"
)

add_library(KBEngine::common_strutil ALIAS kbe_common_strutil)

target_link_libraries(kbe_common_strutil
  PUBLIC
    KBEngine::build_config
)

target_include_directories(kbe_common_strutil
  PUBLIC
    "${KBE_SOURCE_DIR}/lib"
)

add_dependencies(kbe-bootstrap kbe_common_strutil)

add_library(kbe_common_timestamp STATIC
  "${KBE_SOURCE_DIR}/lib/common/timestamp_cmake.cpp"
)

add_library(KBEngine::common_timestamp ALIAS kbe_common_timestamp)

target_link_libraries(kbe_common_timestamp
  PUBLIC
    KBEngine::build_config
)

target_include_directories(kbe_common_timestamp
  PUBLIC
    "${KBE_SOURCE_DIR}/lib"
)

add_dependencies(kbe-bootstrap kbe_common_timestamp)

add_library(kbe_common_timer STATIC
  "${KBE_SOURCE_DIR}/lib/common/timer.cpp"
)

add_library(KBEngine::common_timer ALIAS kbe_common_timer)

target_link_libraries(kbe_common_timer
  PUBLIC
    KBEngine::build_config
)

target_include_directories(kbe_common_timer
  PUBLIC
    "${KBE_SOURCE_DIR}/lib"
)

add_dependencies(kbe-bootstrap kbe_common_timer)

add_library(kbe_common_md5 STATIC
  "${KBE_SOURCE_DIR}/lib/common/md5_cmake.cpp"
)

add_library(KBEngine::common_md5 ALIAS kbe_common_md5)

target_link_libraries(kbe_common_md5
  PUBLIC
    KBEngine::build_config
    OpenSSL::Crypto
)

target_compile_definitions(kbe_common_md5
  PUBLIC
    KBE_USE_EVP_MD5
)

target_include_directories(kbe_common_md5
  PUBLIC
    "${KBE_SOURCE_DIR}/lib"
)

add_dependencies(kbe-bootstrap kbe_common_md5)

add_library(kbe_common_objectpool INTERFACE)

add_library(KBEngine::common_objectpool ALIAS kbe_common_objectpool)

target_link_libraries(kbe_common_objectpool
  INTERFACE
    KBEngine::build_config
    KBEngine::common_timestamp
    fmt::fmt
)

target_include_directories(kbe_common_objectpool
  INTERFACE
    "${KBE_SOURCE_DIR}/lib"
)

add_dependencies(kbe-bootstrap kbe_common_objectpool)

add_library(kbe_common_memorystream STATIC
  "${KBE_SOURCE_DIR}/lib/common/memorystream.cpp"
)

add_library(KBEngine::common_memorystream ALIAS kbe_common_memorystream)

target_link_libraries(kbe_common_memorystream
  PUBLIC
    KBEngine::build_config
    KBEngine::common_objectpool
    fmt::fmt
)

target_compile_definitions(kbe_common_memorystream
  PUBLIC
    KBE_CMAKE_BOOTSTRAP_MEMORYSTREAM
)

target_include_directories(kbe_common_memorystream
  PUBLIC
    "${KBE_SOURCE_DIR}/lib"
)

add_dependencies(kbe-bootstrap kbe_common_memorystream)

add_library(kbe_common_ssl STATIC
  "${KBE_SOURCE_DIR}/lib/common/ssl.cpp"
)

add_library(KBEngine::common_ssl ALIAS kbe_common_ssl)

target_link_libraries(kbe_common_ssl
  PUBLIC
    KBEngine::build_config
    KBEngine::common_memorystream
    OpenSSL::SSL
    OpenSSL::Crypto
)

target_include_directories(kbe_common_ssl
  PUBLIC
    "${KBE_SOURCE_DIR}/lib"
)

add_dependencies(kbe-bootstrap kbe_common_ssl)

add_library(kbe_common_rsa STATIC
  "${KBE_SOURCE_DIR}/lib/common/rsa_cmake.cpp"
)

add_library(KBEngine::common_rsa ALIAS kbe_common_rsa)

target_link_libraries(kbe_common_rsa
  PUBLIC
    KBEngine::build_config
    KBEngine::common_strutil
    OpenSSL::Crypto
)

target_include_directories(kbe_common_rsa
  PUBLIC
    "${KBE_SOURCE_DIR}/lib"
)

add_dependencies(kbe-bootstrap kbe_common_rsa)

add_library(kbe_common_kbekey STATIC
  "${KBE_SOURCE_DIR}/lib/common/kbekey_cmake.cpp"
)

add_library(KBEngine::common_kbekey ALIAS kbe_common_kbekey)

target_link_libraries(kbe_common_kbekey
  PUBLIC
    KBEngine::build_config
    KBEngine::common_rsa
)

target_include_directories(kbe_common_kbekey
  PUBLIC
    "${KBE_SOURCE_DIR}/lib"
)

add_dependencies(kbe-bootstrap kbe_common_kbekey)

add_library(kbe_common_tasks STATIC
  "${KBE_SOURCE_DIR}/lib/common/tasks_cmake.cpp"
)

add_library(KBEngine::common_tasks ALIAS kbe_common_tasks)

target_link_libraries(kbe_common_tasks
  PUBLIC
    KBEngine::build_config
)

target_include_directories(kbe_common_tasks
  PUBLIC
    "${KBE_SOURCE_DIR}/lib"
)

add_dependencies(kbe-bootstrap kbe_common_tasks)

add_library(kbe_common_version STATIC
  "${KBE_SOURCE_DIR}/lib/common/kbeversion_cmake.cpp"
)

add_library(KBEngine::common_version ALIAS kbe_common_version)

target_link_libraries(kbe_common_version
  PUBLIC
    KBEngine::build_config
)

target_include_directories(kbe_common_version
  PUBLIC
    "${KBE_SOURCE_DIR}/lib"
)

add_dependencies(kbe-bootstrap kbe_common_version)

add_library(kbe_common_blowfish STATIC
  "${KBE_SOURCE_DIR}/lib/common/blowfish_cmake.cpp"
)

add_library(KBEngine::common_blowfish ALIAS kbe_common_blowfish)

target_link_libraries(kbe_common_blowfish
  PUBLIC
    KBEngine::build_config
    OpenSSL::Crypto
)

target_compile_definitions(kbe_common_blowfish
  PRIVATE
    OPENSSL_SUPPRESS_DEPRECATED
)

target_include_directories(kbe_common_blowfish
  PUBLIC
    "${KBE_SOURCE_DIR}/lib"
)

add_dependencies(kbe-bootstrap kbe_common_blowfish)

add_library(kbe_common_refcountable INTERFACE)

add_library(KBEngine::common_refcountable ALIAS kbe_common_refcountable)

target_link_libraries(kbe_common_refcountable
  INTERFACE
    KBEngine::build_config
)

target_include_directories(kbe_common_refcountable
  INTERFACE
    "${KBE_SOURCE_DIR}/lib"
)

add_dependencies(kbe-bootstrap kbe_common_refcountable)

add_library(kbe_common_smartpointer INTERFACE)

add_library(KBEngine::common_smartpointer ALIAS kbe_common_smartpointer)

target_link_libraries(kbe_common_smartpointer
  INTERFACE
    KBEngine::build_config
    KBEngine::common_refcountable
)

target_include_directories(kbe_common_smartpointer
  INTERFACE
    "${KBE_SOURCE_DIR}/lib"
)

add_dependencies(kbe-bootstrap kbe_common_smartpointer)

add_library(kbe_thread_primitives INTERFACE)

add_library(KBEngine::thread_primitives ALIAS kbe_thread_primitives)

target_link_libraries(kbe_thread_primitives
  INTERFACE
    KBEngine::build_config
)

target_include_directories(kbe_thread_primitives
  INTERFACE
    "${KBE_SOURCE_DIR}/lib"
)

add_dependencies(kbe-bootstrap kbe_thread_primitives)

add_library(kbe_thread_concurrency STATIC
  "${KBE_SOURCE_DIR}/lib/thread/concurrency.cpp"
)

add_library(KBEngine::thread_concurrency ALIAS kbe_thread_concurrency)

target_link_libraries(kbe_thread_concurrency
  PUBLIC
    KBEngine::build_config
)

target_include_directories(kbe_thread_concurrency
  PUBLIC
    "${KBE_SOURCE_DIR}/lib"
)

add_dependencies(kbe-bootstrap kbe_thread_concurrency)

add_library(kbe_thread_task INTERFACE)

add_library(KBEngine::thread_task ALIAS kbe_thread_task)

target_link_libraries(kbe_thread_task
  INTERFACE
    KBEngine::build_config
)

target_include_directories(kbe_thread_task
  INTERFACE
    "${KBE_SOURCE_DIR}/lib"
)

add_dependencies(kbe-bootstrap kbe_thread_task)

add_library(kbe_common_deadline INTERFACE)

add_library(KBEngine::common_deadline ALIAS kbe_common_deadline)

target_link_libraries(kbe_common_deadline
  INTERFACE
    KBEngine::build_config
)

target_include_directories(kbe_common_deadline
  INTERFACE
    "${KBE_SOURCE_DIR}/lib"
)

add_dependencies(kbe-bootstrap kbe_common_deadline)

add_library(kbe_common_predicates INTERFACE)

add_library(KBEngine::common_predicates ALIAS kbe_common_predicates)

target_link_libraries(kbe_common_predicates
  INTERFACE
    KBEngine::build_config
)

target_include_directories(kbe_common_predicates
  INTERFACE
    "${KBE_SOURCE_DIR}/lib"
)

add_dependencies(kbe-bootstrap kbe_common_predicates)

add_library(kbe_common_endian INTERFACE)

add_library(KBEngine::common_endian ALIAS kbe_common_endian)

target_link_libraries(kbe_common_endian
  INTERFACE
    KBEngine::build_config
)

target_include_directories(kbe_common_endian
  INTERFACE
    "${KBE_SOURCE_DIR}/lib"
)

add_dependencies(kbe-bootstrap kbe_common_endian)

add_library(kbe_thread_pool STATIC
  "${KBE_SOURCE_DIR}/lib/thread/threadpool.cpp"
)

add_library(KBEngine::thread_pool ALIAS kbe_thread_pool)

target_link_libraries(kbe_thread_pool
  PUBLIC
    KBEngine::build_config
    KBEngine::common_tasks
    KBEngine::thread_task
    KBEngine::thread_primitives
)

target_compile_definitions(kbe_thread_pool
  PUBLIC
    KBE_CMAKE_BOOTSTRAP_THREADPOOL
)

target_include_directories(kbe_thread_pool
  PUBLIC
    "${KBE_SOURCE_DIR}/lib"
)

add_dependencies(kbe-bootstrap kbe_thread_pool)

add_library(kbe_helper_watcher STATIC
  "${KBE_SOURCE_DIR}/lib/helper/watcher.cpp"
)

add_library(KBEngine::helper_watcher ALIAS kbe_helper_watcher)

target_link_libraries(kbe_helper_watcher
  PUBLIC
    KBEngine::build_config
    KBEngine::common_strutil
    KBEngine::common_memorystream
    KBEngine::common_smartpointer
)

target_compile_definitions(kbe_helper_watcher
  PUBLIC
    KBE_CMAKE_BOOTSTRAP_WATCHER
)

target_include_directories(kbe_helper_watcher
  PUBLIC
    "${KBE_SOURCE_DIR}/lib"
)

add_dependencies(kbe-bootstrap kbe_helper_watcher)

add_library(kbe_helper_profile STATIC
  "${KBE_SOURCE_DIR}/lib/helper/profile.cpp"
)

add_library(KBEngine::helper_profile ALIAS kbe_helper_profile)

target_link_libraries(kbe_helper_profile
  PUBLIC
    KBEngine::build_config
  PRIVATE
    KBEngine::common_timer
    KBEngine::common_timestamp
    KBEngine::helper_watcher
)

target_compile_definitions(kbe_helper_profile
  PUBLIC
    KBE_CMAKE_BOOTSTRAP_PROFILE
)

target_include_directories(kbe_helper_profile
  PUBLIC
    "${KBE_SOURCE_DIR}/lib"
)

add_dependencies(kbe-bootstrap kbe_helper_profile)

add_library(kbe_helper_debug_option STATIC
  "${KBE_SOURCE_DIR}/lib/helper/debug_option.cpp"
)

add_library(KBEngine::helper_debug_option ALIAS kbe_helper_debug_option)

target_link_libraries(kbe_helper_debug_option
  PUBLIC
    KBEngine::build_config
)

target_include_directories(kbe_helper_debug_option
  PUBLIC
    "${KBE_SOURCE_DIR}/lib"
)

add_dependencies(kbe-bootstrap kbe_helper_debug_option)

add_library(kbe_helper_event_stats STATIC
  "${KBE_SOURCE_DIR}/lib/helper/eventhistory_stats.cpp"
)

add_library(KBEngine::helper_event_stats ALIAS kbe_helper_event_stats)

target_link_libraries(kbe_helper_event_stats
  PUBLIC
    KBEngine::build_config
)

target_compile_definitions(kbe_helper_event_stats
  PUBLIC
    KBE_CMAKE_BOOTSTRAP_EVENT_STATS
)

target_include_directories(kbe_helper_event_stats
  PUBLIC
    "${KBE_SOURCE_DIR}/lib"
)

add_dependencies(kbe-bootstrap kbe_helper_event_stats)

if(KBE_BUILD_LEGACY_XML_TARGET)
  add_library(kbe_xml STATIC
    "${KBE_SOURCE_DIR}/lib/xml/xml.cpp"
  )

  add_library(KBEngine::xml ALIAS kbe_xml)

  target_link_libraries(kbe_xml
    PUBLIC
      KBEngine::build_config
      KBEngine::tinyxml
      fmt::fmt
  )

  target_include_directories(kbe_xml
    PUBLIC
      "${KBE_SOURCE_DIR}/lib"
  )

  add_dependencies(kbe-bootstrap kbe_xml)
endif()

if(NOT KBE_BUILD_BOOTSTRAP_ONLY)
  message(STATUS "Full KBEngine CMake targets are not migrated yet; bootstrap mode is recommended.")
endif()
