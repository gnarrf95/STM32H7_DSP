set(INCLUDES
	CMSIS_5/CMSIS/Core/Include
	cmsis-device-h7/Include
)

set(DEFINES
	STM32H747xx
	CORE_CM7
)

add_library(CMSIS INTERFACE)
target_include_directories(CMSIS INTERFACE ${INCLUDES})
target_compile_definitions(CMSIS INTERFACE ${DEFINES})
