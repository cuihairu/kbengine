add_subdirectory(dependencies)

add_library(kbe_build_config INTERFACE)
add_library(KBEngine::build_config ALIAS kbe_build_config)

target_compile_features(kbe_build_config INTERFACE cxx_std_17)
target_link_libraries(kbe_build_config INTERFACE
  fmt::fmt
  KBEngine::dependency_utf8cpp
)
target_compile_definitions(kbe_build_config INTERFACE ${KBE_COMMON_DEFINITIONS})
target_include_directories(kbe_build_config INTERFACE
  "${KBE_SOURCE_DIR}"
  "${KBE_SOURCE_DIR}/lib"
)

if(WIN32)
  target_link_libraries(kbe_build_config INTERFACE ws2_32)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  target_compile_options(kbe_build_config INTERFACE
    -Wno-undefined-var-template
  )
endif()

if(KBE_USE_LOG4CXX)
  target_link_libraries(kbe_build_config INTERFACE KBEngine::dependency_log4cxx)
endif()

if(KBE_USE_JEMALLOC)
  target_link_libraries(kbe_build_config INTERFACE KBEngine::dependency_jemalloc)
endif()

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

add_library(kbe_common_timestamp STATIC
  "${KBE_SOURCE_DIR}/lib/common/timestamp.cpp"
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

add_library(kbe_common_md5 STATIC
  "${KBE_SOURCE_DIR}/lib/common/md5.cpp"
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

target_include_directories(kbe_common_memorystream
  PUBLIC
    "${KBE_SOURCE_DIR}/lib"
)

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

add_library(kbe_common_rsa STATIC
  "${KBE_SOURCE_DIR}/lib/common/rsa.cpp"
)

add_library(KBEngine::common_rsa ALIAS kbe_common_rsa)

target_link_libraries(kbe_common_rsa
  PUBLIC
    KBEngine::build_config
    KBEngine::common_strutil
    KBEngine::helper_debug_helper
    OpenSSL::Crypto
)

target_include_directories(kbe_common_rsa
  PUBLIC
    "${KBE_SOURCE_DIR}/lib"
)

add_library(kbe_common_kbekey STATIC
  "${KBE_SOURCE_DIR}/lib/common/kbekey.cpp"
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

add_library(kbe_common_tasks STATIC
  "${KBE_SOURCE_DIR}/lib/common/tasks.cpp"
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

add_library(kbe_common_version STATIC
  "${KBE_SOURCE_DIR}/lib/common/kbeversion.cpp"
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

add_library(kbe_common_blowfish STATIC
  "${KBE_SOURCE_DIR}/lib/common/blowfish.cpp"
)

add_library(KBEngine::common_blowfish ALIAS kbe_common_blowfish)

target_link_libraries(kbe_common_blowfish
  PUBLIC
    KBEngine::build_config
    KBEngine::helper_debug_helper
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

target_include_directories(kbe_thread_pool
  PUBLIC
    "${KBE_SOURCE_DIR}/lib"
)

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

target_include_directories(kbe_helper_watcher
  PUBLIC
    "${KBE_SOURCE_DIR}/lib"
)

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

target_include_directories(kbe_helper_profile
  PUBLIC
    "${KBE_SOURCE_DIR}/lib"
)

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

add_library(kbe_helper_event_stats STATIC
  "${KBE_SOURCE_DIR}/lib/helper/eventhistory_stats.cpp"
)

add_library(KBEngine::helper_event_stats ALIAS kbe_helper_event_stats)

target_link_libraries(kbe_helper_event_stats
  PUBLIC
    KBEngine::build_config
)

target_include_directories(kbe_helper_event_stats
  PUBLIC
    "${KBE_SOURCE_DIR}/lib"
)

add_library(kbe_helper_memory INTERFACE)

add_library(KBEngine::helper_memory ALIAS kbe_helper_memory)

target_link_libraries(kbe_helper_memory
  INTERFACE
    KBEngine::build_config
)

target_include_directories(kbe_helper_memory
  INTERFACE
    "${KBE_SOURCE_DIR}/lib"
)

add_library(kbe_helper_profiler STATIC
  "${KBE_SOURCE_DIR}/lib/helper/profiler.cpp"
)

add_library(KBEngine::helper_profiler ALIAS kbe_helper_profiler)

target_link_libraries(kbe_helper_profiler
  PUBLIC
    KBEngine::build_config
)

target_include_directories(kbe_helper_profiler
  PUBLIC
    "${KBE_SOURCE_DIR}/lib"
)

add_library(kbe_helper_debug_helper STATIC
  "${KBE_SOURCE_DIR}/lib/helper/debug_helper.cpp"
  "${KBE_SOURCE_DIR}/lib/helper/script_loglevel.cpp"
)

if(WIN32)
  target_sources(kbe_helper_debug_helper
    PRIVATE
      "${KBE_SOURCE_DIR}/lib/helper/crashhandler.cpp"
  )
endif()

add_library(KBEngine::helper_debug_helper ALIAS kbe_helper_debug_helper)

target_link_libraries(kbe_helper_debug_helper
  PUBLIC
    KBEngine::build_config
  PRIVATE
    KBEngine::network_common
    KBEngine::network_address
    KBEngine::thread_primitives
    KBEngine::common_objectpool
    KBEngine::common_memorystream
)

target_include_directories(kbe_helper_debug_helper
  PUBLIC
    "${KBE_SOURCE_DIR}/lib"
  PRIVATE
    "${KBE_SOURCE_DIR}/server"
)

add_library(kbe_network_http_utility STATIC
  "${KBE_SOURCE_DIR}/lib/network/http_utility.cpp"
)

add_library(KBEngine::network_http_utility ALIAS kbe_network_http_utility)

target_link_libraries(kbe_network_http_utility
  PUBLIC
    KBEngine::build_config
  PRIVATE
    KBEngine::common_memorystream
    KBEngine::helper_debug_helper
    CURL::libcurl
)

target_include_directories(kbe_network_http_utility
  PUBLIC
    "${KBE_SOURCE_DIR}/lib"
)

add_library(kbe_network_common STATIC
  "${KBE_SOURCE_DIR}/lib/network/common.cpp"
)

add_library(KBEngine::network_common ALIAS kbe_network_common)

target_link_libraries(kbe_network_common
  PUBLIC
    KBEngine::build_config
  PRIVATE
    KBEngine::common_ssl
    KBEngine::network_http_utility
    KBEngine::network_address
    KBEngine::network_endpoint
    KBEngine::network_tcp_packet
    KBEngine::network_udp_packet
    KBEngine::helper_debug_option
)

target_include_directories(kbe_network_common
  PUBLIC
    "${KBE_SOURCE_DIR}/lib"
)

add_library(kbe_network_address STATIC
  "${KBE_SOURCE_DIR}/lib/network/address.cpp"
)

add_library(KBEngine::network_address ALIAS kbe_network_address)

target_link_libraries(kbe_network_address
  PUBLIC
    KBEngine::build_config
  PRIVATE
    KBEngine::common_objectpool
    KBEngine::network_common
    KBEngine::helper_debug_helper
)

target_include_directories(kbe_network_address
  PUBLIC
    "${KBE_SOURCE_DIR}/lib"
)

add_library(kbe_network_exception INTERFACE)

add_library(KBEngine::network_exception ALIAS kbe_network_exception)

target_link_libraries(kbe_network_exception
  INTERFACE
    KBEngine::build_config
    KBEngine::common_timer
    KBEngine::network_common
    KBEngine::network_address
)

target_include_directories(kbe_network_exception
  INTERFACE
    "${KBE_SOURCE_DIR}/lib"
)

add_library(kbe_network_interfaces INTERFACE)

add_library(KBEngine::network_interfaces ALIAS kbe_network_interfaces)

target_link_libraries(kbe_network_interfaces
  INTERFACE
    KBEngine::build_config
)

target_include_directories(kbe_network_interfaces
  INTERFACE
    "${KBE_SOURCE_DIR}/lib"
)

add_library(kbe_network_packet INTERFACE)

add_library(KBEngine::network_packet ALIAS kbe_network_packet)

target_link_libraries(kbe_network_packet
  INTERFACE
    KBEngine::build_config
    KBEngine::common_memorystream
    KBEngine::common_smartpointer
    KBEngine::network_common
)

target_include_directories(kbe_network_packet
  INTERFACE
    "${KBE_SOURCE_DIR}/lib"
)

add_library(kbe_network_message_handler STATIC
  "${KBE_SOURCE_DIR}/lib/network/message_handler.cpp"
)

add_library(KBEngine::network_message_handler ALIAS kbe_network_message_handler)

target_link_libraries(kbe_network_message_handler
  PUBLIC
    KBEngine::build_config
  PRIVATE
    KBEngine::common_md5
    KBEngine::common_memorystream
    KBEngine::network_common
    KBEngine::network_channel
    KBEngine::network_packet_receiver
    KBEngine::network_fixed_messages
    KBEngine::helper_watcher
    KBEngine::xml
    KBEngine::resmgr
)

target_include_directories(kbe_network_message_handler
  PUBLIC
    "${KBE_SOURCE_DIR}/lib"
)

add_library(kbe_network_stats STATIC
  "${KBE_SOURCE_DIR}/lib/network/network_stats.cpp"
)

add_library(KBEngine::network_stats ALIAS kbe_network_stats)

target_link_libraries(kbe_network_stats
  PUBLIC
    KBEngine::build_config
  PRIVATE
    KBEngine::network_interfaces
    KBEngine::network_message_handler
)

target_include_directories(kbe_network_stats
  PUBLIC
    "${KBE_SOURCE_DIR}/lib"
)

add_library(kbe_network_error_reporter STATIC
  "${KBE_SOURCE_DIR}/lib/network/error_reporter.cpp"
)

add_library(KBEngine::network_error_reporter ALIAS kbe_network_error_reporter)

target_link_libraries(kbe_network_error_reporter
  PUBLIC
    KBEngine::build_config
  PRIVATE
    KBEngine::common_timer
    KBEngine::network_exception
    KBEngine::network_address
    KBEngine::network_event_dispatcher
)

target_include_directories(kbe_network_error_reporter
  PUBLIC
    "${KBE_SOURCE_DIR}/lib"
)

add_library(kbe_network_packet_filter STATIC
  "${KBE_SOURCE_DIR}/lib/network/packet_filter.cpp"
)

add_library(KBEngine::network_packet_filter ALIAS kbe_network_packet_filter)

target_link_libraries(kbe_network_packet_filter
  PUBLIC
    KBEngine::build_config
    KBEngine::common_smartpointer
  PRIVATE
    KBEngine::network_common
    KBEngine::network_channel
    KBEngine::network_packet_receiver
    KBEngine::network_packet_sender
)

target_include_directories(kbe_network_packet_filter
  PUBLIC
    "${KBE_SOURCE_DIR}/lib"
)

set(KBE_EVENT_POLLER_SOURCES
  "${KBE_SOURCE_DIR}/lib/network/event_poller.cpp"
  "${KBE_SOURCE_DIR}/lib/network/poller_select.cpp"
)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  list(APPEND KBE_EVENT_POLLER_SOURCES
    "${KBE_SOURCE_DIR}/lib/network/poller_epoll.cpp"
  )
endif()

if(WIN32)
  list(APPEND KBE_EVENT_POLLER_SOURCES
    "${KBE_SOURCE_DIR}/lib/network/poller_iocp.cpp"
  )
endif()

add_library(kbe_network_event_poller STATIC
  ${KBE_EVENT_POLLER_SOURCES}
)

add_library(KBEngine::network_event_poller ALIAS kbe_network_event_poller)

target_link_libraries(kbe_network_event_poller
  PUBLIC
    KBEngine::build_config
  PRIVATE
    KBEngine::helper_profile
    KBEngine::thread_concurrency
)

target_include_directories(kbe_network_event_poller
  PUBLIC
    "${KBE_SOURCE_DIR}/lib"
)

add_library(kbe_math STATIC
  "${KBE_SOURCE_DIR}/lib/dependencies/g3dlite/AABox.cpp"
  "${KBE_SOURCE_DIR}/lib/dependencies/g3dlite/Box.cpp"
  "${KBE_SOURCE_DIR}/lib/dependencies/g3dlite/Crypto.cpp"
  "${KBE_SOURCE_DIR}/lib/dependencies/g3dlite/format.cpp"
  "${KBE_SOURCE_DIR}/lib/dependencies/g3dlite/Matrix3.cpp"
  "${KBE_SOURCE_DIR}/lib/dependencies/g3dlite/Plane.cpp"
  "${KBE_SOURCE_DIR}/lib/dependencies/g3dlite/System.cpp"
  "${KBE_SOURCE_DIR}/lib/dependencies/g3dlite/Triangle.cpp"
  "${KBE_SOURCE_DIR}/lib/dependencies/g3dlite/Vector3.cpp"
  "${KBE_SOURCE_DIR}/lib/dependencies/g3dlite/Vector4.cpp"
  "${KBE_SOURCE_DIR}/lib/math/math.cpp"
)

add_library(KBEngine::math ALIAS kbe_math)

target_link_libraries(kbe_math
  PUBLIC
    KBEngine::build_config
)

target_include_directories(kbe_math
  PUBLIC
    "${KBE_SOURCE_DIR}/lib"
    "${KBE_SOURCE_DIR}/lib/dependencies/g3dlite"
)

add_library(kbe_resmgr STATIC
  "${KBE_SOURCE_DIR}/lib/resmgr/resmgr.cpp"
  "${KBE_SOURCE_DIR}/lib/resmgr/resourceobject.cpp"
)

add_library(KBEngine::resmgr ALIAS kbe_resmgr)

target_link_libraries(kbe_resmgr
  PUBLIC
    KBEngine::build_config
  PRIVATE
    KBEngine::common_smartpointer
    KBEngine::common_timer
    KBEngine::helper_debug_option
    KBEngine::helper_watcher
    KBEngine::thread_primitives
    KBEngine::xml
)

target_include_directories(kbe_resmgr
  PUBLIC
    "${KBE_SOURCE_DIR}/lib"
)

add_library(kbe_network_tcp_packet STATIC
  "${KBE_SOURCE_DIR}/lib/network/tcp_packet.cpp"
)

add_library(KBEngine::network_tcp_packet ALIAS kbe_network_tcp_packet)

target_link_libraries(kbe_network_tcp_packet
  PUBLIC
    KBEngine::build_config
    KBEngine::network_packet
    KBEngine::network_endpoint
  PRIVATE
    KBEngine::helper_debug_helper
)

target_include_directories(kbe_network_tcp_packet
  PUBLIC
    "${KBE_SOURCE_DIR}/lib"
)

add_library(kbe_network_udp_packet STATIC
  "${KBE_SOURCE_DIR}/lib/network/udp_packet.cpp"
)

add_library(KBEngine::network_udp_packet ALIAS kbe_network_udp_packet)

target_link_libraries(kbe_network_udp_packet
  PUBLIC
    KBEngine::build_config
    KBEngine::network_packet
    KBEngine::network_endpoint
  PRIVATE
    KBEngine::helper_debug_helper
)

target_include_directories(kbe_network_udp_packet
  PUBLIC
    "${KBE_SOURCE_DIR}/lib"
)

add_library(kbe_network_event_dispatcher STATIC
  "${KBE_SOURCE_DIR}/lib/network/event_dispatcher.cpp"
)

add_library(KBEngine::network_event_dispatcher ALIAS kbe_network_event_dispatcher)

target_link_libraries(kbe_network_event_dispatcher
  PUBLIC
    KBEngine::build_config
  PRIVATE
    KBEngine::common_tasks
    KBEngine::common_timer
    KBEngine::network_interfaces
    KBEngine::network_event_poller
    KBEngine::network_error_reporter
    KBEngine::helper_profile
)

target_include_directories(kbe_network_event_dispatcher
  PUBLIC
    "${KBE_SOURCE_DIR}/lib"
)

target_link_libraries(kbe_network_http_utility
  PRIVATE
    KBEngine::network_event_dispatcher
)

add_library(kbe_network_endpoint STATIC
  "${KBE_SOURCE_DIR}/lib/network/endpoint.cpp"
)

add_library(KBEngine::network_endpoint ALIAS kbe_network_endpoint)

target_link_libraries(kbe_network_endpoint
  PUBLIC
    KBEngine::build_config
    KBEngine::network_address
    KBEngine::network_common
    OpenSSL::SSL
    OpenSSL::Crypto
)

target_include_directories(kbe_network_endpoint
  PUBLIC
    "${KBE_SOURCE_DIR}/lib"
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

endif()

# ---------------------------------------------------------------------------
# Network: fixed_messages
# ---------------------------------------------------------------------------
add_library(kbe_network_fixed_messages STATIC
  "${KBE_SOURCE_DIR}/lib/network/fixed_messages.cpp"
)

add_library(KBEngine::network_fixed_messages ALIAS kbe_network_fixed_messages)

target_link_libraries(kbe_network_fixed_messages
  PUBLIC
    KBEngine::build_config
  PRIVATE
    KBEngine::network_common
    KBEngine::resmgr
    KBEngine::xml
)

target_include_directories(kbe_network_fixed_messages
  PUBLIC
    "${KBE_SOURCE_DIR}/lib"
)

# ---------------------------------------------------------------------------
# Server: machine_infos
# ---------------------------------------------------------------------------
add_library(kbe_server_machine_infos STATIC
  "${KBE_SOURCE_DIR}/lib/server/machine_infos.cpp"
)

add_library(KBEngine::server_machine_infos ALIAS kbe_server_machine_infos)

target_link_libraries(kbe_server_machine_infos
  PUBLIC
    KBEngine::build_config
  PRIVATE
    KBEngine::network_common
    KBEngine::network_address
    KBEngine::thread_primitives
    KBEngine::common_objectpool
    KBEngine::common_memorystream
)

target_include_directories(kbe_server_machine_infos
  PUBLIC
    "${KBE_SOURCE_DIR}/lib"
    "${KBE_SOURCE_DIR}/server"
)

# ---------------------------------------------------------------------------
# Server: serverconfig
# ---------------------------------------------------------------------------
add_library(kbe_server_serverconfig STATIC
  "${KBE_SOURCE_DIR}/lib/server/serverconfig.cpp"
)

add_library(KBEngine::server_serverconfig ALIAS kbe_server_serverconfig)

target_link_libraries(kbe_server_serverconfig
  PUBLIC
    KBEngine::build_config
  PRIVATE
    KBEngine::network_common
    KBEngine::network_address
    KBEngine::resmgr
    KBEngine::common_kbekey
    KBEngine::common_version
    KBEngine::xml
    KBEngine::common_timer
    KBEngine::common_tasks
    KBEngine::thread_primitives
)

target_include_directories(kbe_server_serverconfig
  PUBLIC
    "${KBE_SOURCE_DIR}/lib"
    "${KBE_SOURCE_DIR}/server"
)

# ---------------------------------------------------------------------------
# Server: shutdowner
# ---------------------------------------------------------------------------
add_library(kbe_server_shutdowner STATIC
  "${KBE_SOURCE_DIR}/lib/server/shutdowner.cpp"
)

add_library(KBEngine::server_shutdowner ALIAS kbe_server_shutdowner)

target_link_libraries(kbe_server_shutdowner
  PUBLIC
    KBEngine::build_config
  PRIVATE
    KBEngine::server_serverconfig
    KBEngine::network_event_dispatcher
    KBEngine::common_timer
)

target_include_directories(kbe_server_shutdowner
  PUBLIC
    "${KBE_SOURCE_DIR}/lib"
    "${KBE_SOURCE_DIR}/server"
)

# ---------------------------------------------------------------------------
# Server: signal_handler (stub)
# ---------------------------------------------------------------------------
add_library(kbe_server_signal_handler STATIC
  "${KBE_SOURCE_DIR}/lib/server/signal_handler.cpp"
)

add_library(KBEngine::server_signal_handler ALIAS kbe_server_signal_handler)

target_link_libraries(kbe_server_signal_handler
  PUBLIC
    KBEngine::build_config
  PRIVATE
    KBEngine::common_tasks
    KBEngine::network_event_dispatcher
)

target_include_directories(kbe_server_signal_handler
  PUBLIC
    "${KBE_SOURCE_DIR}/lib"
    "${KBE_SOURCE_DIR}/server"
)

# ---------------------------------------------------------------------------
# Server: common
# ---------------------------------------------------------------------------
add_library(kbe_server_common STATIC
  "${KBE_SOURCE_DIR}/lib/server/common.cpp"
)

add_library(KBEngine::server_common ALIAS kbe_server_common)

target_link_libraries(kbe_server_common
  PUBLIC
    KBEngine::build_config
    KBEngine::common_md5
    KBEngine::server_serverconfig
    KBEngine::server_machine_infos
    KBEngine::helper_sys_info
)

target_include_directories(kbe_server_common
  PUBLIC
    "${KBE_SOURCE_DIR}/lib"
    "${KBE_SOURCE_DIR}/server"
)

# ===========================================================================
# Network: ikcp (self-contained C protocol implementation)
# ===========================================================================
add_library(kbe_network_ikcp STATIC
  "${KBE_SOURCE_DIR}/lib/network/ikcp.c"
)

add_library(KBEngine::network_ikcp ALIAS kbe_network_ikcp)

target_link_libraries(kbe_network_ikcp
  PUBLIC
    KBEngine::build_config
)

target_include_directories(kbe_network_ikcp
  PUBLIC
    "${KBE_SOURCE_DIR}/lib"
)

# ---------------------------------------------------------------------------
# Network: packet_reader
# ---------------------------------------------------------------------------
add_library(kbe_network_packet_reader STATIC
  "${KBE_SOURCE_DIR}/lib/network/packet_reader.cpp"
)

add_library(KBEngine::network_packet_reader ALIAS kbe_network_packet_reader)

target_link_libraries(kbe_network_packet_reader
  PUBLIC
    KBEngine::build_config
  PRIVATE
    KBEngine::common_memorystream
    KBEngine::helper_debug_helper
)

target_include_directories(kbe_network_packet_reader
  PUBLIC
    "${KBE_SOURCE_DIR}/lib"
)

# ---------------------------------------------------------------------------
# Network: packet_sender
# ---------------------------------------------------------------------------
add_library(kbe_network_packet_sender STATIC
  "${KBE_SOURCE_DIR}/lib/network/packet_sender.cpp"
)

add_library(KBEngine::network_packet_sender ALIAS kbe_network_packet_sender)

target_link_libraries(kbe_network_packet_sender
  PUBLIC
    KBEngine::build_config
  PRIVATE
    KBEngine::network_common
    KBEngine::network_address
    KBEngine::network_endpoint
    KBEngine::helper_debug_helper
    OpenSSL::SSL
)

target_include_directories(kbe_network_packet_sender
  PUBLIC
    "${KBE_SOURCE_DIR}/lib"
)

# ---------------------------------------------------------------------------
# Network: encryption_filter
# ---------------------------------------------------------------------------
add_library(kbe_network_encryption_filter STATIC
  "${KBE_SOURCE_DIR}/lib/network/encryption_filter.cpp"
)

add_library(KBEngine::network_encryption_filter ALIAS kbe_network_encryption_filter)

target_link_libraries(kbe_network_encryption_filter
  PUBLIC
    KBEngine::build_config
    KBEngine::network_packet_filter
  PRIVATE
    KBEngine::network_common
    OpenSSL::SSL
)

target_include_directories(kbe_network_encryption_filter
  PUBLIC
    "${KBE_SOURCE_DIR}/lib"
)

# ---------------------------------------------------------------------------
# Network: websocket_protocol
# ---------------------------------------------------------------------------
add_library(kbe_network_websocket_protocol STATIC
  "${KBE_SOURCE_DIR}/lib/network/websocket_protocol.cpp"
)

add_library(KBEngine::network_websocket_protocol ALIAS kbe_network_websocket_protocol)

target_link_libraries(kbe_network_websocket_protocol
  PUBLIC
    KBEngine::build_config
  PRIVATE
    KBEngine::common_core
    KBEngine::helper_debug_helper
)

target_include_directories(kbe_network_websocket_protocol
  PUBLIC
    "${KBE_SOURCE_DIR}/lib"
)

# ---------------------------------------------------------------------------
# Network: delayed_channels
# ---------------------------------------------------------------------------
add_library(kbe_network_delayed_channels STATIC
  "${KBE_SOURCE_DIR}/lib/network/delayed_channels.cpp"
)

add_library(KBEngine::network_delayed_channels ALIAS kbe_network_delayed_channels)

target_link_libraries(kbe_network_delayed_channels
  PUBLIC
    KBEngine::build_config
  PRIVATE
    KBEngine::common_tasks
    KBEngine::common_smartpointer
)

target_include_directories(kbe_network_delayed_channels
  PUBLIC
    "${KBE_SOURCE_DIR}/lib"
)

# ---------------------------------------------------------------------------
# Network: packet_receiver (base class for tcp/udp receivers)
# ---------------------------------------------------------------------------
add_library(kbe_network_packet_receiver STATIC
  "${KBE_SOURCE_DIR}/lib/network/packet_receiver.cpp"
)

add_library(KBEngine::network_packet_receiver ALIAS kbe_network_packet_receiver)

target_link_libraries(kbe_network_packet_receiver
  PUBLIC
    KBEngine::build_config
  PRIVATE
    KBEngine::network_common
    KBEngine::network_tcp_packet
    KBEngine::helper_debug_helper
)

target_include_directories(kbe_network_packet_receiver
  PUBLIC
    "${KBE_SOURCE_DIR}/lib"
)

# ---------------------------------------------------------------------------
# Network: tcp_packet_receiver
# ---------------------------------------------------------------------------
add_library(kbe_network_tcp_packet_receiver STATIC
  "${KBE_SOURCE_DIR}/lib/network/tcp_packet_receiver.cpp"
)

add_library(KBEngine::network_tcp_packet_receiver ALIAS kbe_network_tcp_packet_receiver)

target_link_libraries(kbe_network_tcp_packet_receiver
  PUBLIC
    KBEngine::build_config
  PRIVATE
    KBEngine::network_common
    KBEngine::network_tcp_packet
    KBEngine::helper_debug_helper
)

target_include_directories(kbe_network_tcp_packet_receiver
  PUBLIC
    "${KBE_SOURCE_DIR}/lib"
)

# ---------------------------------------------------------------------------
# Network: tcp_packet_sender
# ---------------------------------------------------------------------------
add_library(kbe_network_tcp_packet_sender STATIC
  "${KBE_SOURCE_DIR}/lib/network/tcp_packet_sender.cpp"
)

add_library(KBEngine::network_tcp_packet_sender ALIAS kbe_network_tcp_packet_sender)

target_link_libraries(kbe_network_tcp_packet_sender
  PUBLIC
    KBEngine::build_config
  PRIVATE
    KBEngine::network_common
    KBEngine::network_packet_sender
    KBEngine::network_tcp_packet
    KBEngine::network_endpoint
    KBEngine::helper_debug_helper
)

target_include_directories(kbe_network_tcp_packet_sender
  PUBLIC
    "${KBE_SOURCE_DIR}/lib"
)

# ---------------------------------------------------------------------------
# Network: udp_packet_receiver
# ---------------------------------------------------------------------------
add_library(kbe_network_udp_packet_receiver STATIC
  "${KBE_SOURCE_DIR}/lib/network/udp_packet_receiver.cpp"
)

add_library(KBEngine::network_udp_packet_receiver ALIAS kbe_network_udp_packet_receiver)

target_link_libraries(kbe_network_udp_packet_receiver
  PUBLIC
    KBEngine::build_config
  PRIVATE
    KBEngine::network_common
    KBEngine::network_udp_packet
    KBEngine::helper_debug_helper
)

target_include_directories(kbe_network_udp_packet_receiver
  PUBLIC
    "${KBE_SOURCE_DIR}/lib"
)

# ---------------------------------------------------------------------------
# Network: udp_packet_sender
# ---------------------------------------------------------------------------
add_library(kbe_network_udp_packet_sender STATIC
  "${KBE_SOURCE_DIR}/lib/network/udp_packet_sender.cpp"
)

add_library(KBEngine::network_udp_packet_sender ALIAS kbe_network_udp_packet_sender)

target_link_libraries(kbe_network_udp_packet_sender
  PUBLIC
    KBEngine::build_config
  PRIVATE
    KBEngine::network_common
    KBEngine::network_packet_sender
    KBEngine::network_udp_packet
    KBEngine::network_endpoint
    KBEngine::network_ikcp
    KBEngine::helper_debug_helper
)

target_include_directories(kbe_network_udp_packet_sender
  PUBLIC
    "${KBE_SOURCE_DIR}/lib"
)

# ---------------------------------------------------------------------------
# Network: kcp_packet_reader / kcp_packet_receiver / kcp_packet_sender
# ---------------------------------------------------------------------------
add_library(kbe_network_kcp_packet_reader STATIC
  "${KBE_SOURCE_DIR}/lib/network/kcp_packet_reader.cpp"
)
add_library(KBEngine::network_kcp_packet_reader ALIAS kbe_network_kcp_packet_reader)
target_link_libraries(kbe_network_kcp_packet_reader
  PUBLIC KBEngine::build_config
)
target_include_directories(kbe_network_kcp_packet_reader
  PUBLIC "${KBE_SOURCE_DIR}/lib"
)

add_library(kbe_network_kcp_packet_receiver STATIC
  "${KBE_SOURCE_DIR}/lib/network/kcp_packet_receiver.cpp"
)
add_library(KBEngine::network_kcp_packet_receiver ALIAS kbe_network_kcp_packet_receiver)
target_link_libraries(kbe_network_kcp_packet_receiver
  PUBLIC KBEngine::build_config
  PRIVATE KBEngine::network_ikcp KBEngine::helper_debug_helper
)
target_include_directories(kbe_network_kcp_packet_receiver
  PUBLIC "${KBE_SOURCE_DIR}/lib"
)

add_library(kbe_network_kcp_packet_sender STATIC
  "${KBE_SOURCE_DIR}/lib/network/kcp_packet_sender.cpp"
)
add_library(KBEngine::network_kcp_packet_sender ALIAS kbe_network_kcp_packet_sender)
target_link_libraries(kbe_network_kcp_packet_sender
  PUBLIC KBEngine::build_config
  PRIVATE
    KBEngine::network_ikcp
    KBEngine::network_udp_packet_sender
    KBEngine::helper_debug_helper
)
target_include_directories(kbe_network_kcp_packet_sender
  PUBLIC "${KBE_SOURCE_DIR}/lib"
)

# ---------------------------------------------------------------------------
# Network: websocket_packet_reader / websocket_packet_filter
# ---------------------------------------------------------------------------
add_library(kbe_network_websocket_packet_reader STATIC
  "${KBE_SOURCE_DIR}/lib/network/websocket_packet_reader.cpp"
)
add_library(KBEngine::network_websocket_packet_reader ALIAS kbe_network_websocket_packet_reader)
target_link_libraries(kbe_network_websocket_packet_reader
  PUBLIC KBEngine::build_config
  PRIVATE KBEngine::helper_debug_helper
)
target_include_directories(kbe_network_websocket_packet_reader
  PUBLIC "${KBE_SOURCE_DIR}/lib"
)

add_library(kbe_network_websocket_packet_filter STATIC
  "${KBE_SOURCE_DIR}/lib/network/websocket_packet_filter.cpp"
)
add_library(KBEngine::network_websocket_packet_filter ALIAS kbe_network_websocket_packet_filter)
target_link_libraries(kbe_network_websocket_packet_filter
  PUBLIC KBEngine::build_config
  PRIVATE KBEngine::network_packet_filter KBEngine::network_websocket_protocol KBEngine::helper_debug_helper
)
target_include_directories(kbe_network_websocket_packet_filter
  PUBLIC "${KBE_SOURCE_DIR}/lib"
)

# ---------------------------------------------------------------------------
# Network: bundle (key milestone: part of circular dependency chain)
# ---------------------------------------------------------------------------
add_library(kbe_network_bundle STATIC
  "${KBE_SOURCE_DIR}/lib/network/bundle.cpp"
)

add_library(KBEngine::network_bundle ALIAS kbe_network_bundle)

target_link_libraries(kbe_network_bundle
  PUBLIC
    KBEngine::build_config
    KBEngine::common_objectpool
    KBEngine::common_memorystream
    KBEngine::network_tcp_packet
    KBEngine::network_udp_packet
    KBEngine::network_endpoint
  PRIVATE
    KBEngine::helper_debug_helper
    KBEngine::helper_profile
    KBEngine::network_packet_sender
    KBEngine::network_stats
    KBEngine::common_blowfish
)

target_include_directories(kbe_network_bundle
  PUBLIC
    "${KBE_SOURCE_DIR}/lib"
)

# ---------------------------------------------------------------------------
# Network: channel (key milestone: part of circular dependency chain)
# ---------------------------------------------------------------------------
add_library(kbe_network_channel STATIC
  "${KBE_SOURCE_DIR}/lib/network/channel.cpp"
)

add_library(KBEngine::network_channel ALIAS kbe_network_channel)

target_link_libraries(kbe_network_channel
  PUBLIC
    KBEngine::build_config
    KBEngine::network_bundle
    KBEngine::network_ikcp
    KBEngine::network_packet_reader
  PRIVATE
    KBEngine::common_ssl
    KBEngine::network_tcp_packet_receiver
    KBEngine::network_tcp_packet_sender
    KBEngine::network_udp_packet_receiver
    KBEngine::network_kcp_packet_sender
    KBEngine::network_kcp_packet_receiver
    KBEngine::network_kcp_packet_reader
    KBEngine::network_websocket_protocol
    KBEngine::network_websocket_packet_reader
    KBEngine::network_websocket_packet_filter
    KBEngine::network_packet_sender
    KBEngine::network_stats
    KBEngine::helper_profile
)

target_include_directories(kbe_network_channel
  PUBLIC
    "${KBE_SOURCE_DIR}/lib"
)

# ---------------------------------------------------------------------------
# Network: network_interface (THE key milestone: unlocks server layer)
# ---------------------------------------------------------------------------
add_library(kbe_network_interface STATIC
  "${KBE_SOURCE_DIR}/lib/network/network_interface.cpp"
  "${KBE_SOURCE_DIR}/lib/network/listener_receiver.cpp"
  "${KBE_SOURCE_DIR}/lib/network/listener_udp_receiver.cpp"
  "${KBE_SOURCE_DIR}/lib/network/listener_tcp_receiver.cpp"
)

add_library(KBEngine::network_interface ALIAS kbe_network_interface)

target_link_libraries(kbe_network_interface
  PUBLIC
    KBEngine::build_config
    KBEngine::network_channel
    KBEngine::network_bundle
  PRIVATE
    KBEngine::network_delayed_channels
    KBEngine::network_packet_receiver
    KBEngine::network_tcp_packet_receiver
    KBEngine::network_event_dispatcher
    KBEngine::network_stats
    KBEngine::network_message_handler
)

target_include_directories(kbe_network_interface
  PUBLIC
    "${KBE_SOURCE_DIR}/lib"
)

# ---------------------------------------------------------------------------
# Network: bundle_broadcast
# ---------------------------------------------------------------------------
add_library(kbe_network_bundle_broadcast STATIC
  "${KBE_SOURCE_DIR}/lib/network/bundle_broadcast.cpp"
)

add_library(KBEngine::network_bundle_broadcast ALIAS kbe_network_bundle_broadcast)

target_link_libraries(kbe_network_bundle_broadcast
  PUBLIC
    KBEngine::build_config
    KBEngine::network_bundle
    KBEngine::network_endpoint
  PRIVATE
    KBEngine::network_interface
    KBEngine::network_message_handler
)

target_include_directories(kbe_network_bundle_broadcast
  PUBLIC
    "${KBE_SOURCE_DIR}/lib"
)

# ---------------------------------------------------------------------------
# Helper: profile_handler
# ---------------------------------------------------------------------------
add_library(kbe_helper_profile_handler STATIC
  "${KBE_SOURCE_DIR}/lib/helper/profile_handler.cpp"
)

add_library(KBEngine::helper_profile_handler ALIAS kbe_helper_profile_handler)

target_link_libraries(kbe_helper_profile_handler
  PUBLIC
    KBEngine::build_config
  PRIVATE
    KBEngine::common_timer
    KBEngine::common_tasks
    KBEngine::helper_debug_helper
    KBEngine::helper_event_stats
    KBEngine::helper_profile
    KBEngine::network_interface
    KBEngine::network_address
    KBEngine::network_message_handler
    KBEngine::network_bundle
)

target_include_directories(kbe_helper_profile_handler
  PUBLIC
    "${KBE_SOURCE_DIR}/lib"
)

# ---------------------------------------------------------------------------
# Helper: sys_info
# ---------------------------------------------------------------------------
add_library(kbe_helper_sys_info STATIC
  "${KBE_SOURCE_DIR}/lib/helper/sys_info.cpp"
)

add_library(KBEngine::helper_sys_info ALIAS kbe_helper_sys_info)

target_link_libraries(kbe_helper_sys_info
  PUBLIC
    KBEngine::build_config
    KBEngine::helper_debug_helper
)

target_include_directories(kbe_helper_sys_info
  PUBLIC
    "${KBE_SOURCE_DIR}/lib"
)

# ---------------------------------------------------------------------------
# Helper: watch_pools
# ---------------------------------------------------------------------------
add_library(kbe_helper_watch_pools STATIC
  "${KBE_SOURCE_DIR}/lib/helper/watch_pools.cpp"
)

add_library(KBEngine::helper_watch_pools ALIAS kbe_helper_watch_pools)

target_link_libraries(kbe_helper_watch_pools
  PUBLIC
    KBEngine::build_config
    KBEngine::helper_watcher
  PRIVATE
    KBEngine::network_bundle
    KBEngine::network_address
    KBEngine::network_endpoint
    KBEngine::network_tcp_packet
    KBEngine::network_udp_packet
    KBEngine::network_tcp_packet_receiver
    KBEngine::network_udp_packet_receiver
)

target_include_directories(kbe_helper_watch_pools
  PUBLIC
    "${KBE_SOURCE_DIR}/lib"
)

# ---------------------------------------------------------------------------
# Server: components bootstrap support
# ---------------------------------------------------------------------------
add_library(kbe_server_components_bootstrap_support STATIC
  "${KBE_SOURCE_DIR}/lib/server/components_interface_bootstrap.cpp"
  "${KBE_SOURCE_DIR}/lib/server/components_bootstrap_support.cpp"
)

target_link_libraries(kbe_server_components_bootstrap_support
  PUBLIC
    KBEngine::build_config
)

target_include_directories(kbe_server_components_bootstrap_support
  PUBLIC
    "${KBE_SOURCE_DIR}/lib"
    "${KBE_SOURCE_DIR}/server"
)

# ---------------------------------------------------------------------------
# Server: components
# ---------------------------------------------------------------------------
add_library(kbe_server_components STATIC
  "${KBE_SOURCE_DIR}/lib/server/components.cpp"
)

add_library(KBEngine::server_components ALIAS kbe_server_components)

target_link_libraries(kbe_server_components
  PUBLIC
    KBEngine::build_config
    KBEngine::common_tasks
    KBEngine::common_timer
    KBEngine::common_smartpointer
    KBEngine::helper_debug_helper
    KBEngine::helper_sys_info
    KBEngine::server_serverconfig
    KBEngine::network_channel
    KBEngine::network_address
    KBEngine::network_bundle
    KBEngine::network_bundle_broadcast
    KBEngine::network_tcp_packet
    KBEngine::network_udp_packet
    KBEngine::network_interface
    kbe_server_components_bootstrap_support
  PRIVATE
    KBEngine::network_endpoint
)

target_include_directories(kbe_server_components
  PUBLIC
    "${KBE_SOURCE_DIR}/lib"
    "${KBE_SOURCE_DIR}/server"
)

# ---------------------------------------------------------------------------
# Server: component_active_report_handler
# ---------------------------------------------------------------------------
add_library(kbe_server_component_active_report_handler STATIC
  "${KBE_SOURCE_DIR}/lib/server/component_active_report_handler.cpp"
)

add_library(KBEngine::server_component_active_report_handler ALIAS kbe_server_component_active_report_handler)

target_link_libraries(kbe_server_component_active_report_handler
  PUBLIC
    KBEngine::build_config
    KBEngine::common_timer
    KBEngine::common_tasks
    KBEngine::network_bundle
    KBEngine::network_interface
    KBEngine::server_components
)

target_include_directories(kbe_server_component_active_report_handler
  PUBLIC
    "${KBE_SOURCE_DIR}/lib"
    "${KBE_SOURCE_DIR}/server"
)

# ---------------------------------------------------------------------------
# Server: script_timers
# ---------------------------------------------------------------------------
add_library(kbe_server_script_timers STATIC
  "${KBE_SOURCE_DIR}/lib/server/script_timers.cpp"
)

add_library(KBEngine::server_script_timers ALIAS kbe_server_script_timers)

target_link_libraries(kbe_server_script_timers
  PUBLIC
    KBEngine::build_config
    KBEngine::common_timer
    KBEngine::server_serverconfig
)

target_include_directories(kbe_server_script_timers
  PUBLIC
    "${KBE_SOURCE_DIR}/lib"
    "${KBE_SOURCE_DIR}/server"
)

# ---------------------------------------------------------------------------
# Server: globaldata_server
# ---------------------------------------------------------------------------
add_library(kbe_server_globaldata_server STATIC
  "${KBE_SOURCE_DIR}/lib/server/globaldata_server.cpp"
)

add_library(KBEngine::server_globaldata_server ALIAS kbe_server_globaldata_server)

target_link_libraries(kbe_server_globaldata_server
  PUBLIC
    KBEngine::build_config
  PRIVATE
    KBEngine::server_components
    KBEngine::network_channel
)

target_include_directories(kbe_server_globaldata_server
  PUBLIC
    "${KBE_SOURCE_DIR}/lib"
    "${KBE_SOURCE_DIR}/server"
)

# ---------------------------------------------------------------------------
# Server: sendmail_threadtasks
# ---------------------------------------------------------------------------
add_library(kbe_server_sendmail STATIC
  "${KBE_SOURCE_DIR}/lib/server/sendmail_threadtasks.cpp"
)

add_library(KBEngine::server_sendmail ALIAS kbe_server_sendmail)

target_link_libraries(kbe_server_sendmail
  PUBLIC
    KBEngine::build_config
    KBEngine::thread_task
    KBEngine::network_address
    KBEngine::server_serverconfig
    KBEngine::helper_debug_helper
    KBEngine::common_deadline
    CURL::libcurl
)

target_include_directories(kbe_server_sendmail
  PUBLIC
    "${KBE_SOURCE_DIR}/lib"
    "${KBE_SOURCE_DIR}/server"
)

# ---------------------------------------------------------------------------
# Server: pendingLoginmgr
# ---------------------------------------------------------------------------
add_library(kbe_server_pending_loginmgr STATIC
  "${KBE_SOURCE_DIR}/lib/server/pendingLoginmgr.cpp"
)

add_library(KBEngine::server_pending_loginmgr ALIAS kbe_server_pending_loginmgr)

target_link_libraries(kbe_server_pending_loginmgr
  PUBLIC
    KBEngine::build_config
    KBEngine::common_tasks
    KBEngine::common_timer
    KBEngine::network_channel
    KBEngine::network_interface
  PRIVATE
    KBEngine::helper_profile
)

target_include_directories(kbe_server_pending_loginmgr
  PUBLIC
    "${KBE_SOURCE_DIR}/lib"
    "${KBE_SOURCE_DIR}/server"
)

# ---------------------------------------------------------------------------
# Server: forward_messagebuffer
# ---------------------------------------------------------------------------
add_library(kbe_server_forward_messagebuffer STATIC
  "${KBE_SOURCE_DIR}/lib/server/forward_messagebuffer.cpp"
)

add_library(KBEngine::server_forward_messagebuffer ALIAS kbe_server_forward_messagebuffer)

target_link_libraries(kbe_server_forward_messagebuffer
  PUBLIC
    KBEngine::build_config
    KBEngine::common_tasks
    KBEngine::network_bundle
    KBEngine::network_channel
    KBEngine::network_interface
    KBEngine::server_components
)

target_include_directories(kbe_server_forward_messagebuffer
  PUBLIC
    "${KBE_SOURCE_DIR}/lib"
    "${KBE_SOURCE_DIR}/server"
)

# ---------------------------------------------------------------------------
# Script: pyscript
# ---------------------------------------------------------------------------
add_library(kbe_pyscript STATIC
  "${KBE_SOURCE_DIR}/lib/pyscript/copy.cpp"
  "${KBE_SOURCE_DIR}/lib/pyscript/map.cpp"
  "${KBE_SOURCE_DIR}/lib/pyscript/math.cpp"
  "${KBE_SOURCE_DIR}/lib/pyscript/pickler.cpp"
  "${KBE_SOURCE_DIR}/lib/pyscript/py_compression.cpp"
  "${KBE_SOURCE_DIR}/lib/pyscript/py_gc.cpp"
  "${KBE_SOURCE_DIR}/lib/pyscript/pyurl.cpp"
  "${KBE_SOURCE_DIR}/lib/pyscript/py_platform.cpp"
  "${KBE_SOURCE_DIR}/lib/pyscript/pystruct.cpp"
  "${KBE_SOURCE_DIR}/lib/pyscript/pyprofile.cpp"
  "${KBE_SOURCE_DIR}/lib/pyscript/pyprofile_handler.cpp"
  "${KBE_SOURCE_DIR}/lib/pyscript/py_memorystream.cpp"
  "${KBE_SOURCE_DIR}/lib/pyscript/pywatcher.cpp"
  "${KBE_SOURCE_DIR}/lib/pyscript/script.cpp"
  "${KBE_SOURCE_DIR}/lib/pyscript/scriptobject.cpp"
  "${KBE_SOURCE_DIR}/lib/pyscript/scriptstdout.cpp"
  "${KBE_SOURCE_DIR}/lib/pyscript/scriptstderr.cpp"
  "${KBE_SOURCE_DIR}/lib/pyscript/scriptstdouterr.cpp"
  "${KBE_SOURCE_DIR}/lib/pyscript/scriptstdouterrhook.cpp"
  "${KBE_SOURCE_DIR}/lib/pyscript/sequence.cpp"
  "${KBE_SOURCE_DIR}/lib/pyscript/vector2.cpp"
  "${KBE_SOURCE_DIR}/lib/pyscript/vector3.cpp"
  "${KBE_SOURCE_DIR}/lib/pyscript/vector4.cpp"
)

add_library(KBEngine::pyscript ALIAS kbe_pyscript)

target_link_libraries(kbe_pyscript
  PUBLIC
    KBEngine::build_config
    KBEngine::common_memorystream
    KBEngine::helper_debug_helper
    KBEngine::helper_profile
    KBEngine::helper_watcher
    KBEngine::math
    KBEngine::network_address
    KBEngine::network_common
    KBEngine::resmgr
    KBEngine::thread_concurrency
    KBEngine::tinyxml
    Python3::Python
)

target_include_directories(kbe_pyscript
  PUBLIC
    "${KBE_SOURCE_DIR}/lib"
    "${KBE_SOURCE_DIR}/server"
)

# ---------------------------------------------------------------------------
# Entity: entitydef
# ---------------------------------------------------------------------------
add_library(kbe_entitydef STATIC
  "${KBE_SOURCE_DIR}/lib/entitydef/fixedarray.cpp"
  "${KBE_SOURCE_DIR}/lib/entitydef/common.cpp"
  "${KBE_SOURCE_DIR}/lib/entitydef/datatype.cpp"
  "${KBE_SOURCE_DIR}/lib/entitydef/datatypes.cpp"
  "${KBE_SOURCE_DIR}/lib/entitydef/detaillevel.cpp"
  "${KBE_SOURCE_DIR}/lib/entitydef/entity_component.cpp"
  "${KBE_SOURCE_DIR}/lib/entitydef/entity_component_call.cpp"
  "${KBE_SOURCE_DIR}/lib/entitydef/entity_call.cpp"
  "${KBE_SOURCE_DIR}/lib/entitydef/entitydef.cpp"
  "${KBE_SOURCE_DIR}/lib/entitydef/py_entitydef.cpp"
  "${KBE_SOURCE_DIR}/lib/entitydef/entitycallabstract.cpp"
  "${KBE_SOURCE_DIR}/lib/entitydef/fixeddict.cpp"
  "${KBE_SOURCE_DIR}/lib/entitydef/method.cpp"
  "${KBE_SOURCE_DIR}/lib/entitydef/property.cpp"
  "${KBE_SOURCE_DIR}/lib/entitydef/remote_entity_method.cpp"
  "${KBE_SOURCE_DIR}/lib/entitydef/scriptdef_module.cpp"
  "${KBE_SOURCE_DIR}/lib/entitydef/volatileinfo.cpp"
)

add_library(KBEngine::entitydef ALIAS kbe_entitydef)

target_link_libraries(kbe_entitydef
  PUBLIC
    KBEngine::build_config
    KBEngine::math
    KBEngine::network_bundle
    KBEngine::network_channel
    KBEngine::network_message_handler
    KBEngine::pyscript
)

target_include_directories(kbe_entitydef
  PUBLIC
    "${KBE_SOURCE_DIR}/lib"
    "${KBE_SOURCE_DIR}/server"
)

# ---------------------------------------------------------------------------
# Server: runtime
# ---------------------------------------------------------------------------
add_library(kbe_server_runtime STATIC
  "${KBE_SOURCE_DIR}/lib/server/entity_app.cpp"
  "${KBE_SOURCE_DIR}/lib/server/globaldata_client.cpp"
  "${KBE_SOURCE_DIR}/lib/server/idallocate.cpp"
  "${KBE_SOURCE_DIR}/lib/server/id_component_querier.cpp"
  "${KBE_SOURCE_DIR}/lib/server/py_file_descriptor.cpp"
  "${KBE_SOURCE_DIR}/lib/server/python_app.cpp"
  "${KBE_SOURCE_DIR}/lib/server/serverapp.cpp"
  "${KBE_SOURCE_DIR}/lib/server/telnet_handler.cpp"
  "${KBE_SOURCE_DIR}/lib/server/telnet_server.cpp"
)

add_library(KBEngine::server_runtime ALIAS kbe_server_runtime)

target_link_libraries(kbe_server_runtime
  PUBLIC
    KBEngine::build_config
    KBEngine::common_core
    KBEngine::common_kbekey
    KBEngine::common_memorystream
    KBEngine::entitydef
    KBEngine::helper_debug_helper
    KBEngine::helper_event_stats
    KBEngine::helper_profile_handler
    KBEngine::helper_sys_info
    KBEngine::helper_watch_pools
    KBEngine::math
    KBEngine::network_bundle_broadcast
    KBEngine::network_http_utility
    KBEngine::network_interface
    KBEngine::network_udp_packet_sender
    KBEngine::pyscript
    KBEngine::resmgr
    KBEngine::server_sendmail
    KBEngine::thread_pool
  PRIVATE
    KBEngine::server_component_active_report_handler
    KBEngine::server_common
    KBEngine::server_components
    KBEngine::server_forward_messagebuffer
    KBEngine::server_globaldata_server
    KBEngine::server_machine_infos
    KBEngine::server_pending_loginmgr
    KBEngine::server_script_timers
    KBEngine::server_serverconfig
    KBEngine::server_shutdowner
    KBEngine::server_signal_handler
)

target_include_directories(kbe_server_runtime
  PUBLIC
    "${KBE_SOURCE_DIR}/lib"
    "${KBE_SOURCE_DIR}/server"
)

# ---------------------------------------------------------------------------
# Client: client_lib
# ---------------------------------------------------------------------------
add_library(kbe_client_lib STATIC
  "${KBE_SOURCE_DIR}/lib/client_lib/client_interface.cpp"
  "${KBE_SOURCE_DIR}/lib/client_lib/clientapp.cpp"
  "${KBE_SOURCE_DIR}/lib/client_lib/clientobjectbase.cpp"
  "${KBE_SOURCE_DIR}/lib/client_lib/config.cpp"
  "${KBE_SOURCE_DIR}/lib/client_lib/script_callbacks.cpp"
  "${KBE_SOURCE_DIR}/lib/client_lib/entity.cpp"
  "${KBE_SOURCE_DIR}/lib/client_lib/entity_aspect.cpp"
  "${KBE_SOURCE_DIR}/lib/client_lib/entity_component.cpp"
  "${KBE_SOURCE_DIR}/lib/client_lib/event.cpp"
  "${KBE_SOURCE_DIR}/lib/client_lib/moveto_point_handler.cpp"
  "${KBE_SOURCE_DIR}/lib/client_lib/profile.cpp"
)

add_library(KBEngine::client_lib ALIAS kbe_client_lib)

target_link_libraries(kbe_client_lib
  PUBLIC
    KBEngine::build_config
    KBEngine::entitydef
    KBEngine::helper_debug_helper
    KBEngine::math
    KBEngine::network_encryption_filter
    KBEngine::network_interface
    KBEngine::pyscript
    KBEngine::thread_pool
    KBEngine::tinyxml
)

target_compile_definitions(kbe_client_lib
  PUBLIC
    KBE_CLIENT
)

target_include_directories(kbe_client_lib
  PUBLIC
    "${KBE_SOURCE_DIR}/lib"
    "${KBE_SOURCE_DIR}/server"
)

# ---------------------------------------------------------------------------
# Navigation
# ---------------------------------------------------------------------------
add_library(kbe_navigation STATIC
  "${KBE_SOURCE_DIR}/lib/navigation/DetourAlloc.cpp"
  "${KBE_SOURCE_DIR}/lib/navigation/DetourAssert.cpp"
  "${KBE_SOURCE_DIR}/lib/navigation/DetourCommon.cpp"
  "${KBE_SOURCE_DIR}/lib/navigation/DetourCrowd.cpp"
  "${KBE_SOURCE_DIR}/lib/navigation/DetourLocalBoundary.cpp"
  "${KBE_SOURCE_DIR}/lib/navigation/DetourNavMesh.cpp"
  "${KBE_SOURCE_DIR}/lib/navigation/DetourNavMeshBuilder.cpp"
  "${KBE_SOURCE_DIR}/lib/navigation/DetourNavMeshQuery.cpp"
  "${KBE_SOURCE_DIR}/lib/navigation/DetourNode.cpp"
  "${KBE_SOURCE_DIR}/lib/navigation/DetourObstacleAvoidance.cpp"
  "${KBE_SOURCE_DIR}/lib/navigation/DetourPathCorridor.cpp"
  "${KBE_SOURCE_DIR}/lib/navigation/DetourPathQueue.cpp"
  "${KBE_SOURCE_DIR}/lib/navigation/DetourProximityGrid.cpp"
  "${KBE_SOURCE_DIR}/lib/navigation/DetourTileCache.cpp"
  "${KBE_SOURCE_DIR}/lib/navigation/DetourTileCacheBuilder.cpp"
  "${KBE_SOURCE_DIR}/lib/navigation/navigation.cpp"
  "${KBE_SOURCE_DIR}/lib/navigation/navigation_handle.cpp"
  "${KBE_SOURCE_DIR}/lib/navigation/navigation_mesh_handle.cpp"
  "${KBE_SOURCE_DIR}/lib/navigation/navigation_tile_handle.cpp"
  "${KBE_SOURCE_DIR}/lib/navigation/Recast.cpp"
  "${KBE_SOURCE_DIR}/lib/navigation/RecastAlloc.cpp"
  "${KBE_SOURCE_DIR}/lib/navigation/RecastArea.cpp"
  "${KBE_SOURCE_DIR}/lib/navigation/RecastAssert.cpp"
  "${KBE_SOURCE_DIR}/lib/navigation/RecastContour.cpp"
  "${KBE_SOURCE_DIR}/lib/navigation/RecastFilter.cpp"
  "${KBE_SOURCE_DIR}/lib/navigation/RecastLayers.cpp"
  "${KBE_SOURCE_DIR}/lib/navigation/RecastMesh.cpp"
  "${KBE_SOURCE_DIR}/lib/navigation/RecastMeshDetail.cpp"
  "${KBE_SOURCE_DIR}/lib/navigation/RecastRasterization.cpp"
  "${KBE_SOURCE_DIR}/lib/navigation/RecastRegion.cpp"
)

add_library(KBEngine::navigation ALIAS kbe_navigation)

target_link_libraries(kbe_navigation
  PUBLIC
    KBEngine::build_config
    KBEngine::dependency_tmxparser
    KBEngine::math
    KBEngine::resmgr
    KBEngine::thread_pool
)

target_include_directories(kbe_navigation
  PUBLIC
    "${KBE_SOURCE_DIR}/lib"
    "${KBE_SOURCE_DIR}/lib/dependencies/tmxparser"
)

# ---------------------------------------------------------------------------
# Database: interface core
# ---------------------------------------------------------------------------
add_library(kbe_db_interface STATIC
  "${KBE_SOURCE_DIR}/lib/db_interface/db_tasks.cpp"
  "${KBE_SOURCE_DIR}/lib/db_interface/db_threadpool.cpp"
  "${KBE_SOURCE_DIR}/lib/db_interface/entity_table.cpp"
  "${KBE_SOURCE_DIR}/lib/db_interface/kbe_tables.cpp"
)

add_library(KBEngine::db_interface ALIAS kbe_db_interface)

target_link_libraries(kbe_db_interface
  PUBLIC
    KBEngine::build_config
    KBEngine::entitydef
    KBEngine::server_runtime
    KBEngine::thread_pool
)

target_include_directories(kbe_db_interface
  PUBLIC
    "${KBE_SOURCE_DIR}/lib"
    "${KBE_SOURCE_DIR}/server"
)

# ---------------------------------------------------------------------------
# Database: mysql backend
# ---------------------------------------------------------------------------
add_library(kbe_db_mysql STATIC
  "${KBE_SOURCE_DIR}/lib/db_mysql/common.cpp"
  "${KBE_SOURCE_DIR}/lib/db_mysql/db_exception.cpp"
  "${KBE_SOURCE_DIR}/lib/db_mysql/db_interface_mysql.cpp"
  "${KBE_SOURCE_DIR}/lib/db_mysql/db_transaction.cpp"
  "${KBE_SOURCE_DIR}/lib/db_mysql/entity_sqlstatement_mapping.cpp"
  "${KBE_SOURCE_DIR}/lib/db_mysql/entity_table_mysql.cpp"
  "${KBE_SOURCE_DIR}/lib/db_mysql/kbe_table_mysql.cpp"
)

add_library(KBEngine::db_mysql ALIAS kbe_db_mysql)

target_link_libraries(kbe_db_mysql
  PUBLIC
    KBEngine::build_config
    KBEngine::db_interface
    KBEngine::network_fixed_messages
)

if(TARGET kbe_dependency_mysqlclient)
  target_link_libraries(kbe_db_mysql PUBLIC KBEngine::dependency_mysqlclient)
endif()

target_compile_definitions(kbe_db_mysql
  PUBLIC
    KBE_USE_MYSQL
)

target_include_directories(kbe_db_mysql
  PUBLIC
    "${KBE_SOURCE_DIR}/lib"
    "${KBE_SOURCE_DIR}/server"
)

# ---------------------------------------------------------------------------
# Database: redis backend
# ---------------------------------------------------------------------------
add_library(kbe_db_redis STATIC
  "${KBE_SOURCE_DIR}/lib/db_redis/common.cpp"
  "${KBE_SOURCE_DIR}/lib/db_redis/db_exception.cpp"
  "${KBE_SOURCE_DIR}/lib/db_redis/db_interface_redis.cpp"
  "${KBE_SOURCE_DIR}/lib/db_redis/db_transaction.cpp"
  "${KBE_SOURCE_DIR}/lib/db_redis/entity_table_redis.cpp"
  "${KBE_SOURCE_DIR}/lib/db_redis/kbe_table_redis.cpp"
  "${KBE_SOURCE_DIR}/lib/db_redis/redis_watcher.cpp"
)

add_library(KBEngine::db_redis ALIAS kbe_db_redis)

target_link_libraries(kbe_db_redis
  PUBLIC
    KBEngine::build_config
    KBEngine::db_interface
    KBEngine::dependency_hiredis
    KBEngine::network_fixed_messages
)

target_compile_definitions(kbe_db_redis
  PUBLIC
    KBE_USE_REDIS
)

target_include_directories(kbe_db_redis
  PUBLIC
    "${KBE_SOURCE_DIR}/lib"
    "${KBE_SOURCE_DIR}/server"
)

# ---------------------------------------------------------------------------
# Database: runtime bridge/factory
# ---------------------------------------------------------------------------
add_library(kbe_db_runtime STATIC
  "${KBE_SOURCE_DIR}/lib/db_interface/db_interface.cpp"
)

add_library(KBEngine::db_runtime ALIAS kbe_db_runtime)

target_link_libraries(kbe_db_runtime
  PUBLIC
    KBEngine::build_config
    KBEngine::db_interface
    KBEngine::db_mysql
    KBEngine::db_redis
)

target_include_directories(kbe_db_runtime
  PUBLIC
    "${KBE_SOURCE_DIR}/lib"
)
