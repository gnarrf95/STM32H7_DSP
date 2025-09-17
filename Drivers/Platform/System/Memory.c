#include "Memory.h"

#include <stm32h7xx.h>

static void SystemMemory_InitFmcClock(void);
static void SystemMemory_InitFmcGpio(void);
static void SystemMemory_ConfigureSdRam(void);

static void SystemMemory_SdCommand(uint32_t commandMode, uint32_t commandTarget, uint32_t autoRefreshNumber, uint32_t modeRegisterDefinition);

//------------------------------------------------------------------------------
//
void SystemMemory_InitSdRam(void)
{
	SystemMemory_InitFmcGpio();
	SystemMemory_InitFmcClock();

	// Enable IRQ
	NVIC_SetPriority(FMC_IRQn, 0);
	NVIC_EnableIRQ(FMC_IRQn);

	SystemMemory_ConfigureSdRam();
}

//------------------------------------------------------------------------------
//
void SystemMemory_InitMpu(void)
{
	// Not used yet.
}



//------------------------------------------------------------------------------
//
static void SystemMemory_InitFmcGpio(void)
{
	/**
	 * Address:
	 * A[0:5] -> PF[0:5] AF12
	 * A[6:9] -> PF[12:15] AF12
	 * A[10:12] -> PG[0:2] AF12
	 * A[14:15] -> PG[4:5] AF12
	 * 
	 * Data:
	 * D[0:1] -> PD[14:15] AF12
	 * D[2:3] -> PD[0:1] AF12
	 * D[4:12] -> PE[7:15] AF12
	 * D[13:15] -> PD[8:10] AF12
	 * D[16:23] -> PH[8:15] AF12
	 * D[24:27] -> PI[0:3] AF12
	 * D[28:29] -> PI[6:7] AF12
	 * D[30:31] -> PI[9:10] AF12
	 * 
	 * Other:
	 * NBL0 -> PE0 AF12
	 * NBL1 -> PE1 AF12
	 * NBL2 -> PI4 AF12
	 * NBL3 -> PI5 AF12
	 * SDCLK -> PG8 AF12
	 * SDCKE1 -> PH7 AF12
	 * SDNE1 -> PH6 AF12
	 * SDNRAS -> PF11 AF12
	 * SDNCAS -> PG15 AF12
	 * SDNWE -> PH5 AF12
	 */

	RCC->AHB4ENR |= RCC_AHB4ENR_GPIODEN | RCC_AHB4ENR_GPIOEEN | RCC_AHB4ENR_GPIOFEN | RCC_AHB4ENR_GPIOGEN | RCC_AHB4ENR_GPIOHEN | RCC_AHB4ENR_GPIOIEN;

	// GPIOD
	GPIOD->MODER = (GPIOD->MODER & 0x0FC0FFF0) | 0xA02A000A;
	GPIOD->OSPEEDR |= 0xF03F000F;
	GPIOD->AFR[0] = (GPIOD->AFR[0] & 0xFFFFFF00) | 0x0000000CC;
	GPIOD->AFR[1] = (GPIOD->AFR[1] & 0x00FFF000) | 0xCC000CCC;

	// GPIOE
	GPIOE->MODER = (GPIOE->MODER & 0x00003FF0) | 0xAAAA800A;
	GPIOE->OSPEEDR |= 0xFFFFC00F;
	GPIOE->AFR[0] = (GPIOE->AFR[0] & 0x0FFFFF00) | 0xC00000CC;
	GPIOE->AFR[1] = (GPIOE->AFR[1] & 0x00000000) | 0xCCCCCCCC;

	// GPIOF
	GPIOF->MODER = (GPIOF->MODER & 0x003FF000) | 0xAA800AAA;
	GPIOF->OSPEEDR |= 0xFFC00FFF;
	GPIOF->AFR[0] = (GPIOF->AFR[0] & 0xFF000000) | 0x00CCCCCC;
	GPIOF->AFR[1] = (GPIOF->AFR[1] & 0x00000FFF) | 0xCCCCC000;

	// GPIOG
	GPIOG->MODER = (GPIOG->MODER & 0x3F3CF0C0) | 0x80820A2A;
	GPIOG->OSPEEDR |= 0xC0C30F3F;
	GPIOG->AFR[0] = (GPIOG->AFR[0] & 0xFF00F000) | 0x00CC0CCC;
	GPIOG->AFR[1] = (GPIOG->AFR[1] & 0x0FFF0FF0) | 0xC000C00C;

	// GPIOH
	GPIOH->MODER = (GPIOH->MODER & 0x000003FF) | 0xAAAAA800;
	GPIOH->OSPEEDR |= 0xFFFFFC00;
	GPIOH->AFR[0] = (GPIOH->AFR[0] & 0x000FFFFF) | 0xCCC00000;
	GPIOH->AFR[1] = (GPIOH->AFR[1] & 0x00000000) | 0xCCCCCCCC;

	// GPIOI
	GPIOI->MODER = (GPIOI->MODER & 0xFFC30000) | 0x0028AAAA;
	GPIOI->OSPEEDR |= 0x003CFFFF;
	GPIOI->AFR[0] = (GPIOI->AFR[0] & 0x00000000) | 0xCCCCCCCC;
	GPIOI->AFR[1] = (GPIOI->AFR[1] & 0xFFFFF00F) | 0x00000CC0;
}

//------------------------------------------------------------------------------
//
static void SystemMemory_InitFmcClock(void)
{
	// Select HCLK as FMC Kernel Clock
	RCC->D1CCIPR &= ~RCC_D1CCIPR_FMCSEL_Msk;

	// Enable FMC Clock
	RCC->AHB3ENR |= RCC_AHB3ENR_FMCEN;

	for (uint32_t ctr = 0; ctr < 500000; ctr++)
	{
		__NOP();
	}
}

//------------------------------------------------------------------------------
//
static void SystemMemory_ConfigureSdRam(void)
{
	// Configure SDRAM Interface
	FMC_Bank5_6_R->SDCR[0] = 0;
	FMC_Bank5_6_R->SDCR[1] = 0;

	FMC_Bank5_6_R->SDCR[0] |= 0b00 << FMC_SDCRx_RPIPE_Pos;
	FMC_Bank5_6_R->SDCR[0] |= 0b1 << FMC_SDCRx_RBURST_Pos;
	FMC_Bank5_6_R->SDCR[0] |= 0b10 << FMC_SDCRx_SDCLK_Pos;

	FMC_Bank5_6_R->SDCR[1] |= 0b0 << FMC_SDCRx_WP_Pos;
	FMC_Bank5_6_R->SDCR[1] |= 0b11 << FMC_SDCRx_CAS_Pos;
	FMC_Bank5_6_R->SDCR[1] |= 0b1 << FMC_SDCRx_NB_Pos;
	FMC_Bank5_6_R->SDCR[1] |= 0b10 << FMC_SDCRx_MWID_Pos;
	FMC_Bank5_6_R->SDCR[1] |= 0b01 << FMC_SDCRx_NR_Pos;
	FMC_Bank5_6_R->SDCR[1] |= 0b01 << FMC_SDCRx_NC_Pos;

	// Configure SDRAM Timings
	FMC_Bank5_6_R->SDTR[0] = 0;
	FMC_Bank5_6_R->SDTR[1] = 0;

	FMC_Bank5_6_R->SDTR[0] |= (2 - 1) << FMC_SDTRx_TRP_Pos;
	FMC_Bank5_6_R->SDTR[0] |= (7 - 1) << FMC_SDTRx_TRC_Pos;

	FMC_Bank5_6_R->SDTR[1] |= (2 - 1) << FMC_SDTRx_TRCD_Pos;
	FMC_Bank5_6_R->SDTR[1] |= (3 - 1) << FMC_SDTRx_TWR_Pos;
	FMC_Bank5_6_R->SDTR[1] |= (4 - 1) << FMC_SDTRx_TRAS_Pos;
	FMC_Bank5_6_R->SDTR[1] |= (7 - 1) << FMC_SDTRx_TXSR_Pos;
	FMC_Bank5_6_R->SDTR[1] |= (2 - 1) << FMC_SDTRx_TMRD_Pos;

	// Enable FMC
	FMC_Bank1_R->BTCR[0] |= FMC_BCR1_FMCEN;

	// SDRAM Command: Enable Clock
	SystemMemory_SdCommand(1, FMC_SDCMR_CTB2, 1, 0);

	// Delay 10ms
	for (uint32_t ctr = 0; ctr < 500000; ctr++)
	{
		__NOP();
	}

	// SDRAM Command: Precharge
	SystemMemory_SdCommand(2, FMC_SDCMR_CTB2, 1, 0);

	// SDRAM Command: Set Refresh Mode
	SystemMemory_SdCommand(3, FMC_SDCMR_CTB2, 8, 0);

	// SDRAM Command: Program External Memory Mode Register
	SystemMemory_SdCommand(4, FMC_SDCMR_CTB2, 1, 0x230);

	// SDRAM Command: Set Refresh Rate
	FMC_Bank5_6_R->SDRTR = (FMC_Bank5_6_R->SDRTR & ~FMC_SDRTR_COUNT_Msk) | (0x0603 << FMC_SDRTR_COUNT_Pos);

	// Disable SDRAM Write Protection
	FMC_Bank5_6_R->SDCR[1] &= ~FMC_SDCRx_WP;
}

//------------------------------------------------------------------------------
//
static void SystemMemory_SdCommand(uint32_t commandMode, uint32_t commandTarget, uint32_t autoRefreshNumber, uint32_t modeRegisterDefinition)
{
	FMC_Bank5_6_R->SDCMR = (FMC_Bank5_6_R->SDCMR & ~(FMC_SDCMR_MODE_Msk | FMC_SDCMR_CTB2_Msk | FMC_SDCMR_CTB1_Msk | FMC_SDCMR_NRFS_Msk | FMC_SDCMR_MRD_Msk))
		| commandMode | commandTarget
		| ((autoRefreshNumber - 1) << FMC_SDCMR_NRFS_Pos)
		| (modeRegisterDefinition << FMC_SDCMR_MRD_Pos);
}

//------------------------------------------------------------------------------
//
void FMC_IRQHandler(void)
{
	if (FMC_Bank5_6_R->SDSR & FMC_SDSR_RE)
	{


		FMC_Bank5_6_R->SDRTR |= FMC_SDRTR_CRE;
	}
}
