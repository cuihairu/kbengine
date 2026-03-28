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
