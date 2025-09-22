set(INCLUDES
	CMSIS_5/CMSIS/Core/Include
	cmsis-device-h7/Include
	CMSIS_5/CMSIS/DSP/Include
)

set(DEFINES
	STM32H747xx
	CORE_CM7

	ARM_MATH_CM7
)

set(LIB_DIRS
	CMSIS_5/CMSIS/DSP/Lib/GCC
)

set(LIBRARIES
	arm_cortexM7lfsp_math
)

add_library(CMSIS INTERFACE ${SOURCES})
target_include_directories(CMSIS INTERFACE ${INCLUDES})
target_compile_definitions(CMSIS INTERFACE ${DEFINES})
target_link_directories(CMSIS INTERFACE ${LIB_DIRS})
target_link_libraries(CMSIS INTERFACE ${LIBRARIES})
