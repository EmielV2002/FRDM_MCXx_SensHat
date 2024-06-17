# Add set(CONFIG_USE_middleware_motor_control_mcaa_cm33nodsp true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

if((CONFIG_CORE STREQUAL cm33) AND (CONFIG_FPU STREQUAL NO_FPU) AND CONFIG_USE_middleware_rtcesl_cm33nodsp)

if(CONFIG_TOOLCHAIN STREQUAL iar)
target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
  ${CMAKE_CURRENT_LIST_DIR}/pmsm/pmsm_frac/mc_algorithms/mcaa_lib/cm33nodsp_iar/Include
)
endif()

if(CONFIG_TOOLCHAIN STREQUAL mdk)
target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
  ${CMAKE_CURRENT_LIST_DIR}/pmsm/pmsm_frac/mc_algorithms/mcaa_lib/cm33nodsp_mdk/Include
)
endif()

if((CONFIG_TOOLCHAIN STREQUAL mcux OR CONFIG_TOOLCHAIN STREQUAL armgcc))
target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
  ${CMAKE_CURRENT_LIST_DIR}/pmsm/pmsm_frac/mc_algorithms/mcaa_lib/cm33nodsp_mcux/Include
)
endif()

if(CONFIG_TOOLCHAIN STREQUAL iar)
  target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE
    -Wl,--start-group
      ${CMAKE_CURRENT_LIST_DIR}/pmsm/pmsm_frac/mc_algorithms/mcaa_lib/cm33nodsp_iar/mcaa_lib.a
      -Wl,--end-group
  )
endif()

if(CONFIG_TOOLCHAIN STREQUAL mdk)
  target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE
    -Wl,--start-group
      ${CMAKE_CURRENT_LIST_DIR}/pmsm/pmsm_frac/mc_algorithms/mcaa_lib/cm33nodsp_mdk/mcaa_lib.lib
      -Wl,--end-group
  )
endif()

if((CONFIG_TOOLCHAIN STREQUAL mcux OR CONFIG_TOOLCHAIN STREQUAL armgcc))
  target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE
    -Wl,--start-group
      ${CMAKE_CURRENT_LIST_DIR}/pmsm/pmsm_frac/mc_algorithms/mcaa_lib/cm33nodsp_mcux/libmcaa_lib.a
      -Wl,--end-group
  )
endif()

else()

message(SEND_ERROR "middleware_motor_control_mcaa_cm33nodsp dependency does not meet, please check ${CMAKE_CURRENT_LIST_FILE}.")

endif()
