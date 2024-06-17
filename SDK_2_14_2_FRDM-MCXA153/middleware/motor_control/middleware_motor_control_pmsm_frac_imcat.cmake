# Add set(CONFIG_USE_middleware_motor_control_pmsm_frac_imcat true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

if((CONFIG_DEVICE_ID STREQUAL MCXA153))

target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
  ${CMAKE_CURRENT_LIST_DIR}/freemaster/pmsm_frac/mcat
)

else()

message(SEND_ERROR "middleware_motor_control_pmsm_frac_imcat dependency does not meet, please check ${CMAKE_CURRENT_LIST_FILE}.")

endif()
