#include "ControlBus.h"

#include <stm32h7xx.h>

#include <stddef.h>

static ControlBus_TxComplete_Callback_t gpTxCompleteCallback = NULL;
static ControlBus_RxComplete_Callback_t gpRxCompleteCallback = NULL;
static ControlBus_BusFailure_Callback_t gpBusFailureCallback = NULL;

//------------------------------------------------------------------------------
//
void ControlBus_Init(void)
{
	/**
	 * I2C Instance: I2C4
	 * SCL: PD12 AF4
	 * SDA: PD13 AF4
	 * 
	 * TX DMA: BDMA Channel0
	 * RX DMA: BDMA Channel1
	 * 
	 * Bus: APB4
	 * Clock: APB4 Clock (100 MHz)
	 */

	RCC->AHB4ENR |= RCC_AHB4ENR_GPIODEN;

	GPIOD->MODER &= ~(GPIO_MODER_MODER12_Msk | GPIO_MODER_MODER13_Msk);
	GPIOD->MODER |= (0b10 << GPIO_MODER_MODER12_Pos) | (0b10 << GPIO_MODER_MODER13_Pos);
	GPIOD->OTYPER |= GPIO_OTYPER_OT12 | GPIO_OTYPER_OT13;
	GPIOD->PUPDR &= ~(GPIO_PUPDR_PUPD12_Msk | GPIO_PUPDR_PUPD13_Msk);
	GPIOD->OSPEEDR |= (0b11 << GPIO_OSPEEDR_OSPEED12_Pos) | (0b11 << GPIO_OSPEEDR_OSPEED13_Pos);
	GPIOD->AFR[1] &= ~(GPIO_AFRH_AFSEL12_Msk | GPIO_AFRH_AFSEL13_Msk);
	GPIOD->AFR[1] |= (4 << GPIO_AFRH_AFSEL12_Pos) | (4 << GPIO_AFRH_AFSEL13_Pos);



	RCC->AHB4ENR |= RCC_AHB4ENR_BDMAEN;

	BDMA_Channel0->CCR = 0;
	DMAMUX2_Channel0->CCR = 14 << DMAMUX_CxCR_DMAREQ_ID_Pos;

	BDMA_Channel1->CCR = 0;
	DMAMUX2_Channel1->CCR = 13 << DMAMUX_CxCR_DMAREQ_ID_Pos;

	NVIC_ClearPendingIRQ(BDMA_Channel0_IRQn);
	NVIC_EnableIRQ(BDMA_Channel0_IRQn);
	
	NVIC_ClearPendingIRQ(BDMA_Channel1_IRQn);
	NVIC_EnableIRQ(BDMA_Channel1_IRQn);



	RCC->D3CCIPR &= ~RCC_D3CCIPR_I2C4SEL_Msk;

	RCC->APB4ENR |= RCC_APB4ENR_I2C4EN;

	I2C4->CR1 = 0;
	I2C4->CR2 = 0;
	I2C4->OAR1 = 0;
	I2C4->OAR2 = 0;

	I2C4->TIMINGR = 0x10C0ECFF;

	// I2C4->CR1 |= I2C_CR1_NOSTRETCH;
	// I2C4->CR1 |= I2C_CR1_ERRIE;

	NVIC_ClearPendingIRQ(I2C4_ER_IRQn);
	NVIC_EnableIRQ(I2C4_ER_IRQn);

	I2C4->CR1 |= I2C_CR1_PE;
}

//------------------------------------------------------------------------------
//
int ControlBus_SendBlocking(uint8_t address, const uint8_t* pDataBuffer, size_t dataSize, bool genStop)
{
	if ((pDataBuffer == NULL) || (dataSize == 0) || (dataSize > 255))
	{
		return -1;
	}

	I2C4->CR2 = 0;

	I2C4->CR2 |= ((uint32_t)dataSize & 0xFF) << I2C_CR2_NBYTES_Pos;
	I2C4->CR2 |= ((uint32_t)address) << I2C_CR2_SADD_Pos;

	I2C4->TXDR = *pDataBuffer;
	pDataBuffer++;

	I2C4->CR2 |= I2C_CR2_START;

	while (1)
	{
		if (I2C4->ISR & I2C_ISR_TXIS)
		{
			I2C4->TXDR = *pDataBuffer;
			pDataBuffer++;
		}

		if (I2C4->ISR & I2C_ISR_TC)
		{
			break;
		}

		if (I2C4->ISR & I2C_ISR_NACKF)
		{
			I2C4->ICR |= I2C_ICR_NACKCF;
			return -1;
		}
	}

	if (genStop)
	{
		I2C4->CR2 |= I2C_CR2_STOP;

		while ((I2C4->ISR & I2C_ISR_STOPF) == 0);
		I2C4->ICR |= I2C_ICR_STOPCF;
	}

	return 0;
}

//------------------------------------------------------------------------------
//
int ControlBus_RecvBlocking(uint8_t address, uint8_t* pDataBuffer, size_t dataSize, bool genStop)
{
	if ((pDataBuffer == NULL) || (dataSize == 0) || (dataSize > 255))
	{
		return -1;
	}

	I2C4->CR2 = 0;

	I2C4->CR2 |= ((uint32_t)dataSize & 0xFF) << I2C_CR2_NBYTES_Pos;
	I2C4->CR2 |= ((uint32_t)address) << I2C_CR2_SADD_Pos;
	I2C4->CR2 |= I2C_CR2_RD_WRN;

	I2C4->CR2 |= I2C_CR2_START;

	while (1)
	{
		if (I2C4->ISR & I2C_ISR_RXNE)
		{
			*pDataBuffer = I2C4->RXDR;
			pDataBuffer++;
		}

		if (I2C4->ISR & I2C_ISR_TC)
		{
			break;
		}
	}

	if (genStop)
	{
		I2C4->CR2 |= I2C_CR2_STOP;

		while ((I2C4->ISR & I2C_ISR_STOPF) == 0);
		I2C4->ICR |= I2C_ICR_STOPCF;
	}

	return 0;
}

//------------------------------------------------------------------------------
//
int ControlBus_Send(uint8_t address, const uint8_t* pDataBuffer, size_t dataSize, bool genStop, ControlBus_TxComplete_Callback_t pCpltCb, ControlBus_BusFailure_Callback_t pErrCb)
{
	if ((pDataBuffer == NULL) || (dataSize == 0) || (dataSize > 255))
	{
		return -1;
	}

	// if (I2C4->ISR & I2C_ISR_BUSY)
	// {
	// 	return -1;
	// }

	I2C4->CR1 |= I2C_CR1_TXDMAEN;
	I2C4->CR2 = 0;

	I2C4->CR2 |= ((uint32_t)dataSize & 0xFF) << I2C_CR2_NBYTES_Pos;
	I2C4->CR2 |= ((uint32_t)address) << I2C_CR2_SADD_Pos;

	if (genStop)
	{
		I2C4->CR2 |= I2C_CR2_AUTOEND;
	}

	BDMA_Channel0->CCR = 0;

	BDMA_Channel0->CCR |= BDMA_CCR_MINC;
	BDMA_Channel0->CCR |= BDMA_CCR_DIR;
	BDMA_Channel0->CCR |= BDMA_CCR_TCIE;
	
	BDMA_Channel0->CNDTR = dataSize;
	BDMA_Channel0->CPAR = (uint32_t)&I2C4->TXDR;
	BDMA_Channel0->CM0AR = (uint32_t)pDataBuffer;

	BDMA->IFCR |= BDMA_IFCR_CGIF0 | BDMA_IFCR_CTCIF0 | BDMA_IFCR_CHTIF0 | BDMA_IFCR_CTEIF0;

	gpTxCompleteCallback = pCpltCb;
	gpBusFailureCallback = pErrCb;

	BDMA_Channel0->CCR |= BDMA_CCR_EN;
	I2C4->CR2 |= I2C_CR2_START;

	return 0;
}

//------------------------------------------------------------------------------
//
int ControlBus_Recv(uint8_t address, uint8_t* pDataBuffer, size_t dataSize, bool genStop, ControlBus_RxComplete_Callback_t pCpltCb, ControlBus_BusFailure_Callback_t pErrCb)
{
	if ((pDataBuffer == NULL) || (dataSize == 0) || (dataSize > 255))
	{
		return -1;
	}

	// if (I2C4->ISR & I2C_ISR_BUSY)
	// {
	// 	return -1;
	// }

	I2C4->CR1 |= I2C_CR1_RXDMAEN;
	I2C4->CR2 = 0;

	I2C4->CR2 |= ((uint32_t)dataSize & 0xFF) << I2C_CR2_NBYTES_Pos;
	I2C4->CR2 |= ((uint32_t)address) << I2C_CR2_SADD_Pos;
	I2C4->CR2 |= I2C_CR2_RD_WRN;

	if (genStop)
	{
		I2C4->CR2 |= I2C_CR2_AUTOEND;
	}

	BDMA_Channel1->CCR = 0;

	BDMA_Channel1->CCR |= BDMA_CCR_MINC;
	BDMA_Channel1->CCR |= BDMA_CCR_TCIE;
	
	BDMA_Channel1->CNDTR = dataSize;
	BDMA_Channel1->CPAR = (uint32_t)&I2C4->RXDR;
	BDMA_Channel1->CM0AR = (uint32_t)pDataBuffer;

	BDMA->IFCR |= BDMA_IFCR_CGIF1 | BDMA_IFCR_CTCIF1 | BDMA_IFCR_CHTIF1 | BDMA_IFCR_CTEIF1;

	gpRxCompleteCallback = pCpltCb;
	gpBusFailureCallback = pErrCb;

	BDMA_Channel1->CCR |= BDMA_CCR_EN;
	I2C4->CR2 |= I2C_CR2_START;

	return 0;
}



//------------------------------------------------------------------------------
//
void BDMA_Channel0_IRQHandler(void)
{
	if (BDMA->ISR & BDMA_ISR_TCIF0)
	{
		BDMA_Channel0->CCR = 0;
		I2C4->CR1 &= ~I2C_CR1_TXDMAEN;
		BDMA->IFCR |= BDMA_IFCR_CTCIF0;

		if (gpTxCompleteCallback)
			gpTxCompleteCallback();
	}

	BDMA->IFCR |= BDMA_IFCR_CGIF0;
}

//------------------------------------------------------------------------------
//
void BDMA_Channel1_IRQHandler(void)
{
	if (BDMA->ISR & BDMA_ISR_TCIF1)
	{
		BDMA_Channel1->CCR = 0;
		I2C4->CR1 &= ~I2C_CR1_RXDMAEN;
		BDMA->IFCR |= BDMA_IFCR_CTCIF1;

		if (gpRxCompleteCallback)
			gpRxCompleteCallback();
	}

	BDMA->IFCR |= BDMA_IFCR_CGIF1;
}

//------------------------------------------------------------------------------
//
void I2C4_ER_IRQHandler(void)
{
	if (gpBusFailureCallback)
		gpBusFailureCallback();
}
