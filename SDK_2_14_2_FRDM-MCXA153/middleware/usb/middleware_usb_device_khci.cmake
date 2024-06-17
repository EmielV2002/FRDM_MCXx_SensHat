# Add set(CONFIG_USE_middleware_usb_device_khci true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

if((CONFIG_DEVICE_ID STREQUAL MCXA142 OR CONFIG_DEVICE_ID STREQUAL MCXA143 OR CONFIG_DEVICE_ID STREQUAL MCXA152 OR CONFIG_DEVICE_ID STREQUAL MCXA153) AND CONFIG_USE_middleware_usb_device_khci_config_header AND CONFIG_USE_middleware_usb_device_common_header)

target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}/device/usb_device_khci.c
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
  ${CMAKE_CURRENT_LIST_DIR}/device
  ${CMAKE_CURRENT_LIST_DIR}/include
)

else()

message(SEND_ERROR "middleware_usb_device_khci dependency does not meet, please check ${CMAKE_CURRENT_LIST_FILE}.")

endif()
