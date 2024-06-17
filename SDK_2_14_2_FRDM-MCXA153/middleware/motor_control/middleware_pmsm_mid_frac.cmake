# Add set(CONFIG_USE_middleware_pmsm_mid_frac true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

if((CONFIG_USE_middleware_motor_control_mcaa_cm33nodsp) AND CONFIG_USE_middleware_motor_control_pmsm_frac_template AND (CONFIG_CORE STREQUAL cm33))

target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}/pmsm/pmsm_frac/mc_identification/mid_sm_states.c
  ${CMAKE_CURRENT_LIST_DIR}/pmsm/pmsm_frac/mc_identification/pp_measure.c
  ${CMAKE_CURRENT_LIST_DIR}/pmsm/pmsm_frac/mc_identification/m1_mid_switch.c
  ${CMAKE_CURRENT_LIST_DIR}/pmsm/pmsm_frac/mc_algorithms/pmsm_control.c
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
  ${CMAKE_CURRENT_LIST_DIR}/pmsm/pmsm_frac/mc_identification
  ${CMAKE_CURRENT_LIST_DIR}/pmsm/pmsm_frac/mc_algorithms
  ${CMAKE_CURRENT_LIST_DIR}/pmsm/pmsm_frac
)

if(CONFIG_USE_COMPONENT_CONFIGURATION)
  message("===>Import configuration from ${CMAKE_CURRENT_LIST_FILE}")

  target_compile_definitions(${MCUX_SDK_PROJECT_NAME} PUBLIC
    -DMID_EN
  )

endif()

else()

message(SEND_ERROR "middleware_pmsm_mid_frac dependency does not meet, please check ${CMAKE_CURRENT_LIST_FILE}.")

endif()
