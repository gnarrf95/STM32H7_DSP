#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void Serial_Init(void);

bool Serial_TxBusy(void);
int Serial_Send(const uint8_t* pBuffer, uint16_t bufferSize);

#ifdef __cplusplus
}
#endif
