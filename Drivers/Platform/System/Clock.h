#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
	SYSTEMCLOCK_AUDIOSAMPLINGRATE_ODD,	/* 44.1kHz, 88.2kHz, 176.4kHz */
	SYSTEMCLOCK_AUDIOSAMPLINGRATE_EVEN	/* 48kHz, 96kHz, 192kHz */
} SystemClock_AudioSamplingRate_e;

void SystemClock_Init(void);
void SystemClock_SelectAudioClock(SystemClock_AudioSamplingRate_e samplingRate);

#ifdef __cplusplus
}
#endif
