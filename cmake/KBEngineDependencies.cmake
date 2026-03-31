if(KBE_USING_VCPKG)
  find_package(fmt CONFIG QUIET)
  find_package(hiredis CONFIG QUIET)
  find_package(unofficial-libmysql CONFIG QUIET)
  find_package(unofficial-libmariadb CONFIG QUIET)
endif()

if(TARGET fmt::fmt)
  message(STATUS "KBEngine: using package-provided fmt target")
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

add_library(tinyxml STATIC
  "${KBE_SOURCE_DIR}/lib/dependencies/tinyxml/tinystr.cpp"
  "${KBE_SOURCE_DIR}/lib/dependencies/tinyxml/tinyxml.cpp"
  "${KBE_SOURCE_DIR}/lib/dependencies/tinyxml/tinyxmlerror.cpp"
  "${KBE_SOURCE_DIR}/lib/dependencies/tinyxml/tinyxmlparser.cpp"
)

add_library(KBEngine::tinyxml ALIAS tinyxml)

target_compile_features(tinyxml PUBLIC cxx_std_17)
target_include_directories(tinyxml
  PUBLIC
    "${KBE_SOURCE_DIR}/lib/dependencies/tinyxml"
)

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

add_library(KBEngine::dependency_tmxparser ALIAS kbe_dependency_tmxparser)

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

if(TARGET hiredis::hiredis)
  add_library(kbe_dependency_hiredis INTERFACE)
  target_link_libraries(kbe_dependency_hiredis INTERFACE hiredis::hiredis)
  message(STATUS "KBEngine: using package-provided hiredis target")
elseif(TARGET hiredis::hiredis_static)
  add_library(kbe_dependency_hiredis INTERFACE)
  target_link_libraries(kbe_dependency_hiredis INTERFACE hiredis::hiredis_static)
  message(STATUS "KBEngine: using package-provided hiredis::hiredis_static target")
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

add_library(kbe_dependency_jwsmtp STATIC
  "${KBE_SOURCE_DIR}/lib/dependencies/jwsmtp/jwsmtp/jwsmtp/base64.cpp"
  "${KBE_SOURCE_DIR}/lib/dependencies/jwsmtp/jwsmtp/jwsmtp/compat.cpp"
  "${KBE_SOURCE_DIR}/lib/dependencies/jwsmtp/jwsmtp/jwsmtp/mailer.cpp"
)

add_library(KBEngine::dependency_jwsmtp ALIAS kbe_dependency_jwsmtp)

target_compile_features(kbe_dependency_jwsmtp PUBLIC cxx_std_17)
target_include_directories(kbe_dependency_jwsmtp
  PUBLIC
    "${KBE_SOURCE_DIR}/lib/dependencies/jwsmtp/jwsmtp/jwsmtp"
)

if(WIN32)
  target_link_libraries(kbe_dependency_jwsmtp PUBLIC ws2_32)
endif()

if(BUILD_TESTING AND KBE_ENABLE_TESTING)
  if(KBE_USING_VCPKG)
    find_package(GTest CONFIG QUIET)
  endif()

  if(TARGET GTest::gtest_main)
    message(STATUS "KBEngine: using package-provided GTest targets")
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
