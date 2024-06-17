# config to select component, the format is CONFIG_USE_${component}
# Please refer to cmake files below to get available components:
#  ${SdkRootDirPath}/devices/MCXA153/all_lib_device.cmake

set(CONFIG_COMPILER gcc)
set(CONFIG_TOOLCHAIN armgcc)
set(CONFIG_USE_device_MCXA153_CMSIS true)
set(CONFIG_USE_device_MCXA153_startup true)
set(CONFIG_USE_CMSIS_Include_core_cm true)
set(CONFIG_USE_device_MCXA153_system true)
set(CONFIG_CORE cm33)
set(CONFIG_DEVICE MCXA153)
set(CONFIG_DEVICE_ID MCXA153)
set(CONFIG_FPU NO_FPU)
set(CONFIG_DSP NO_DSP)

# Utilities which is needed for particular toolchain like the SBRK function
# required to address limitation between HEAP and STACK in GCC toolchain library.
set(CONFIG_USE_utilities_misc_utilities true)
