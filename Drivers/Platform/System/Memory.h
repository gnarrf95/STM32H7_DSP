#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define MEM_DMA_D1	__attribute__((section(".d1dmabuffer")))
#define MEM_DMA_D3	__attribute__((section(".d3dmabuffer")))
#define MEM_AUDIO	__attribute__((section(".audiodata")))
#define MEM_VIDEO	__attribute__((section(".videobuffer")))
#define MEM_BULK	__attribute__((section(".bulkdata")))

void SystemMemory_InitSdRam(void);
void SystemMemory_InitMpu(void);

#ifdef __cplusplus
}
#endif
