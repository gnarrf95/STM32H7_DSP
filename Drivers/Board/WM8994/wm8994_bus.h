#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void wm8994_Delay(uint32_t delayMs);

int wm8994_ReadRegister(uint16_t address, uint16_t* pData);
int wm8994_WriteRegister(uint16_t address, uint16_t value, bool validate);

int wm8994_SetRegisterBits(uint16_t address, uint16_t bitMask, bool validate);
int wm8994_ResetRegisterBits(uint16_t address, uint16_t bitMask, bool validate);

#ifdef __cplusplus
}
#endif
