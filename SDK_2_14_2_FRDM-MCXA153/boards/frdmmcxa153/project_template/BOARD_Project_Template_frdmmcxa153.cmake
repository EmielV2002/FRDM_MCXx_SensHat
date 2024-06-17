# Add set(CONFIG_USE_BOARD_Project_Template_frdmmcxa153 true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

if(CONFIG_USE_component_lpuart_adapter AND CONFIG_USE_driver_lpuart AND CONFIG_USE_driver_gpio AND CONFIG_USE_driver_reset AND CONFIG_USE_driver_clock AND CONFIG_USE_driver_mcx_spc AND CONFIG_USE_driver_port AND (CONFIG_DEVICE_ID STREQUAL MCXA153) AND CONFIG_USE_device_MCXA153_startup AND (CONFIG_BOARD STREQUAL frdmmcxa153) AND ((CONFIG_USE_component_serial_manager AND CONFIG_USE_utility_assert AND CONFIG_USE_utility_debug_console) OR (CONFIG_USE_utility_assert_lite AND CONFIG_USE_utility_debug_console_lite)))

add_config_file(${CMAKE_CURRENT_LIST_DIR}/board.h "" BOARD_Project_Template_frdmmcxa153)
add_config_file(${CMAKE_CURRENT_LIST_DIR}/board.c "" BOARD_Project_Template_frdmmcxa153)
add_config_file(${CMAKE_CURRENT_LIST_DIR}/clock_config.h ${CMAKE_CURRENT_LIST_DIR}/. BOARD_Project_Template_frdmmcxa153)
add_config_file(${CMAKE_CURRENT_LIST_DIR}/clock_config.c "" BOARD_Project_Template_frdmmcxa153)
add_config_file(${CMAKE_CURRENT_LIST_DIR}/pin_mux.h ${CMAKE_CURRENT_LIST_DIR}/. BOARD_Project_Template_frdmmcxa153)
add_config_file(${CMAKE_CURRENT_LIST_DIR}/pin_mux.c "" BOARD_Project_Template_frdmmcxa153)
add_config_file(${CMAKE_CURRENT_LIST_DIR}/peripherals.h ${CMAKE_CURRENT_LIST_DIR}/. BOARD_Project_Template_frdmmcxa153)
add_config_file(${CMAKE_CURRENT_LIST_DIR}/peripherals.c "" BOARD_Project_Template_frdmmcxa153)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
  ${CMAKE_CURRENT_LIST_DIR}/.
)

else()

message(SEND_ERROR "BOARD_Project_Template_frdmmcxa153 dependency does not meet, please check ${CMAKE_CURRENT_LIST_FILE}.")

endif()
