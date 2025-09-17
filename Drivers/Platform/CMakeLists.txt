set(SOURCES
	Startup/startup_stm32h747xihx.s
	Startup/Init.c

	System/Clock.c
	System/Memory.c

	Timer/TimerMicro.c
	Serial/Serial.c
	Bus/ControlBus.c
)

add_library(Platform STATIC ${SOURCES})
target_include_directories(Platform PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(Platform PUBLIC CMSIS)