#include "led.h"

#include <stm32h7xx.h>

//------------------------------------------------------------------------------
//
void led_Init(void)
{
    RCC->AHB4ENR |= RCC_AHB4ENR_GPIOIEN;

    GPIOI->MODER &= ~(GPIO_MODER_MODER12_Msk | GPIO_MODER_MODER13_Msk | GPIO_MODER_MODER14_Msk | GPIO_MODER_MODER15_Msk);
    GPIOI->MODER |= (0b01 << GPIO_MODER_MODER12_Pos) | (0b01 << GPIO_MODER_MODER13_Pos) | (0b01 << GPIO_MODER_MODER14_Pos) | (0b01 << GPIO_MODER_MODER15_Pos);
}

//------------------------------------------------------------------------------
//
void led_Write(led_Index_t index, bool set)
{
    uint32_t pos = 0;

    switch (index)
    {
        case LED_INDEX_GREEN:
            pos = 12;
            break;
        case LED_INDEX_ORANGE:
            pos = 13;
            break;
        case LED_INDEX_RED:
            pos = 14;
            break;
        case LED_INDEX_BLUE:
            pos = 15;
            break;
        default:
            return;
    }

    if (set)
    {
        GPIOI->ODR |= (1 << pos);
    }
    else
    {
        GPIOI->ODR &= ~(1 << pos);
    }
}

//------------------------------------------------------------------------------
//
void led_Toggle(led_Index_t index)
{
    uint32_t pos = 0;

    switch (index)
    {
        case LED_INDEX_GREEN:
            pos = 12;
            break;
        case LED_INDEX_ORANGE:
            pos = 13;
            break;
        case LED_INDEX_RED:
            pos = 14;
            break;
        case LED_INDEX_BLUE:
            pos = 15;
            break;
        default:
            return;
    }

    GPIOI->ODR ^= (1 << pos);
}
