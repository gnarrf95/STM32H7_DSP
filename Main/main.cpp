#include <cstdint>

#include <stm32h7xx.h>

#include <System/Clock.h>
#include <System/Memory.h>

#include <Timer/TimerMicro.h>
#include <Serial/Serial.h>

static MEM_DMA_D1 const char gMsg[] = "Hello World!\r\n";

//------------------------------------------------------------------------------
//
extern "C" int main(void)
{
	SystemClock_Init();
	SystemMemory_InitSdRam();

	TimerMicro_Init();
	Serial_Init();

	RCC->AHB4ENR |= RCC_AHB4ENR_GPIOIEN;
	GPIOI->MODER &= ~GPIO_MODER_MODER12_Msk;
	GPIOI->MODER |= 0b01 << GPIO_MODER_MODER12_Pos;

	TimerMicro_t timer;
	TimerMicro_Reset(&timer);
	while (1)
	{
		if (TimerMicro_Check(&timer) >= 500000.0F)
		{
			TimerMicro_Reset(&timer);

			GPIOI->ODR ^= GPIO_ODR_OD12;
			Serial_Send((const uint8_t*)gMsg, sizeof(gMsg));
		}
	}

	return 0;
}
