set(SOURCES
	LED/led.c
	button/button.c

	WM8994/wm8994_bus.c
	WM8994/wm8994.c
)

add_library(Board STATIC ${SOURCES})
target_include_directories(Board PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(Board PUBLIC Platform)