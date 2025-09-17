#include "TimerMicro.h"

#include <stm32h7xx.h>
#include <stddef.h>

#define TIMERMICRO_TIMER_FREQUENCY_MHZ (200)
#define TIMERMICRO_COUNTER_MAX (TIM_CNT_CNT_Msk)

//------------------------------------------------------------------------------
//
int TimerMicro_Init(void)
{
	RCC->APB1LENR |= RCC_APB1LENR_TIM2EN;

	TIM2->CR1 = 0;
	TIM2->CR2 = 0;
	TIM2->SMCR = 0;
	TIM2->DIER = 0;

	TIM2->PSC = 0;
	TIM2->ARR = 0xFFFFFFFF;
	TIM2->CNT = 0;

	TIM2->EGR = TIM_EGR_UG;
	TIM2->CR1 |= TIM_CR1_CEN;

	return 0;
}

//------------------------------------------------------------------------------
//
uint64_t TimerMicro_GetTicks(void)
{
	return (uint64_t)TIM2->CNT;
}

//------------------------------------------------------------------------------
//
uint64_t TimerMicro_GetTimestampUs(void)
{
	return (uint64_t)(TIM2->CNT / TIMERMICRO_TIMER_FREQUENCY_MHZ);
}

//------------------------------------------------------------------------------
//
float TimerMicro_Reset(TimerMicro_t* pTimer)
{
	const uint64_t ticksNow = TimerMicro_GetTicks();
	if (pTimer == NULL)
	{
		return 0.0F;
	}

	uint64_t timeDiff;
	if (ticksNow < pTimer->timestamp)
	{
		timeDiff = (ticksNow + TIMERMICRO_COUNTER_MAX) - pTimer->timestamp;
	}
	else
	{
		timeDiff = ticksNow - pTimer->timestamp;
	}

	pTimer->timestamp = ticksNow;
	return (float)timeDiff / (float)TIMERMICRO_TIMER_FREQUENCY_MHZ;
}

//------------------------------------------------------------------------------
//
float TimerMicro_Check(TimerMicro_t* pTimer)
{
	const uint64_t ticksNow = TimerMicro_GetTicks();
	if (pTimer == NULL)
	{
		return 0.0F;
	}

	uint64_t timeDiff;
	if (ticksNow < pTimer->timestamp)
	{
		timeDiff = (ticksNow + TIMERMICRO_COUNTER_MAX) - pTimer->timestamp;
	}
	else
	{
		timeDiff = ticksNow - pTimer->timestamp;
	}

	return (float)timeDiff / (float)TIMERMICRO_TIMER_FREQUENCY_MHZ;
}
