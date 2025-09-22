#include "AudioBus.h"

#include <stm32h7xx.h>

#include <stddef.h>
#include <stdint.h>

#include <System/Memory.h>

// DMA Buffer Size: BufferSize * 2 (Stereo) * 2 (Double Buffering)
static MEM_DMA_D1 int16_t gAudioRxBuffer[AUDIOBUS_BUFFER_LENGTH * 4];
static MEM_DMA_D1 int16_t gAudioTxBuffer[AUDIOBUS_BUFFER_LENGTH * 4];

static volatile int16_t* gpCurrentInputBuffer = NULL;
static volatile int16_t* gpCurrentOutputBuffer = NULL;
static volatile bool gNewBufferReady = false;

//------------------------------------------------------------------------------
//
int AudioBus_Init(void)
{
	/**
	 * ADC Instance: SAI1_B
	 * DAC Instance: SAI1_A
	 * 
	 * MCLKA: 	PG7 AF6
	 * SCKA: 	PE5 AF6
	 * FSA: 	PE4 AF6
	 * SDA: 	PE6 AF6
	 * SDB: 	PE3 AF6
	 * 
	 * ADC DMA: DMA1 Stream2
	 * DAC DMA: DMA1 Stream3
	 * 
	 * Bus: APB2
	 * Clock: PLL2P (Configurable)
	 */

	// Clock
	RCC->D2CCIP1R &= ~RCC_D2CCIP1R_SAI1SEL_Msk;
	RCC->D2CCIP1R |= 0b001 << RCC_D2CCIP1R_SAI1SEL_Pos;

	RCC->APB2ENR |= RCC_APB2ENR_SAI1EN;

	// GPIO
	GPIOE->MODER &= ~(GPIO_MODER_MODER3_Msk | GPIO_MODER_MODER4_Msk | GPIO_MODER_MODER5_Msk | GPIO_MODER_MODER6_Msk);
	GPIOE->MODER |= (0b10 << GPIO_MODER_MODER3_Pos) | (0b10 << GPIO_MODER_MODER4_Pos) | (0b10 << GPIO_MODER_MODER5_Pos) | (0b10 << GPIO_MODER_MODER6_Pos);
	GPIOE->AFR[0] &= ~(GPIO_AFRL_AFSEL3_Msk | GPIO_AFRL_AFSEL4_Msk | GPIO_AFRL_AFSEL5_Msk | GPIO_AFRL_AFSEL6_Msk);
	GPIOE->AFR[0] |= (6 << GPIO_AFRL_AFSEL3_Pos) | (6 << GPIO_AFRL_AFSEL4_Pos) | (6 << GPIO_AFRL_AFSEL5_Pos) | (6 << GPIO_AFRL_AFSEL6_Pos);

	GPIOG->MODER &= ~GPIO_MODER_MODER7_Msk;
	GPIOG->MODER |= 0b10 << GPIO_MODER_MODER7_Pos;
	GPIOG->AFR[0] &= ~GPIO_AFRL_AFSEL7_Msk;
	GPIOG->AFR[0] |= 6 << GPIO_AFRL_AFSEL7_Pos;

	// Reset Peripheral
	SAI1_Block_A->CR1 &= ~SAI_xCR1_SAIEN;
	while ((SAI1_Block_A->CR1 & SAI_xCR1_SAIEN) != 0);

	SAI1_Block_B->CR2 &= ~SAI_xCR1_SAIEN;
	while ((SAI1_Block_B->CR1 & SAI_xCR1_SAIEN) != 0);

	SAI1->PDMCR = 0;
	SAI1->GCR = 0;

	SAI1_Block_A->CR1 = 0;
	SAI1_Block_A->CR2 = 0;
	SAI1_Block_A->FRCR = 0;
	SAI1_Block_A->SLOTR = 0;

	SAI1_Block_B->CR1 = 0;
	SAI1_Block_B->CR2 = 0;
	SAI1_Block_B->FRCR = 0;
	SAI1_Block_B->SLOTR = 0;

	// DMA Init
	RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;

	DMA1_Stream2->CR = 0;
	DMA1_Stream3->CR = 0;

	DMAMUX1_Channel2->CCR = 88 << DMAMUX_CxCR_DMAREQ_ID_Pos;
	DMAMUX1_Channel3->CCR = 87 << DMAMUX_CxCR_DMAREQ_ID_Pos;

	NVIC_EnableIRQ(DMA1_Stream2_IRQn);
	NVIC_EnableIRQ(DMA1_Stream3_IRQn);

	// SAI Block A (Tx)
	SAI1_Block_A->CR1 |= 0b100 << SAI_xCR1_DS_Pos;
	SAI1_Block_A->CR1 |= SAI_xCR1_OUTDRIV;
	SAI1_Block_A->CR1 |= 1 << SAI_xCR1_MCKDIV_Pos;

	SAI1_Block_A->CR2 |= 0b001 << SAI_xCR2_FTH_Pos;

	SAI1_Block_A->FRCR |= (64-1) << SAI_xFRCR_FRL_Pos;
	SAI1_Block_A->FRCR |= SAI_xFRCR_FSOFF;
	SAI1_Block_A->FRCR |= SAI_xFRCR_FSDEF;
	SAI1_Block_A->FRCR |= (32-1) << SAI_xFRCR_FSALL_Pos;

	SAI1_Block_A->SLOTR |= 0b0101 << SAI_xSLOTR_SLOTEN_Pos;
	SAI1_Block_A->SLOTR |= (4-1) << SAI_xSLOTR_NBSLOT_Pos;

	// SAI Block B (Rx)
	SAI1_Block_B->CR1 |= 0b11 << SAI_xCR1_MODE_Pos;
	SAI1_Block_B->CR1 |= 0b100 << SAI_xCR1_DS_Pos;
	SAI1_Block_B->CR1 |= SAI_xCR1_CKSTR;
	SAI1_Block_B->CR1 |= 0b01 << SAI_xCR1_SYNCEN_Pos;
	SAI1_Block_B->CR1 |= 1 << SAI_xCR1_MCKDIV_Pos;

	SAI1_Block_B->CR2 |= 0b001 << SAI_xCR2_FTH_Pos;
	SAI1_Block_B->CR2 |= SAI_xCR2_TRIS;

	SAI1_Block_B->FRCR |= (64-1) << SAI_xFRCR_FRL_Pos;
	SAI1_Block_B->FRCR |= SAI_xFRCR_FSOFF;
	SAI1_Block_B->FRCR |= SAI_xFRCR_FSDEF;
	SAI1_Block_B->FRCR |= (32-1) << SAI_xFRCR_FSALL_Pos;

	SAI1_Block_B->SLOTR |= 0b0101 << SAI_xSLOTR_SLOTEN_Pos;
	SAI1_Block_B->SLOTR |= (4-1) << SAI_xSLOTR_NBSLOT_Pos;

	SAI1_Block_B->CR1 |= SAI_xCR1_SAIEN;
	SAI1_Block_A->CR1 |= SAI_xCR1_SAIEN;

	return 0;
}

//------------------------------------------------------------------------------
//
bool AudioBus_IsRunning(void)
{
	return (SAI1_Block_A->CR1 & SAI_xCR1_DMAEN) ? true : false;
}

//------------------------------------------------------------------------------
//
int AudioBus_Start(void)
{
	// Start RX
	DMA1_Stream2->CR = 0;
	DMA1_Stream2->CR |= 0b10 << DMA_SxCR_PL_Pos;
	DMA1_Stream2->CR |= 0b01 << DMA_SxCR_MSIZE_Pos;
	DMA1_Stream2->CR |= 0b01 << DMA_SxCR_PSIZE_Pos;
	DMA1_Stream2->CR |= DMA_SxCR_MINC;
	DMA1_Stream2->CR |= DMA_SxCR_CIRC;
	DMA1_Stream2->CR |= DMA_SxCR_TCIE;
	DMA1_Stream2->CR |= DMA_SxCR_HTIE;

	DMA1_Stream2->NDTR = (uint32_t)(AUDIOBUS_BUFFER_LENGTH * 4);
	DMA1_Stream2->M0AR = (uint32_t)gAudioRxBuffer;
	DMA1_Stream2->PAR = (uint32_t)&SAI1_Block_B->DR;

	// Start TX
	DMA1_Stream3->CR = 0;
	DMA1_Stream3->CR |= 0b10 << DMA_SxCR_PL_Pos;
	DMA1_Stream3->CR |= 0b01 << DMA_SxCR_MSIZE_Pos;
	DMA1_Stream3->CR |= 0b01 << DMA_SxCR_PSIZE_Pos;
	DMA1_Stream3->CR |= DMA_SxCR_MINC;
	DMA1_Stream3->CR |= DMA_SxCR_CIRC;
	DMA1_Stream3->CR |= 0b01 << DMA_SxCR_DIR_Pos;
	DMA1_Stream3->CR |= DMA_SxCR_TCIE;
	DMA1_Stream3->CR |= DMA_SxCR_HTIE;

	DMA1_Stream3->NDTR = (uint32_t)(AUDIOBUS_BUFFER_LENGTH * 4);
	DMA1_Stream3->M0AR = (uint32_t)gAudioTxBuffer;
	DMA1_Stream3->PAR = (uint32_t)&SAI1_Block_A->DR;

	DMA1->LIFCR |= DMA_LIFCR_CFEIF2 | DMA_LIFCR_CDMEIF2 | DMA_LIFCR_CTEIF2 | DMA_LIFCR_CHTIF2 | DMA_LIFCR_CTCIF2;
	DMA1->LIFCR |= DMA_LIFCR_CFEIF3 | DMA_LIFCR_CDMEIF3 | DMA_LIFCR_CTEIF3 | DMA_LIFCR_CHTIF3 | DMA_LIFCR_CTCIF3;

	DMA1_Stream2->CR |= DMA_SxCR_EN;
	DMA1_Stream3->CR |= DMA_SxCR_EN;

	SAI1_Block_B->CR1 |= SAI_xCR1_DMAEN;
	SAI1_Block_A->CR1 |= SAI_xCR1_DMAEN;

	return 0;
}

//------------------------------------------------------------------------------
//
int AudioBus_Stop(void)
{
	SAI1_Block_B->CR1 &= ~SAI_xCR1_DMAEN;
	SAI1_Block_A->CR1 &= ~SAI_xCR1_DMAEN;

	DMA1_Stream2->CR &= ~DMA_SxCR_EN;
	DMA1_Stream3->CR &= ~DMA_SxCR_EN;

	return 0;
}

//------------------------------------------------------------------------------
//
bool AudioBus_IsBufferReady(void)
{
	return gNewBufferReady;
}

//------------------------------------------------------------------------------
//
int AudioBus_GetInputBuffer(float* pBuffer)
{
	if ((gNewBufferReady == false) || (gpCurrentInputBuffer == NULL) || (pBuffer == NULL))
	{
		return -1;
	}

	for (uint16_t ctr = 0; ctr < AUDIOBUS_BUFFER_LENGTH; ctr++)
	{
		pBuffer[ctr] = (float)gpCurrentInputBuffer[ctr * 2] / (float)INT16_MAX;
	}

	gNewBufferReady = false;
	return 0;
}

//------------------------------------------------------------------------------
//
int AudioBus_SetOutputBuffer(const float* pBuffer)
{
	if ((gpCurrentOutputBuffer == NULL) || (pBuffer == NULL))
	{
		return -1;
	}

	for (uint16_t ctr = 0; ctr < AUDIOBUS_BUFFER_LENGTH; ctr++)
	{
		const int16_t sample = (int16_t)(pBuffer[ctr] * (float)INT16_MAX);

		gpCurrentOutputBuffer[ctr * 2] = sample;
		gpCurrentOutputBuffer[ctr * 2 + 1] = sample;
	}

	return 0;
}



//------------------------------------------------------------------------------
//
void DMA1_Stream2_IRQHandler(void)
{
	// Handle Buffers at Rx

	if (DMA1->LISR & DMA_LISR_HTIF2)
	{
		gpCurrentInputBuffer = gAudioRxBuffer;
		gpCurrentOutputBuffer = gAudioTxBuffer;
		gNewBufferReady = true;

		DMA1->LIFCR |= DMA_LIFCR_CHTIF2;
	}

	if (DMA1->LISR & DMA_LISR_TCIF2)
	{
		gpCurrentInputBuffer = &gAudioRxBuffer[AUDIOBUS_BUFFER_LENGTH * 2];
		gpCurrentOutputBuffer = &gAudioTxBuffer[AUDIOBUS_BUFFER_LENGTH * 2];
		gNewBufferReady = true;

		DMA1->LIFCR |= DMA_LIFCR_CTCIF2;
	}
}

//------------------------------------------------------------------------------
//
void DMA1_Stream3_IRQHandler(void)
{
	// Only Handle Flags

	if (DMA1->LISR & DMA_LISR_HTIF3)
	{
		DMA1->LIFCR |= DMA_LIFCR_CHTIF3;
	}

	if (DMA1->LISR & DMA_LISR_TCIF3)
	{
		DMA1->LIFCR |= DMA_LIFCR_CTCIF3;
	}
}
