# Add set(CONFIG_USE_CMSIS_RTOS2_NonSecure true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

if(CONFIG_USE_CMSIS_Device_API_OSTick AND CONFIG_USE_CMSIS_Device_API_RTOS2)

target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}/RTX/Source/rtx_lib.c
  ${CMAKE_CURRENT_LIST_DIR}/RTX/Config/RTX_Config.c
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
  ${CMAKE_CURRENT_LIST_DIR}/RTX/Source
  ${CMAKE_CURRENT_LIST_DIR}/RTX/Include
  ${CMAKE_CURRENT_LIST_DIR}/RTX/Config
)

else()

message(SEND_ERROR "CMSIS_RTOS2_NonSecure dependency does not meet, please check ${CMAKE_CURRENT_LIST_FILE}.")

endif()
