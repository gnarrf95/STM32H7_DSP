#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int wm8994_Reset(void);
int wm8994_ConfigureInterface(void);
int wm8994_RouteInput(void);
int wm8994_RouteOutput(bool enableDac, bool enablePassthrough);

int wm8994_SetInputGain(float gain, bool mute);
int wm8994_SetOutputGain(float gain, bool mute);

int wm8994_EnableInput(bool enable);
int wm8994_EnableOutput(bool enable);

#ifdef __cplusplus
}
#endif
