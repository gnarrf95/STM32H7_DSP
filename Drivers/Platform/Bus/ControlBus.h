#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void(* ControlBus_TxComplete_Callback_t)(void);
typedef void(* ControlBus_RxComplete_Callback_t)(void);
typedef void(* ControlBus_BusFailure_Callback_t)(void);

void ControlBus_Init(void);

int ControlBus_Send(uint8_t address, const uint8_t* pDataBuffer, size_t dataSize, bool genStop, ControlBus_TxComplete_Callback_t pCpltCb, ControlBus_BusFailure_Callback_t pErrCb);
int ControlBus_Recv(uint8_t address, uint8_t* pDataBuffer, size_t dataSize, bool genStop, ControlBus_RxComplete_Callback_t pCpltCb, ControlBus_BusFailure_Callback_t pErrCb);

#ifdef __cplusplus
}
#endif
