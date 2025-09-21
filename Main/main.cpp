#include <cstdint>

#include <stm32h7xx.h>

#include <System/Clock.h>
#include <System/Memory.h>

#include <Timer/TimerMicro.h>
#include <Serial/Serial.h>
#include <Bus/ControlBus.h>
#include <SerialAudio/AudioBus.h>

#include <WM8994/wm8994.h>

static MEM_DMA_D1 const char gMsg[] = "Hello World!\r\n";

static float gInputBuffer[AUDIOBUS_BUFFER_LENGTH];
static float gOutputBuffer[AUDIOBUS_BUFFER_LENGTH];

//------------------------------------------------------------------------------
//
extern "C" int main(void)
{
	SystemClock_Init();
	SystemMemory_InitSdRam();

	TimerMicro_Init();
	Serial_Init();
	ControlBus_Init();

	if (AudioBus_Init() < 0)
	{
		while (1);
	}

	wm8994_Reset();
	wm8994_RouteInput();
	wm8994_RouteOutput(true, false);
	wm8994_ConfigureInterface();
	wm8994_EnableOutput(true);
	wm8994_EnableInput(true);

	wm8994_SetInputGain(0.0F, false);
	wm8994_SetOutputGain(-12.0F, false);

	if (AudioBus_Start() < 0)
	{
		while (1);
	}

	RCC->AHB4ENR |= RCC_AHB4ENR_GPIOIEN;
	GPIOI->MODER &= ~GPIO_MODER_MODER12_Msk;
	GPIOI->MODER |= 0b01 << GPIO_MODER_MODER12_Pos;

	TimerMicro_t timer;
	TimerMicro_t test;

	TimerMicro_Reset(&timer);
	TimerMicro_Reset(&test);
	while (1)
	{
		if (AudioBus_IsBufferReady() && (AudioBus_GetInputBuffer(gInputBuffer) == 0))
		{
			for (uint16_t ctr = 0; ctr < AUDIOBUS_BUFFER_LENGTH; ctr++)
			{
				float sample = gInputBuffer[ctr];

				if (sample >= 0.05F)
				{
					sample = 0.05F;
				}
				else if (sample <= -0.05F)
				{
					sample = -0.05F;
				}

				gOutputBuffer[ctr] = sample;
			}

			AudioBus_SetOutputBuffer(gOutputBuffer);
		}

		if (TimerMicro_Check(&timer) >= 500000.0F)
		{
			TimerMicro_Reset(&timer);

			GPIOI->ODR ^= GPIO_ODR_OD12;
			Serial_Send((const uint8_t*)gMsg, sizeof(gMsg));
		}

		if (TimerMicro_Check(&test) >= 2000000.0F)
		{
			TimerMicro_Reset(&test);

			if (AudioBus_IsRunning())
			{
				AudioBus_Stop();
			}
			else
			{
				AudioBus_Start();
			}
		}
	}

	return 0;
}
