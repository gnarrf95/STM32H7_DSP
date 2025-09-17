#include "Serial.h"

#include <stm32h7xx.h>

#include <stddef.h>

//------------------------------------------------------------------------------
//
void Serial_Init(void)
{
	/**
	 * UART Instance: USART1
	 * TX: PA9 AF7
	 * RX: PA10 AF7
	 * 
	 * TX DMA: DMA1 Stream0
	 * Rx DMA: DMA1 Stream0
	 * 
	 * Bus: APB2
	 * Clock: APB2 Clock (100 MHz)
	 */

	RCC->AHB4ENR |= RCC_AHB4ENR_GPIOAEN;

	GPIOA->MODER &= ~(GPIO_MODER_MODER9_Msk | GPIO_MODER_MODER10_Msk);
	GPIOA->MODER |= (0b10 << GPIO_MODER_MODER9_Pos) | (0b10 << GPIO_MODER_MODER10_Pos);
	GPIOA->AFR[1] &= ~(GPIO_AFRH_AFSEL9_Msk | GPIO_AFRH_AFSEL10_Msk);
	GPIOA->AFR[1] |= (7 << GPIO_AFRH_AFSEL9_Pos) | (7 << GPIO_AFRH_AFSEL10_Pos);



	RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;

	DMA1_Stream0->CR = 0;
	DMAMUX1_Channel0->CCR = 42 << DMAMUX_CxCR_DMAREQ_ID_Pos;

	NVIC_EnableIRQ(DMA1_Stream0_IRQn);



	RCC->D2CCIP2R &= ~RCC_D2CCIP2R_USART16SEL_Msk;

	RCC->APB2ENR |= RCC_APB2ENR_USART1EN;

	USART1->CR1 = 0;
	USART1->CR2 = 0;
	USART1->CR3 = 0;

	USART1->CR1 |= USART_CR1_TE;
	USART1->BRR = 868;

	USART1->CR1 |= USART_CR1_UE;
}

//------------------------------------------------------------------------------
//
bool Serial_TxBusy(void)
{
	return (DMA1_Stream0->CR & DMA_SxCR_EN);
}

//------------------------------------------------------------------------------
//
int Serial_Send(const uint8_t* pBuffer, uint16_t bufferSize)
{
	if (bufferSize == 0)
	{
		return 0;
	}
	if (Serial_TxBusy())
	{
		return -1;
	}
	if (pBuffer == NULL)
	{
		return -1;
	}
	if (bufferSize >= DMA_SxNDT_Msk)
	{
		return -1;
	}

	
	DMA1_Stream0->CR |= DMA_SxCR_TRBUFF;
	DMA1_Stream0->CR |= DMA_SxCR_MINC;
	DMA1_Stream0->CR |= 0b01 << DMA_SxCR_DIR_Pos;
	DMA1_Stream0->CR |= DMA_SxCR_TCIE | DMA_SxCR_TEIE;

	DMA1_Stream0->NDTR = bufferSize;
	DMA1_Stream0->M0AR = (uint32_t)pBuffer;
	DMA1_Stream0->PAR = (uint32_t)&USART1->TDR;

	DMA1->LIFCR |= DMA_LIFCR_CFEIF0 | DMA_LIFCR_CDMEIF0 | DMA_LIFCR_CTEIF0 | DMA_LIFCR_CHTIF0 | DMA_LIFCR_CTCIF0;

	DMA1_Stream0->CR |= DMA_SxCR_EN;
	USART1->CR3 |= USART_CR3_DMAT;

	return 0;
}



//------------------------------------------------------------------------------
//
void DMA1_Stream0_IRQHandler(void)
{
	// TC Flag
	if (DMA1->LISR & DMA_LISR_TCIF0)
	{
		USART1->CR3 &= ~USART_CR3_DMAT;
		DMA1_Stream0->CR &= ~DMA_SxCR_EN;

		DMA1->LIFCR |= DMA_LIFCR_CTCIF0;
	}

	// Error Flag
	if (DMA1->LISR & DMA_LISR_TEIF0)
	{
		USART1->CR3 &= ~USART_CR3_DMAT;
		DMA1_Stream0->CR &= ~DMA_SxCR_EN;

		DMA1->LIFCR |= DMA_LIFCR_CTEIF0;
	}
}
