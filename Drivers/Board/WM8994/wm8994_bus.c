#include "wm8994_bus.h"

#include <System/Memory.h>
#include <Timer/TimerMicro.h>
#include <Bus/ControlBus.h>

#include <stddef.h>

#define WM8994_BUS_ADDRESS 0x34

//------------------------------------------------------------------------------
//
void wm8994_Delay(uint32_t delayMs)
{
	const uint64_t timeStart = TimerMicro_GetTimestampUs();
	const uint64_t intervalUs = (uint64_t)delayMs * 1000;

	while ((TimerMicro_GetTimestampUs() - timeStart) >= intervalUs);
}

//------------------------------------------------------------------------------
//
int wm8994_ReadRegister(uint16_t address, uint16_t* pData)
{
	if (pData == NULL)
	{
		return -1;
	}

	wm8994_Delay(1);

	uint8_t busBuffer[2] = {
		(uint8_t)(address >> 8),
		(uint8_t)(address)
	};

	if (ControlBus_SendBlocking(WM8994_BUS_ADDRESS, busBuffer, 2, false) < 0)
	{
		return -1;
	}

	if (ControlBus_RecvBlocking(WM8994_BUS_ADDRESS, busBuffer, 2, true) < 0)
	{
		return -1;
	}

	*pData = ((uint16_t)busBuffer[0] << 8) | (uint16_t)busBuffer[1];
	return 0;
}

//------------------------------------------------------------------------------
//
int wm8994_WriteRegister(uint16_t address, uint16_t value, bool validate)
{
	wm8994_Delay(1);

	uint8_t busBuffer[4] = {
		(uint8_t)(address >> 8),
		(uint8_t)(address),
		(uint8_t)(value >> 8),
		(uint8_t)(value)
	};

	if (ControlBus_SendBlocking(WM8994_BUS_ADDRESS, busBuffer, 4, true) < 0)
	{
		return -1;
	}

	if (validate)
	{
		uint16_t check = 0;
		if (wm8994_ReadRegister(address, &check) < 0)
		{
			return -1;
		}

		if (check != value)
		{
			return -1;
		}
	}

	return 0;
}

//------------------------------------------------------------------------------
//
int wm8994_SetRegisterBits(uint16_t address, uint16_t bitMask, bool validate)
{
	uint16_t registerBuffer = 0;
	if (wm8994_ReadRegister(address, &registerBuffer) < 0)
	{
		return -1;
	}

	registerBuffer |= bitMask;

	if (wm8994_WriteRegister(address, registerBuffer, validate) < 0)
	{
		return -1;
	}

	return 0;
}

//------------------------------------------------------------------------------
//
int wm8994_ResetRegisterBits(uint16_t address, uint16_t bitMask, bool validate)
{
	uint16_t registerBuffer = 0;
	if (wm8994_ReadRegister(address, &registerBuffer) < 0)
	{
		return -1;
	}

	registerBuffer &= ~bitMask;

	if (wm8994_WriteRegister(address, registerBuffer, validate) < 0)
	{
		return -1;
	}

	return 0;
}
