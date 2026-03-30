add_subdirectory(lib)

function(kbe_add_server_executable target_name output_name compile_define)
  add_executable(${target_name} ${ARGN})

  target_link_libraries(${target_name}
    PRIVATE
      KBEngine::server_runtime
  )

  target_include_directories(${target_name}
    PRIVATE
      "${KBE_SOURCE_DIR}/lib"
      "${KBE_SOURCE_DIR}/server"
  )

  target_compile_definitions(${target_name}
    PRIVATE
      ${compile_define}
  )

  set_target_properties(${target_name}
    PROPERTIES
      OUTPUT_NAME ${output_name}
  )
endfunction()

function(kbe_add_tool_executable target_name output_name compile_define)
  add_executable(${target_name} ${ARGN})

  target_link_libraries(${target_name}
    PRIVATE
      KBEngine::server_runtime
  )

  target_include_directories(${target_name}
    PRIVATE
      "${KBE_SOURCE_DIR}/lib"
      "${KBE_SOURCE_DIR}/server"
  )

  target_compile_definitions(${target_name}
    PRIVATE
      ${compile_define}
  )

  set_target_properties(${target_name}
    PROPERTIES
      OUTPUT_NAME ${output_name}
  )
endfunction()

function(kbe_add_client_tool_executable target_name output_name compile_define)
  add_executable(${target_name} ${ARGN})

  target_link_libraries(${target_name}
    PRIVATE
      KBEngine::client_lib
      KBEngine::server_runtime
  )

  target_include_directories(${target_name}
    PRIVATE
      "${KBE_SOURCE_DIR}/lib"
      "${KBE_SOURCE_DIR}/server"
  )

  target_compile_definitions(${target_name}
    PRIVATE
      ${compile_define}
  )

  set_target_properties(${target_name}
    PROPERTIES
      OUTPUT_NAME ${output_name}
  )
endfunction()

kbe_add_server_executable(kbe_machine machine KBE_MACHINE
  "${KBE_SOURCE_DIR}/server/machine/machine.cpp"
  "${KBE_SOURCE_DIR}/server/machine/machine_interface.cpp"
  "${KBE_SOURCE_DIR}/server/machine/main.cpp"
)

kbe_add_server_executable(kbe_baseappmgr baseappmgr KBE_BASEAPPMGR
  "${KBE_SOURCE_DIR}/server/baseappmgr/baseapp.cpp"
  "${KBE_SOURCE_DIR}/server/baseappmgr/baseappmgr.cpp"
  "${KBE_SOURCE_DIR}/server/baseappmgr/baseappmgr_interface.cpp"
  "${KBE_SOURCE_DIR}/server/baseappmgr/main.cpp"
)

kbe_add_server_executable(kbe_baseapp baseapp KBE_BASEAPP
  "${KBE_SOURCE_DIR}/server/baseapp/archiver.cpp"
  "${KBE_SOURCE_DIR}/server/baseapp/entity.cpp"
  "${KBE_SOURCE_DIR}/server/baseapp/entity_component.cpp"
  "${KBE_SOURCE_DIR}/server/baseapp/baseapp.cpp"
  "${KBE_SOURCE_DIR}/server/baseapp/baseapp_interface.cpp"
  "${KBE_SOURCE_DIR}/server/baseapp/backuper.cpp"
  "${KBE_SOURCE_DIR}/server/baseapp/entity_messages_forward_handler.cpp"
  "${KBE_SOURCE_DIR}/server/baseapp/data_download.cpp"
  "${KBE_SOURCE_DIR}/server/baseapp/data_downloads.cpp"
  "${KBE_SOURCE_DIR}/server/baseapp/entity_autoloader.cpp"
  "${KBE_SOURCE_DIR}/server/baseapp/entity_remotemethod.cpp"
  "${KBE_SOURCE_DIR}/server/baseapp/forward_message_over_handler.cpp"
  "${KBE_SOURCE_DIR}/server/baseapp/initprogress_handler.cpp"
  "${KBE_SOURCE_DIR}/server/baseapp/restore_entity_handler.cpp"
  "${KBE_SOURCE_DIR}/server/baseapp/sync_entitystreamtemplate_handler.cpp"
  "${KBE_SOURCE_DIR}/server/baseapp/space.cpp"
  "${KBE_SOURCE_DIR}/server/baseapp/proxy.cpp"
  "${KBE_SOURCE_DIR}/server/baseapp/proxy_forwarder.cpp"
  "${KBE_SOURCE_DIR}/server/baseapp/profile.cpp"
  "${KBE_SOURCE_DIR}/server/baseapp/main.cpp"
)

target_link_libraries(kbe_baseapp
  PRIVATE
    KBEngine::network_encryption_filter
)

kbe_add_server_executable(kbe_cellappmgr cellappmgr KBE_CELLAPPMGR
  "${KBE_SOURCE_DIR}/server/cellappmgr/cell.cpp"
  "${KBE_SOURCE_DIR}/server/cellappmgr/cells.cpp"
  "${KBE_SOURCE_DIR}/server/cellappmgr/cellapp.cpp"
  "${KBE_SOURCE_DIR}/server/cellappmgr/cellappmgr.cpp"
  "${KBE_SOURCE_DIR}/server/cellappmgr/cellappmgr_interface.cpp"
  "${KBE_SOURCE_DIR}/server/cellappmgr/main.cpp"
  "${KBE_SOURCE_DIR}/server/cellappmgr/space.cpp"
  "${KBE_SOURCE_DIR}/server/cellappmgr/spaces.cpp"
  "${KBE_SOURCE_DIR}/server/cellappmgr/space_viewer.cpp"
)

kbe_add_server_executable(kbe_loginapp loginapp KBE_LOGINAPP
  "${KBE_SOURCE_DIR}/server/loginapp/clientsdk_downloader.cpp"
  "${KBE_SOURCE_DIR}/server/loginapp/http_cb_handler.cpp"
  "${KBE_SOURCE_DIR}/server/loginapp/loginapp.cpp"
  "${KBE_SOURCE_DIR}/server/loginapp/loginapp_interface.cpp"
  "${KBE_SOURCE_DIR}/server/loginapp/profile.cpp"
  "${KBE_SOURCE_DIR}/server/loginapp/main.cpp"
)

target_link_libraries(kbe_loginapp
  PRIVATE
    KBEngine::network_encryption_filter
)

kbe_add_server_executable(kbe_cellapp cellapp KBE_CELLAPP
  "${KBE_SOURCE_DIR}/server/cellapp/all_clients.cpp"
  "${KBE_SOURCE_DIR}/server/cellapp/view_trigger.cpp"
  "${KBE_SOURCE_DIR}/server/cellapp/cell.cpp"
  "${KBE_SOURCE_DIR}/server/cellapp/cells.cpp"
  "${KBE_SOURCE_DIR}/server/cellapp/cellapp.cpp"
  "${KBE_SOURCE_DIR}/server/cellapp/cellapp_interface.cpp"
  "${KBE_SOURCE_DIR}/server/cellapp/clients_remote_entity_method.cpp"
  "${KBE_SOURCE_DIR}/server/cellapp/controller.cpp"
  "${KBE_SOURCE_DIR}/server/cellapp/controllers.cpp"
  "${KBE_SOURCE_DIR}/server/cellapp/client_entity.cpp"
  "${KBE_SOURCE_DIR}/server/cellapp/client_entity_method.cpp"
  "${KBE_SOURCE_DIR}/server/cellapp/forward_message_over_handler.cpp"
  "${KBE_SOURCE_DIR}/server/cellapp/entity.cpp"
  "${KBE_SOURCE_DIR}/server/cellapp/entityref.cpp"
  "${KBE_SOURCE_DIR}/server/cellapp/entity_remotemethod.cpp"
  "${KBE_SOURCE_DIR}/server/cellapp/entity_coordinate_node.cpp"
  "${KBE_SOURCE_DIR}/server/cellapp/entity_component.cpp"
  "${KBE_SOURCE_DIR}/server/cellapp/ghost_manager.cpp"
  "${KBE_SOURCE_DIR}/server/cellapp/history_event.cpp"
  "${KBE_SOURCE_DIR}/server/cellapp/initprogress_handler.cpp"
  "${KBE_SOURCE_DIR}/server/cellapp/loadnavmesh_threadtasks.cpp"
  "${KBE_SOURCE_DIR}/server/cellapp/main.cpp"
  "${KBE_SOURCE_DIR}/server/cellapp/space.cpp"
  "${KBE_SOURCE_DIR}/server/cellapp/spacememory.cpp"
  "${KBE_SOURCE_DIR}/server/cellapp/spacememorys.cpp"
  "${KBE_SOURCE_DIR}/server/cellapp/space_viewer.cpp"
  "${KBE_SOURCE_DIR}/server/cellapp/move_controller.cpp"
  "${KBE_SOURCE_DIR}/server/cellapp/moveto_entity_handler.cpp"
  "${KBE_SOURCE_DIR}/server/cellapp/moveto_point_handler.cpp"
  "${KBE_SOURCE_DIR}/server/cellapp/navigate_handler.cpp"
  "${KBE_SOURCE_DIR}/server/cellapp/profile.cpp"
  "${KBE_SOURCE_DIR}/server/cellapp/proximity_controller.cpp"
  "${KBE_SOURCE_DIR}/server/cellapp/coordinate_node.cpp"
  "${KBE_SOURCE_DIR}/server/cellapp/coordinate_system.cpp"
  "${KBE_SOURCE_DIR}/server/cellapp/rotator_handler.cpp"
  "${KBE_SOURCE_DIR}/server/cellapp/range_trigger.cpp"
  "${KBE_SOURCE_DIR}/server/cellapp/range_trigger_node.cpp"
  "${KBE_SOURCE_DIR}/server/cellapp/real_entity_method.cpp"
  "${KBE_SOURCE_DIR}/server/cellapp/trap_trigger.cpp"
  "${KBE_SOURCE_DIR}/server/cellapp/turn_controller.cpp"
  "${KBE_SOURCE_DIR}/server/cellapp/updatable.cpp"
  "${KBE_SOURCE_DIR}/server/cellapp/updatables.cpp"
  "${KBE_SOURCE_DIR}/server/cellapp/watch_obj_pools.cpp"
  "${KBE_SOURCE_DIR}/server/cellapp/witness.cpp"
  "${KBE_SOURCE_DIR}/server/cellapp/witnessed_timeout_handler.cpp"
)

target_link_libraries(kbe_cellapp
  PRIVATE
    KBEngine::client_lib
    KBEngine::navigation
)

kbe_add_server_executable(kbe_dbmgr dbmgr KBE_DBMGR
  "${KBE_SOURCE_DIR}/server/dbmgr/buffered_dbtasks.cpp"
  "${KBE_SOURCE_DIR}/server/dbmgr/dbmgr.cpp"
  "${KBE_SOURCE_DIR}/server/dbmgr/dbmgr_interface.cpp"
  "${KBE_SOURCE_DIR}/server/dbmgr/dbtasks.cpp"
  "${KBE_SOURCE_DIR}/server/dbmgr/entity_component.cpp"
  "${KBE_SOURCE_DIR}/server/dbmgr/interfaces_handler.cpp"
  "${KBE_SOURCE_DIR}/server/dbmgr/main.cpp"
  "${KBE_SOURCE_DIR}/server/dbmgr/profile.cpp"
  "${KBE_SOURCE_DIR}/server/dbmgr/sync_app_datas_handler.cpp"
  "${KBE_SOURCE_DIR}/server/dbmgr/update_dblog_handler.cpp"
)

target_link_libraries(kbe_dbmgr
  PRIVATE
    KBEngine::db_runtime
)

kbe_add_tool_executable(kbe_interfaces interfaces KBE_INTERFACES
  "${KBE_SOURCE_DIR}/server/tools/interfaces/interfaces.cpp"
  "${KBE_SOURCE_DIR}/server/tools/interfaces/interfaces_tasks.cpp"
  "${KBE_SOURCE_DIR}/server/tools/interfaces/interfaces_interface.cpp"
  "${KBE_SOURCE_DIR}/server/tools/interfaces/orders.cpp"
  "${KBE_SOURCE_DIR}/server/tools/interfaces/profile.cpp"
  "${KBE_SOURCE_DIR}/server/tools/interfaces/main.cpp"
)

kbe_add_tool_executable(kbe_logger logger KBE_LOGGER
  "${KBE_SOURCE_DIR}/server/tools/logger/logger.cpp"
  "${KBE_SOURCE_DIR}/server/tools/logger/logger_interface.cpp"
  "${KBE_SOURCE_DIR}/server/tools/logger/logwatcher.cpp"
  "${KBE_SOURCE_DIR}/server/tools/logger/profile.cpp"
  "${KBE_SOURCE_DIR}/server/tools/logger/main.cpp"
)

kbe_add_tool_executable(kbe_kbcmd kbcmd KBE_CMD
  "${KBE_SOURCE_DIR}/server/tools/kbcmd/client_sdk.cpp"
  "${KBE_SOURCE_DIR}/server/tools/kbcmd/client_sdk_unity.cpp"
  "${KBE_SOURCE_DIR}/server/tools/kbcmd/client_sdk_ue4.cpp"
  "${KBE_SOURCE_DIR}/server/tools/kbcmd/server_assets.cpp"
  "${KBE_SOURCE_DIR}/server/tools/kbcmd/entity_component.cpp"
  "${KBE_SOURCE_DIR}/server/tools/kbcmd/kbcmd.cpp"
  "${KBE_SOURCE_DIR}/server/tools/kbcmd/kbcmd_interface.cpp"
  "${KBE_SOURCE_DIR}/server/tools/kbcmd/profile.cpp"
  "${KBE_SOURCE_DIR}/server/tools/kbcmd/main.cpp"
)

kbe_add_client_tool_executable(kbe_bots bots KBE_BOTS
  "${KBE_SOURCE_DIR}/server/tools/bots/bots.cpp"
  "${KBE_SOURCE_DIR}/server/tools/bots/bots_interface.cpp"
  "${KBE_SOURCE_DIR}/server/tools/bots/clientobject.cpp"
  "${KBE_SOURCE_DIR}/server/tools/bots/create_and_login_handler.cpp"
  "${KBE_SOURCE_DIR}/server/tools/bots/profile.cpp"
  "${KBE_SOURCE_DIR}/server/tools/bots/pybots.cpp"
  "${KBE_SOURCE_DIR}/server/tools/bots/kcp_packet_receiver_ex.cpp"
  "${KBE_SOURCE_DIR}/server/tools/bots/kcp_packet_sender_ex.cpp"
  "${KBE_SOURCE_DIR}/server/tools/bots/tcp_packet_receiver_ex.cpp"
  "${KBE_SOURCE_DIR}/server/tools/bots/tcp_packet_sender_ex.cpp"
  "${KBE_SOURCE_DIR}/server/tools/bots/main.cpp"
)
