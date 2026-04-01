#include "sys.h"

void LED_GPIO_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;// 定义GPIO初始化结构体
    RCC_APB2PeriphClockCmd(LED_GPIO_CLK, ENABLE);// 使能LED所在GPIO端口的时钟（APB2总线）
	
    GPIO_InitStruct.GPIO_Pin = LED_GPIO_PIN1;// 选择要控制的GPIO引脚（宏定义）
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;// 设置为推挽输出模式
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;// 设置GPIO输出速度为50MHz

    GPIO_Init(LED_GPIO_PORT, &GPIO_InitStruct);// 初始化GPIO（参数：GPIO端口地址，初始化结构体指针）
}
