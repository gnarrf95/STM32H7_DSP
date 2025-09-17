#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	uint64_t timestamp;
} TimerMicro_t;

int TimerMicro_Init(void);

uint64_t TimerMicro_GetTicks(void);
uint64_t TimerMicro_GetTimestampUs(void);

float TimerMicro_Reset(TimerMicro_t* pTimer);
float TimerMicro_Check(TimerMicro_t* pTimer);

#ifdef __cplusplus
}
#endif
