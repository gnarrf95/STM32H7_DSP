#include "Clock.h"

#include <stm32h7xx.h>

//------------------------------------------------------------------------------
//
void SystemClock_Init(void)
{
	/**
	 * HSE = 25 MHz
	 * sysclk = PLL1P = PLL1R = PLL1Q = ((HSE / 2) * 64) / 2 = 400 MHz
	 * 
	 * CPU1_CLK = sysclk = 400 MHz
	 * CPU1_SYSTICK_CLK = 50 MHz
	 * AXI_CLK = 200 MHz
	 * AHB3_CLK = 200 MHz
	 * APB3_CLK = 100 MHz
	 * 
	 * CPU2_CLK = D1_CLK = 200 MHz
	 * CPU2_SYSTICK_CLK = 25 MHz
	 * AHB1_CLK = 200 MHz
	 * APB1_CLK = 100 MHz
	 * APB1_TIMER_CLK = 200 MHz
	 * AHB2_CLK = 200 MHz
	 * APB2_CLK = 100 MHz
	 * APB2_TIMER_CLK = 200 MHz
	 * 
	 * AHB4_CLK = 200 MHz
	 * APB4_CLK = 100 MHz
	 */



	// Set SMPS only mode, set voltage to VOS1
	PWR->CR3 = PWR_CR3_SMPSEN;
	PWR->D3CR |= PWR_D3CR_VOS_Msk;
	while ((PWR->D3CR & PWR_D3CR_VOSRDY) == 0);



	// Enable HSE
	RCC->CR &= ~RCC_CR_HSEBYP;
	RCC->CR |= RCC_CR_HSEON;

	// Wait till HSE is ready
	while ((RCC->CR & RCC_CR_HSERDY) == 0);

	// Disable PLLs
	RCC->CR &= ~RCC_CR_PLL1ON;
	RCC->CR &= ~RCC_CR_PLL2ON;
	RCC->CR &= ~RCC_CR_PLL3ON;

	// Wait till PLLs are disabled
	while ((RCC->CR & RCC_CR_PLL1RDY) != 0);
	while ((RCC->CR & RCC_CR_PLL2RDY) != 0);
	while ((RCC->CR & RCC_CR_PLL3RDY) != 0);

	// Select HSE as PLL source
	RCC->PLLCKSELR &= ~RCC_PLLCKSELR_PLLSRC_Msk;
	RCC->PLLCKSELR |= 0b10 << RCC_PLLCKSELR_PLLSRC_Pos;



	// Set DIVM1 = 2
	RCC->PLLCKSELR &= ~RCC_PLLCKSELR_DIVM1_Msk;
	RCC->PLLCKSELR |= 2 << RCC_PLLCKSELR_DIVM1_Pos;

	// Configure PLL1 (N=64, R=2, P=2, Q=2)
	RCC->PLL1DIVR = 0;
	RCC->PLL1DIVR |= (64-1) << RCC_PLL1DIVR_N1_Pos;
	RCC->PLL1DIVR |= (2-1) << RCC_PLL1DIVR_R1_Pos;
	RCC->PLL1DIVR |= (2-1) << RCC_PLL1DIVR_P1_Pos;
	RCC->PLL1DIVR |= (2-1) << RCC_PLL1DIVR_Q1_Pos;

	// Set FRACN1 to 0
	RCC->PLLCFGR &= ~RCC_PLLCFGR_PLL1FRACEN;
	RCC->PLL1FRACR = 0;

	// Configure PLL1 VCO
	RCC->PLLCFGR &= ~RCC_PLLCFGR_PLL1VCOSEL;
	RCC->PLLCFGR |= RCC_PLLCFGR_PLL1RGE_Msk;

	// Enable PLL1 outputs R, P and Q
	RCC->PLLCFGR |= RCC_PLLCFGR_DIVP1EN;
	RCC->PLLCFGR |= RCC_PLLCFGR_DIVP1EN;
	RCC->PLLCFGR |= RCC_PLLCFGR_DIVQ1EN;

	// Write FRACN1 to modulator
	RCC->PLLCFGR |= RCC_PLLCFGR_PLL1FRACEN;



	// Set DIVM2 = 25
	RCC->PLLCKSELR &= ~RCC_PLLCKSELR_DIVM2_Msk;
	RCC->PLLCKSELR |= 25 << RCC_PLLCKSELR_DIVM2_Pos;

	// Configure PLL2 (N=429, R=2, P=38, Q=2)
	RCC->PLL2DIVR = 0;
	RCC->PLL2DIVR |= (429-1) << RCC_PLL2DIVR_N2_Pos;
	RCC->PLL2DIVR |= (2-1) << RCC_PLL2DIVR_R2_Pos;
	RCC->PLL2DIVR |= (38-1) << RCC_PLL2DIVR_P2_Pos;
	RCC->PLL2DIVR |= (2-1) << RCC_PLL2DIVR_Q2_Pos;

	// Set FRACN2 to 0
	RCC->PLLCFGR &= ~RCC_PLLCFGR_PLL2FRACEN;
	RCC->PLL2FRACR = 0;

	// Configure PLL2 VCO
	RCC->PLLCFGR &= ~RCC_PLLCFGR_PLL2VCOSEL;
	RCC->PLLCFGR |= RCC_PLLCFGR_PLL2RGE_Msk;

	// Enable PLL2 outputs R, P and Q
	RCC->PLLCFGR |= RCC_PLLCFGR_DIVP2EN;
	RCC->PLLCFGR |= RCC_PLLCFGR_DIVP2EN;
	RCC->PLLCFGR |= RCC_PLLCFGR_DIVQ2EN;

	// Write FRACN2 to modulator
	RCC->PLLCFGR |= RCC_PLLCFGR_PLL2FRACEN;



	// Enable PLL1 and PLL2
	RCC->CR |= RCC_CR_PLL1ON;
	RCC->CR |= RCC_CR_PLL2ON;

	// Wait till PLL is ready
	while ((RCC->CR & RCC_CR_PLL1RDY) == 0);
	while ((RCC->CR & RCC_CR_PLL2RDY) == 0);



	// Increase CPU frequency
	FLASH->ACR = FLASH_ACR_LATENCY_2WS | (2 << FLASH_ACR_WRHIGHFREQ_Pos);
	while ((FLASH->ACR & FLASH_ACR_LATENCY_Msk) != FLASH_ACR_LATENCY_2WS);

	// Set D1 PPRE to 2 (APB3)
	RCC->D1CFGR &= ~RCC_D1CFGR_D1PPRE_Msk;
	RCC->D1CFGR |= 0b100 << RCC_D1CFGR_D1PPRE_Pos;
	
	// Set D2 PPRE1 to 2 (APB1)
	RCC->D2CFGR &= ~RCC_D2CFGR_D2PPRE1_Msk;
	RCC->D2CFGR |= 0b100 << RCC_D2CFGR_D2PPRE1_Pos;
	
	// Set D2 PPRE2 to 2 (APB2)
	RCC->D2CFGR &= ~RCC_D2CFGR_D2PPRE2_Msk;
	RCC->D2CFGR |= 0b100 << RCC_D2CFGR_D2PPRE2_Pos;

	// Set D3 PPRE to 2 (APB4)
	RCC->D3CFGR &= ~RCC_D3CFGR_D3PPRE_Msk;
	RCC->D3CFGR |= 0b100 << RCC_D3CFGR_D3PPRE_Pos;

	// Set D1 HPRE to 2 (AHB)
	RCC->D1CFGR &= ~RCC_D1CFGR_HPRE_Msk;
	RCC->D1CFGR |= 0b1000 << RCC_D1CFGR_HPRE_Pos;

	// Set D1 CPRE to 1 (SysClk)
	RCC->D1CFGR &= ~RCC_D1CFGR_D1CPRE_Msk;

	

	// Check that PLL is ready
	while ((RCC->CR & RCC_CR_PLL1RDY) == 0);

	// Set PLL as SysClk source
	RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW_Msk) | RCC_CFGR_SW_PLL1;

	// Wait for SysClk to be set to PLL
	while ((RCC->CFGR & RCC_CFGR_SWS_Msk) != RCC_CFGR_SWS_PLL1);
}

//------------------------------------------------------------------------------
//
void SystemClock_SelectAudioClock(SystemClock_AudioSamplingRate_e samplingRate)
{

}
