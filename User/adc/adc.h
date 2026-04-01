#ifndef _ADC_H
#define _ADC_H
#include "sys.h" // 系统级头文件（包含寄存器定义和基础驱动）

/******************** ADC模块硬件配置宏 ********************/
/*
 * 注意：配置需与硬件电路一致
 比如：
 * PA0 -> ADC1通道0
 具体使用那几个引脚看宏定义  ADC_GPIO_PIN
 */
#define ADC_RCC RCC_APB2Periph_GPIOA | RCC_APB2Periph_ADC1 // GPIOA时钟使能（ADC引脚所在组）使能ADC1时钟
#define ADC_GPIO_PORT GPIOA                                // ADC通道所属GPIO端口
#define ADC_GPIO_PIN GPIO_Pin_0 | GPIO_Pin_5

/******************** 函数声明 ********************/
/*
 * 参数说明：
 *   ch    - ADC通道号（0-17对应ADC_IN0-ADC_IN17，具体可用通道需查芯片手册）
 *   times - 采样次数（1-65535，建议值10-100）
 * 返回值：12位ADC值（0-4095对应0-3.3V）
 */
extern u16 Get_Adc_Average(u8 ch, u8 times); // 获取通道ch的times次采样平均值（带去噪）
extern u16 Get_Adc(u8 ch);                   // 获取通道ch的单次原始采样值
void Adc_Init(void);                         // ADC模块初始化（配置时钟/GPIO/校准流程）

#endif
