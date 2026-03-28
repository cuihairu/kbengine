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

if(KBE_USE_OPENSSL)
  if(EXISTS "${KBE_SOURCE_DIR}/lib/dependencies/openssl/include")
    target_include_directories(kbe_build_config INTERFACE
      "${KBE_SOURCE_DIR}/lib/dependencies/openssl/include"
    )
  elseif(EXISTS "${KBE_SOURCE_DIR}/lib/dependencies/vsopenssl/include")
    target_include_directories(kbe_build_config INTERFACE
      "${KBE_SOURCE_DIR}/lib/dependencies/vsopenssl/include"
    )
  endif()
endif()

add_custom_target(kbe-bootstrap
  COMMENT "Initial KBEngine CMake bootstrap target"
)

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
