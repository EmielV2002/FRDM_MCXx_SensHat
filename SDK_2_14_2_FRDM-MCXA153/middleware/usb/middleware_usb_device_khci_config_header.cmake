# Add set(CONFIG_USE_middleware_usb_device_khci_config_header true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

if((CONFIG_DEVICE_ID STREQUAL MCXA142 OR CONFIG_DEVICE_ID STREQUAL MCXA143 OR CONFIG_DEVICE_ID STREQUAL MCXA152 OR CONFIG_DEVICE_ID STREQUAL MCXA153))

add_config_file(${CMAKE_CURRENT_LIST_DIR}/output/npw/device_config/khci/usb_device_config.h ${CMAKE_CURRENT_LIST_DIR}/output/npw/device_config/khci middleware_usb_device_khci_config_header)

else()

message(SEND_ERROR "middleware_usb_device_khci_config_header dependency does not meet, please check ${CMAKE_CURRENT_LIST_FILE}.")

endif()
