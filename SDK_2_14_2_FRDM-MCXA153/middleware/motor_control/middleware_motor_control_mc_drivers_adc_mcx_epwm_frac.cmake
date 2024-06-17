# Add set(CONFIG_USE_middleware_motor_control_mc_drivers_adc_mcx_epwm_frac true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

if((CONFIG_DEVICE_ID STREQUAL MCXA153) AND (CONFIG_CORE STREQUAL cm33))

target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}/pmsm/pmsm_frac/mc_drivers/mcdrv_adc_mcax.c
  ${CMAKE_CURRENT_LIST_DIR}/pmsm/pmsm_frac/mc_drivers/mcdrv_pwm3ph_epwm.c
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
  ${CMAKE_CURRENT_LIST_DIR}/pmsm/pmsm_frac/mc_drivers
)

else()

message(SEND_ERROR "middleware_motor_control_mc_drivers_adc_mcx_epwm_frac dependency does not meet, please check ${CMAKE_CURRENT_LIST_FILE}.")

endif()
