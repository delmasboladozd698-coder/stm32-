#include "sys.h"

/**
  * @brief  继电器(RELAY) GPIO 配置函数
  * @note   初始化继电器控制引脚，配置为推挽输出模式，并默认关闭所有继电器
  * @param  无
  * @retval 无
  */
void RELAY_GPIO_Config(void)
{
    // 定义GPIO初始化结构体
    GPIO_InitTypeDef GPIO_InitStruct;

    // 使能继电器所在GPIO端口的时钟（APB2总线）
    RCC_APB2PeriphClockCmd(RELAY_GPIO_CLK, ENABLE); 

    // 配置GPIO初始化参数（同时初始化多个引脚）
    GPIO_InitStruct.GPIO_Pin = RELAY_GPIO_PIN1 | RELAY_GPIO_PIN2;  // 选择所有继电器控制引脚
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;  // 推挽输出模式（可直接驱动继电器线圈）
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz; // 高速模式（确保快速响应）
	
    // 初始化GPIO
    GPIO_Init(RELAY_GPIO_PORT, &GPIO_InitStruct);
    
    // 初始化后立即关闭所有继电器（安全设计）
    RELAY1_OFF;
}
