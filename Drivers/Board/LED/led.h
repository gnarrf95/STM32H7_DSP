#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    LED_INDEX_GREEN,    // PI12
    LED_INDEX_ORANGE,   // PI13
    LED_INDEX_RED,      // PI14
    LED_INDEX_BLUE,     // PI15

    LED_INDEX_LIMIT
} led_Index_t;

void led_Init(void);

void led_Write(led_Index_t index, bool set);
void led_Toggle(led_Index_t index);

#ifdef __cplusplus
}
#endif
