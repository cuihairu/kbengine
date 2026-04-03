if(KBE_USING_VCPKG)
  find_package(fmt CONFIG REQUIRED)
  find_package(hiredis CONFIG REQUIRED)
  find_package(tinyxml2 CONFIG REQUIRED)
  find_package(utf8cpp CONFIG REQUIRED)
  find_package(unofficial-libmysql CONFIG QUIET)
  find_package(unofficial-libmariadb CONFIG QUIET)
  if(KBE_USE_LOG4CXX)
    find_package(EXPAT REQUIRED)
    find_package(log4cxx CONFIG REQUIRED)
  endif()
endif()

if(TARGET utf8cpp::utf8cpp)
  add_library(kbe_dependency_utf8cpp INTERFACE)
  target_link_libraries(kbe_dependency_utf8cpp INTERFACE utf8cpp::utf8cpp)
  add_library(KBEngine::dependency_utf8cpp ALIAS kbe_dependency_utf8cpp)
  message(STATUS "KBEngine: using package-provided utf8cpp target")
elseif(KBE_USING_VCPKG)
  message(FATAL_ERROR "KBEngine: vcpkg mode requires utf8cpp::utf8cpp from the manifest-managed utfcpp package.")
else()
  add_library(kbe_dependency_utf8cpp INTERFACE)
  target_include_directories(kbe_dependency_utf8cpp
    INTERFACE
      "${KBE_SOURCE_DIR}/lib/dependencies"
  )
  add_library(KBEngine::dependency_utf8cpp ALIAS kbe_dependency_utf8cpp)
  message(STATUS "KBEngine: using vendored utf8cpp headers")
endif()

if(TARGET fmt::fmt)
  message(STATUS "KBEngine: using package-provided fmt target")
elseif(KBE_USING_VCPKG)
  message(FATAL_ERROR "KBEngine: vcpkg mode requires fmt::fmt from the manifest-managed fmt package.")
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

find_package(tinyxml2 CONFIG REQUIRED)

add_library(kbe_dependency_tinyxml INTERFACE)
add_library(KBEngine::tinyxml ALIAS kbe_dependency_tinyxml)
add_library(KBEngine::tinyxml2 ALIAS kbe_dependency_tinyxml)

target_link_libraries(kbe_dependency_tinyxml
  INTERFACE
    tinyxml2::tinyxml2
)

if(KBE_USE_LOG4CXX)
  set(KBE_LOG4CXX_IMPORTED_TARGET "")
  foreach(candidate
      log4cxx::log4cxx
      log4cxx
  )
    if(TARGET ${candidate})
      set(KBE_LOG4CXX_IMPORTED_TARGET ${candidate})
      break()
    endif()
  endforeach()

  if(KBE_LOG4CXX_IMPORTED_TARGET)
    add_library(kbe_dependency_log4cxx INTERFACE)
    target_link_libraries(kbe_dependency_log4cxx INTERFACE ${KBE_LOG4CXX_IMPORTED_TARGET})
    message(STATUS "KBEngine: using package-provided log4cxx target: ${KBE_LOG4CXX_IMPORTED_TARGET}")
  elseif(KBE_USING_VCPKG)
    message(FATAL_ERROR "KBEngine: vcpkg mode requires a package-provided log4cxx target.")
  else()
    message(FATAL_ERROR "KBEngine: KBE_USE_LOG4CXX=ON requires a discoverable log4cxx package target.")
  endif()

  add_library(KBEngine::dependency_log4cxx ALIAS kbe_dependency_log4cxx)
endif()

if(TARGET tmxparser::tmxparser)
  add_library(kbe_dependency_tmxparser INTERFACE)
  target_link_libraries(kbe_dependency_tmxparser INTERFACE tmxparser::tmxparser)
  message(STATUS "KBEngine: using package-provided tmxparser target")
else()
  add_library(kbe_dependency_tmxparser STATIC
    "${KBE_SOURCE_DIR}/lib/dependencies/tmxparser/base64.cpp"
    "${KBE_SOURCE_DIR}/lib/dependencies/tmxparser/TmxImage.cpp"
    "${KBE_SOURCE_DIR}/lib/dependencies/tmxparser/TmxImageLayer.cpp"
    "${KBE_SOURCE_DIR}/lib/dependencies/tmxparser/TmxLayer.cpp"
    "${KBE_SOURCE_DIR}/lib/dependencies/tmxparser/TmxEllipse.cpp"
    "${KBE_SOURCE_DIR}/lib/dependencies/tmxparser/TmxMap.cpp"
    "${KBE_SOURCE_DIR}/lib/dependencies/tmxparser/TmxObject.cpp"
    "${KBE_SOURCE_DIR}/lib/dependencies/tmxparser/TmxObjectGroup.cpp"
    "${KBE_SOURCE_DIR}/lib/dependencies/tmxparser/TmxPolygon.cpp"
    "${KBE_SOURCE_DIR}/lib/dependencies/tmxparser/TmxPolyline.cpp"
    "${KBE_SOURCE_DIR}/lib/dependencies/tmxparser/TmxPropertySet.cpp"
    "${KBE_SOURCE_DIR}/lib/dependencies/tmxparser/TmxTile.cpp"
    "${KBE_SOURCE_DIR}/lib/dependencies/tmxparser/TmxTileset.cpp"
    "${KBE_SOURCE_DIR}/lib/dependencies/tmxparser/TmxUtil.cpp"
  )

  target_compile_features(kbe_dependency_tmxparser PUBLIC cxx_std_17)
  target_include_directories(kbe_dependency_tmxparser
    PUBLIC
      "${KBE_SOURCE_DIR}/lib"
      "${KBE_SOURCE_DIR}/lib/dependencies/tmxparser"
      "${KBE_SOURCE_DIR}/lib/dependencies"
  )
  target_link_libraries(kbe_dependency_tmxparser
    PUBLIC
      KBEngine::tinyxml
      ZLIB::ZLIB
  )
  message(STATUS "KBEngine: using vendored tmxparser sources")
endif()

add_library(KBEngine::dependency_tmxparser ALIAS kbe_dependency_tmxparser)

if(TARGET hiredis::hiredis)
  add_library(kbe_dependency_hiredis INTERFACE)
  target_link_libraries(kbe_dependency_hiredis INTERFACE hiredis::hiredis)
  message(STATUS "KBEngine: using package-provided hiredis target")
elseif(TARGET hiredis::hiredis_static)
  add_library(kbe_dependency_hiredis INTERFACE)
  target_link_libraries(kbe_dependency_hiredis INTERFACE hiredis::hiredis_static)
  message(STATUS "KBEngine: using package-provided hiredis::hiredis_static target")
elseif(KBE_USING_VCPKG)
  message(FATAL_ERROR "KBEngine: vcpkg mode requires a package-provided hiredis target.")
else()
  add_library(kbe_dependency_hiredis STATIC
    "${KBE_SOURCE_DIR}/lib/dependencies/hiredis/hiredis.c"
    "${KBE_SOURCE_DIR}/lib/dependencies/hiredis/net.c"
    "${KBE_SOURCE_DIR}/lib/dependencies/hiredis/read.c"
    "${KBE_SOURCE_DIR}/lib/dependencies/hiredis/sds.c"
  )

  target_include_directories(kbe_dependency_hiredis
    PUBLIC
      "${KBE_SOURCE_DIR}/lib/dependencies/hiredis"
  )

  message(STATUS "KBEngine: using vendored hiredis sources")
endif()

add_library(KBEngine::dependency_hiredis ALIAS kbe_dependency_hiredis)

set(KBE_MYSQLCLIENT_IMPORTED_TARGET "")
foreach(candidate
    unofficial::libmysql::libmysql
    unofficial::libmariadb
    unofficial::libmariadb::libmariadb
)
  if(TARGET ${candidate})
    set(KBE_MYSQLCLIENT_IMPORTED_TARGET ${candidate})
    break()
  endif()
endforeach()

if(KBE_MYSQLCLIENT_IMPORTED_TARGET)
  add_library(kbe_dependency_mysqlclient INTERFACE)
  target_link_libraries(kbe_dependency_mysqlclient INTERFACE ${KBE_MYSQLCLIENT_IMPORTED_TARGET})
  message(STATUS "KBEngine: using package-provided MySQL client target: ${KBE_MYSQLCLIENT_IMPORTED_TARGET}")
elseif(KBE_USING_VCPKG)
  message(FATAL_ERROR "KBEngine: vcpkg mode requires libmariadb/libmysql from the manifest-managed package set.")
else()
  find_path(KBE_MYSQL_INCLUDE_DIR
    NAMES mysql/mysql.h mysql.h
    PATHS
      "${KBE_SOURCE_DIR}/lib/dependencies/mysql"
      /opt/homebrew/include
      /usr/local/include
    PATH_SUFFIXES ""
  )

  find_library(KBE_MYSQL_LIBRARY
    NAMES mariadb mysqlclient libmysql libmariadb
    PATHS
      /opt/homebrew/lib
      /usr/local/lib
  )

  if(KBE_MYSQL_INCLUDE_DIR AND KBE_MYSQL_LIBRARY)
    add_library(kbe_dependency_mysqlclient INTERFACE)
    target_include_directories(kbe_dependency_mysqlclient INTERFACE "${KBE_MYSQL_INCLUDE_DIR}")
    target_link_libraries(kbe_dependency_mysqlclient INTERFACE "${KBE_MYSQL_LIBRARY}")
    message(STATUS "KBEngine: using discovered MySQL client library: ${KBE_MYSQL_LIBRARY}")
  else()
    message(WARNING "KBEngine: MySQL client library was not found; dbmgr will not build until libmariadb/libmysql is available.")
  endif()
endif()

if(TARGET kbe_dependency_mysqlclient)
  add_library(KBEngine::dependency_mysqlclient ALIAS kbe_dependency_mysqlclient)
endif()

if(KBE_USE_JEMALLOC)
  find_path(KBE_JEMALLOC_INCLUDE_DIR
    NAMES jemalloc/jemalloc.h
  )

  find_library(KBE_JEMALLOC_LIBRARY
    NAMES jemalloc jemalloc_s
  )

  if(KBE_JEMALLOC_INCLUDE_DIR AND KBE_JEMALLOC_LIBRARY)
    add_library(kbe_dependency_jemalloc INTERFACE)
    target_include_directories(kbe_dependency_jemalloc INTERFACE "${KBE_JEMALLOC_INCLUDE_DIR}")
    target_link_libraries(kbe_dependency_jemalloc INTERFACE "${KBE_JEMALLOC_LIBRARY}")
    add_library(KBEngine::dependency_jemalloc ALIAS kbe_dependency_jemalloc)
    message(STATUS "KBEngine: using jemalloc library: ${KBE_JEMALLOC_LIBRARY}")
  elseif(KBE_USING_VCPKG)
    message(FATAL_ERROR "KBEngine: vcpkg mode requires manifest-managed jemalloc when KBE_USE_JEMALLOC=ON.")
  else()
    message(FATAL_ERROR "KBEngine: KBE_USE_JEMALLOC=ON requires jemalloc headers and library.")
  endif()
endif()

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

find_package(Python3 REQUIRED COMPONENTS Development)

if(TARGET Python3::Python)
  message(STATUS "Using system Python target: Python3::Python")

  if(WIN32)
    get_target_property(_kbe_python_implib_release Python3::Python IMPORTED_IMPLIB_RELEASE)
    get_target_property(_kbe_python_location_release Python3::Python IMPORTED_LOCATION_RELEASE)

    if((NOT _kbe_python_implib_release OR _kbe_python_implib_release STREQUAL "_kbe_python_implib_release-NOTFOUND") AND DEFINED Python3_LIBRARIES)
      list(GET Python3_LIBRARIES 0 _kbe_python_implib_release)
    endif()

    if(_kbe_python_implib_release AND EXISTS "${_kbe_python_implib_release}")
      message(STATUS "KBEngine: forcing Windows debug Python target to use release import library: ${_kbe_python_implib_release}")
      set_property(TARGET Python3::Python PROPERTY MAP_IMPORTED_CONFIG_DEBUG Release)
      set_property(TARGET Python3::Python PROPERTY IMPORTED_IMPLIB_DEBUG "${_kbe_python_implib_release}")

      if(_kbe_python_location_release AND EXISTS "${_kbe_python_location_release}")
        set_property(TARGET Python3::Python PROPERTY IMPORTED_LOCATION_DEBUG "${_kbe_python_location_release}")
      endif()
    endif()
  endif()
endif()

if(BUILD_TESTING AND KBE_ENABLE_TESTING)
  if(KBE_USING_VCPKG)
    find_package(GTest CONFIG REQUIRED)
  endif()

  if(TARGET GTest::gtest_main)
    message(STATUS "KBEngine: using package-provided GTest targets")
  elseif(KBE_USING_VCPKG)
    message(FATAL_ERROR "KBEngine: vcpkg mode requires GTest::gtest_main from the manifest-managed gtest package.")
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
