#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    BUTTON_INDEX_USER,          // PC13
    BUTTON_INDEX_JOYSELECT,     // PK2
    BUTTON_INDEX_JOYDOWN,       // PK3
    BUTTON_INDEX_JOYLEFT,       // PK4
    BUTTON_INDEX_JOYRIGHT,      // PK5
    BUTTON_INDEX_JOYUP,         // PK6

    BUTTON_INDEX_LIMIT
} button_Index_t;

void button_Init(void);

bool button_Read(button_Index_t index);

#ifdef __cplusplus
}
#endif
