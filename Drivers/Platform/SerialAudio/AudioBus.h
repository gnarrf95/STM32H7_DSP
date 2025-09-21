#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define AUDIOBUS_BUFFER_LENGTH 128

int AudioBus_Init(void);

bool AudioBus_IsRunning(void);
int AudioBus_Start(void);
int AudioBus_Stop(void);

bool AudioBus_IsBufferReady(void);

int AudioBus_GetInputBuffer(float* pBuffer);
int AudioBus_SetOutputBuffer(const float* pBuffer);

#ifdef __cplusplus
}
#endif
