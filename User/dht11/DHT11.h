#ifndef __DHT11_H
#define __DHT11_H

#include "sys.h"  // 系统级头文件，包含MCU外设驱动和类型定义

/************************** DHT11 数据类型定义 ********************************/
/* 
 * 传感器数据帧结构体定义（DHT11传输40位数据）
 * 注意：DHT11小数部分实际固定为0，保留字段兼容其他传感器
 */
typedef struct {
    uint8_t  humi_int;   // 湿度整数部分（单位：%RH，范围20-90）
    uint8_t  humi_deci;  // 湿度小数部分（DHT11固定为0，保留兼容DHT22）
    uint8_t  temp_int;   // 温度整数部分（单位：℃，范围0-50）
    uint8_t  temp_deci;  // 温度小数部分（DHT11固定为0，保留兼容DHT22）
    uint8_t  check_sum;  // 校验和（humi_int + humi_deci + temp_int + temp_deci）
    uint8_t  isvalid;    // 数据有效性标志（1=校验成功，0=校验失败）
} DHT11_Data_TypeDef;

/************************** DHT11 硬件连接定义 ********************************/
/* 时钟配置 */
#define DHT11_Dout_SCK_APBxClock_FUN     RCC_APB2PeriphClockCmd  // APB2总线时钟使能函数
#define DHT11_Dout_GPIO_CLK              RCC_APB2Periph_GPIOA    // GPIOA时钟使能

/* GPIO引脚配置 */
#define DHT11_Dout_GPIO_PORT             GPIOA       // 数据线连接的GPIO端口
#define DHT11_Dout_GPIO_PIN              GPIO_Pin_4  // 数据线连接的GPIO引脚（需接4.7K上拉电阻）

/************************** DHT11 操作宏定义 ********************************/
/* 单总线控制宏（开漏模式需外接上拉电阻）*/
#define DHT11_Dout_0   DHT11_Dout_GPIO_PORT->BRR  = DHT11_Dout_GPIO_PIN  // 拉低总线（输出0）
#define DHT11_Dout_1   DHT11_Dout_GPIO_PORT->BSRR = DHT11_Dout_GPIO_PIN  // 释放总线（输出1，实际电平由上拉电阻决定）

/* 总线状态读取宏 */
#define DHT11_Dout_IN() (DHT11_Dout_GPIO_PORT->IDR & DHT11_Dout_GPIO_PIN) // 读取总线电平（0=低，非0=高）

/************************** 全局变量声明 ********************************/
extern DHT11_Data_TypeDef dht11Data;  // 全局传感器数据结构体实例

/************************** 函数声明 ********************************/
/* 传感器初始化函数（配置GPIO为开漏输出）*/
void DHT11_Init(void); 

/* 
 * 温湿度读取主函数
 * 参数：DHT11_Data - 数据存储结构体指针
 * 返回：0（固定值，实际状态通过结构体isvalid字段判断）
 */
uint8_t DHT11_Read_TempAndHumidity(DHT11_Data_TypeDef *DHT11_Data);

#endif /* __DHT11_H */
