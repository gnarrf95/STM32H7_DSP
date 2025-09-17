set (CMAKE_SYSTEM_PROCESSOR "arm" CACHE STRING "")
set (CMAKE_SYSTEM_NAME "Generic" CACHE STRING "")

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# Some default GCC settings
# arm-none-eabi- must be part of path environment
set(TOOLCHAIN_PREFIX                "arm-none-eabi-")

# Define compiler settings
set(CMAKE_C_COMPILER                "${TOOLCHAIN_PREFIX}gcc")
set(CMAKE_ASM_COMPILER              "${TOOLCHAIN_PREFIX}gcc")
set(CMAKE_CXX_COMPILER				"${TOOLCHAIN_PREFIX}g++")
set(CMAKE_AR						"${TOOLCHAIN_PREFIX}ar")
set(CMAKE_LINKER					"${TOOLCHAIN_PREFIX}ld")
set(CMAKE_OBJCOPY                   "${TOOLCHAIN_PREFIX}objcopy")
set(CMAKE_SIZE                      "${TOOLCHAIN_PREFIX}size")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
