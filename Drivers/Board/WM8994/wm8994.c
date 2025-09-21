#include "wm8994.h"
#include "wm8994_bus.h"

#define WRITE_REG(addr, value) if (wm8994_WriteRegister(addr, value, false) < 0) { return -1; }
#define WRITE_BITS(addr, mask) if (wm8994_SetRegisterBits(addr, mask, false) < 0) { return -1; }

//------------------------------------------------------------------------------
//
int wm8994_Reset(void)
{
	// Check device ID
	uint16_t deviceId;
	if (wm8994_ReadRegister(0x0000, &deviceId) < 0)
	{
		return -1;
	}
	if (deviceId != 0x8994)
	{
		return -1;
	}

	// Reset
	if (wm8994_WriteRegister(0x0000, 0x0000, false) < 0)
	{
		return -1;
	}
	wm8994_Delay(100);

	// WM8994 Errata Work-Arounds
	WRITE_REG(0x0102, 0x0003);
	WRITE_REG(0x0817, 0x0000);
	WRITE_REG(0x0102, 0x0000);

	// Enable VMID soft start (fast), Start-up Bias Current Enabled
	WRITE_REG(0x0039, 0x006C);

	// Enable bias generator, Enable VMID
	WRITE_BITS(0x0001, 0x0003);
	wm8994_Delay(50);

	return 0;
}

//------------------------------------------------------------------------------
//
int wm8994_ConfigureInterface(void)
{
	// Fs=44.1kHz, Clock Ratio=256
	WRITE_REG(0x0210, 0x0073);

	// Word Length=16bit, Format=I2S
	WRITE_REG(0x0300, 0x4010);

	// Set to Slave mode
	WRITE_REG(0x0302, 0x0000);

	// Enable AIF1 Processing Clock, Enable Core Clock
	WRITE_REG(0x0208, 0x000A);

	// Enable AIF1 Clock, Source=MCLK1
	WRITE_REG(0x0200, 0x0001);

	return 0;
}

//------------------------------------------------------------------------------
//
int wm8994_RouteInput(void)
{
	// IN1LN_TO_IN1L & IN1LP_TO_VMID, IN1RN_TO_IN1R & IN1RP_TO_VMID
	WRITE_REG(0x0028, 0x0011);

	// IN1L_TO_MIXINL, IN1L_MIXINL_VOL
	WRITE_REG(0x0029, 0x0020);

	// IN1R_TO_MIXINR, IN1R_MIXINR_VOL
	WRITE_REG(0x002A, 0x0020);

	// Enable ADC & AIF1ADC1
	WRITE_BITS(0x0004, 0x0303);

	// Enable AIF1 DRC1 Signal Detect & DRC in AIF1ADC1
	WRITE_REG(0x0440, 0x00DB);

	// Enable IN1L & IN1R PGA, Enable MIXINL & MIXINR, Enable Thermal Sensor & Shutdown
	WRITE_BITS(0x0002, 0x6350);

	// ADCL to AIF1L Signal
	WRITE_REG(0x0606, 0x0002);

	// ADCR to AIF1R Signal
	WRITE_REG(0x0607, 0x0002);

	// Configure GPIO1 as AIF DRC1 detect Interrupt
	WRITE_REG(0x0700, 0x000D);

	return 0;
}

//------------------------------------------------------------------------------
//
int wm8994_RouteOutput(bool enableDac, bool enablePassthrough)
{
	// Mute HPOUT1L
	WRITE_REG(0x001C, 0x0100);

	// Mute HPOUT1R
	WRITE_REG(0x001D, 0x0100);

	// Reset MIXOUT
	WRITE_REG(0x002D, 0x0000);
	WRITE_REG(0x002E, 0x0000);
	WRITE_REG(0x002F, 0x0000);
	WRITE_REG(0x0030, 0x0000);

	if (enableDac)
	{
		// Enable DAC1 & AIF1DAC1
		WRITE_BITS(0x0005, 0x0303);

		// AIF1DAC1L -> DAC1L
		WRITE_REG(0x0601, 0x0001);

		// AIF1DAC1R -> DAC1R
		WRITE_REG(0x0602, 0x0001);

		// Disable AIF1DAC2L
		WRITE_REG(0x0604, 0x0000);

		// Disable AIF1DAC2R
		WRITE_REG(0x0605, 0x0000);

		// Unmute DAC1L
		WRITE_REG(0x0610, 0x01C0);

		// Unmute DAC1R
		WRITE_REG(0x0611, 0x01C0);

		// Soft unmute AIF1DAC1
		WRITE_REG(0x0420, 0x0000);

		// DAC1L_TO_MIXOUTL & DAC1R_TO_MIXOUTR Unmute
		WRITE_BITS(0x002D, 0x0001);
		WRITE_BITS(0x002E, 0x0001);
	}

	if (enablePassthrough)
	{
		// IN1L_TO_MIXOUTL & IN1R_TO_MIXOUTR Unmute
		WRITE_BITS(0x002D, 0x0040);
		WRITE_BITS(0x002E, 0x0040);
	}

	// Enable MIXOUT
	WRITE_BITS(0x0003, 0x0030);

	return 0;
}

//------------------------------------------------------------------------------
//
int wm8994_SetInputGain(float gain, bool mute)
{
	// [-16.5dB, +30dB] -> [0, 63]

	if (gain < -16.5F)
	{
		gain = -16.5F;
	}
	else if (gain > 30.0F)
	{
		gain = 30.0F;
	}

	uint16_t gainValue = (uint16_t)((gain + 16.5F) / 1.5F);
	uint16_t registerValue = gainValue | (1 << 8);

	if (mute)
	{
		registerValue |= (1 << 7);
	}

	WRITE_REG(0x0018, registerValue);
	WRITE_REG(0x001A, registerValue);

	return 0;
}

//------------------------------------------------------------------------------
//
int wm8994_SetOutputGain(float gain, bool mute)
{
	// [-57dB, +6dB] -> [0, 63]

	if (gain < -57.0F)
	{
		gain = -57.0F;
	}
	else if (gain > 6.0F)
	{
		gain = 6.0F;
	}

	uint16_t gainValue = (uint16_t)(gain + 57.0F);
	uint16_t registerValue = gainValue | (1 << 8);

	if (mute == false)
	{
		registerValue |= (1 << 6);
	}

	WRITE_REG(0x001C, registerValue);
	WRITE_REG(0x001D, registerValue);

	return 0;
}

//------------------------------------------------------------------------------
//
int wm8994_EnableInput(bool enable)
{
	// IN1L Mute
	WRITE_REG(0x0018, 0x0080);

	// IN1R Mute
	WRITE_REG(0x001A, 0x0080);

	// Enable AIF1ADC1 HPF, HiFi mode
	WRITE_REG(0x0410, 0x1800);

	// Set ADC1L to 0dB
	WRITE_REG(0x0400, 0x00C0 | 0x0100);

	// Set ADC1R to 0dB
	WRITE_REG(0x0401, 0x00C0 | 0x0100);
}

//------------------------------------------------------------------------------
//
int wm8994_EnableOutput(bool enable)
{


	// Enable Class W Envelope Tracking for AIF1DAC1
	WRITE_REG(0x0051, 0x0005);

	// Enable HPOUT1 input stages
	WRITE_BITS(0x0001, 0x0300);

	// Enable HPOUT1 intermediate stages
	WRITE_REG(0x0060, 0x0022);

	// Enable Charge Pump
	WRITE_REG(0x004C, 0x9F25);
	wm8994_Delay(15);



	// Enable DC Servo and trigger Start-up mode
	WRITE_BITS(0x0054, 0x0033);
	wm8994_Delay(250);

	// Enable HPOUT1 intermediate & output stages
	WRITE_BITS(0x0060, 0x00EE);



	return 0;
}
