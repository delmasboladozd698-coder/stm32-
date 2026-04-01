#ifndef __LED_H__
#define __LED_H__

#include "sys.h"

#define LED_GPIO_PIN1 GPIO_Pin_13

#define LED_GPIO_PORT GPIOC
#define LED_GPIO_CLK RCC_APB2Periph_GPIOC

#define LED1_OFF GPIO_ResetBits(LED_GPIO_PORT, LED_GPIO_PIN1) // ¹Ø
#define LED1_ON GPIO_SetBits(LED_GPIO_PORT, LED_GPIO_PIN1)    // ¿ª

void LED_GPIO_Config(void);

#endif
