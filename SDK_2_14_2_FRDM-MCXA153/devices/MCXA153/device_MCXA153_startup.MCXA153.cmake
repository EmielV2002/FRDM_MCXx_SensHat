# Add set(CONFIG_USE_device_MCXA153_startup true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

if(CONFIG_USE_device_MCXA153_system)

if(CONFIG_TOOLCHAIN STREQUAL armgcc)
  target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
      ${CMAKE_CURRENT_LIST_DIR}/gcc/startup_MCXA153.S
  )
endif()

if(CONFIG_TOOLCHAIN STREQUAL mcux)
  target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
      ${CMAKE_CURRENT_LIST_DIR}/mcuxpresso/startup_mcxa153.c
      ${CMAKE_CURRENT_LIST_DIR}/mcuxpresso/startup_mcxa153.cpp
  )
endif()

if(CONFIG_TOOLCHAIN STREQUAL iar)
  add_config_file(${CMAKE_CURRENT_LIST_DIR}/iar/startup_MCXA153.s "" device_MCXA153_startup.MCXA153)
endif()

if(CONFIG_TOOLCHAIN STREQUAL mdk)
  add_config_file(${CMAKE_CURRENT_LIST_DIR}/arm/startup_MCXA153.S "" device_MCXA153_startup.MCXA153)
endif()

else()

message(SEND_ERROR "device_MCXA153_startup.MCXA153 dependency does not meet, please check ${CMAKE_CURRENT_LIST_FILE}.")

endif()
