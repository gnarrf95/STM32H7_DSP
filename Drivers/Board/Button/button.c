#include "button.h"

#include <stm32h7xx.h>

//------------------------------------------------------------------------------
//
void button_Init(void)
{
    RCC->AHB4ENR |= RCC_AHB4ENR_GPIOCEN | RCC_AHB4ENR_GPIOKEN;

    GPIOC->MODER &= ~GPIO_MODER_MODER13_Msk;
    GPIOK->MODER &= ~(GPIO_MODER_MODER2_Msk | GPIO_MODER_MODER3_Msk | GPIO_MODER_MODER4_Msk | GPIO_MODER_MODER5_Msk | GPIO_MODER_MODER6_Msk);

    GPIOK->PUPDR &= ~(GPIO_PUPDR_PUPD2_Msk | GPIO_PUPDR_PUPD3_Msk | GPIO_PUPDR_PUPD4_Msk | GPIO_PUPDR_PUPD5_Msk | GPIO_PUPDR_PUPD6_Msk);
    GPIOK->PUPDR |= (0b01 << GPIO_PUPDR_PUPD2_Pos) | (0b01 << GPIO_PUPDR_PUPD3_Pos) | (0b01 << GPIO_PUPDR_PUPD4_Pos) | (0b01 << GPIO_PUPDR_PUPD5_Pos) | (0b01 << GPIO_PUPDR_PUPD6_Pos);
}

//------------------------------------------------------------------------------
//
bool button_Read(button_Index_t index)
{
    if (index == BUTTON_INDEX_USER)
    {
        return ((GPIOC->IDR & (1 << 13)) != 0);
    }

    uint32_t pos = 0;
    switch (index)
    {
        case BUTTON_INDEX_JOYSELECT:
            pos = 2;
            break;
        case BUTTON_INDEX_JOYDOWN:
            pos = 3;
            break;
        case BUTTON_INDEX_JOYLEFT:
            pos = 4;
            break;
        case BUTTON_INDEX_JOYRIGHT:
            pos = 5;
            break;
        case BUTTON_INDEX_JOYUP:
            pos = 6;
            break;
        default:
            return false;
    }

    return ((GPIOK->IDR & (1 << pos)) == 0);
}
