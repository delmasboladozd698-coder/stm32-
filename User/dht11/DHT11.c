#include "sys.h"

DHT11_Data_TypeDef dht11Data;  // DHT11传感器数据结构体（温度/湿度原始值）

/*----------------------------------------------------------
 * 函数名：DHT11_Init
 * 功能：初始化DHT11传感器的GPIO配置
 * 设计说明：
 *   1. 配置单总线开漏输出模式，支持双向通信
 *   2. 初始状态置高电平，符合DHT11通信协议要求
 
 
 *----------------------------------------------------------*/
void DHT11_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct; // GPIO配置结构体
    
    /* 使能GPIO时钟（APB2总线） */
	RCC_APB2PeriphClockCmd(DHT11_Dout_GPIO_CLK, ENABLE);
    
    /* 配置GPIO参数 */
	GPIO_InitStruct.GPIO_Pin = DHT11_Dout_GPIO_PIN;       // 选择数据引脚
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_OD;         // 开漏输出模式（兼容总线协议）
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;        // 高速模式（确保时序精度）
	
    /* 应用GPIO配置 */
	GPIO_Init(DHT11_Dout_GPIO_PORT, &GPIO_InitStruct);	
    
    /* 总线初始状态置高（准备启动信号） */
    DHT11_Dout_1;  // 宏定义等效：GPIO_SetBits(DHT11_Dout_GPIO_PORT, DHT11_Dout_GPIO_PIN)
}

unsigned int DHT11_us[50]; // 存储DHT11传感器时序数据的数组（单位：微秒）

/*-----------------------------------------------------------
 * 函数名：time_to_data
 * 功能：将时序脉冲宽度数组转换为8位数据字节
 * 参数：
 *   in_time - 时序脉冲宽度数组（单位：微秒）
 * 返回值：
 *   8位数据字节（每位对应时序长度是否>50us）
 * 实现原理：
 *   循环处理前8个时序数据，每个时序>50us视为'1'，否则视为'0'
 *   通过左移位操作构建最终字节
 * 注：本函数专为DHT11传感器协议解码设计
 *-----------------------------------------------------------*/
unsigned char time_to_data(unsigned int *in_time)
{
    unsigned char i;                 // 循环索引（处理前8个数据位）
    unsigned char change_res = 0;    // 结果字节（初始值00000000）
    
    for (i = 0; i < 8; i++)         // 遍历8个数据位（DHT11数据格式）
    {
        change_res <<= 1;           // 左移腾出最低位（首次循环无影响）
        
        /* 时序阈值判断：>50us为逻辑1（DHT11标准协议阈值） */
        if (in_time[i] > 50)        // 判断当前位数据电平持续时间
        {
            change_res |= 1;       // 最低位置1（追加数据位）
        }
    }
    
    return change_res; // 返回构建的8位数据（如：01011011）
}

unsigned int dht11_delay_ms = 18; // DHT11启动信号持续时间（单位：毫秒）

/*
 * 数据格式说明：
 * 一次完整的数据传输为40bit，高位先出
 * 8bit 湿度整数 + 8bit 湿度小数 + 8bit 温度整数 + 8bit 温度小数 + 8bit 校验和
 */
uint8_t DHT11_Read_TempAndHumidity(DHT11_Data_TypeDef *DHT11_Data)
{
    unsigned char i;                     // 循环计数器
    static unsigned int last_up_tick_us = 0, present_dowm_tick_us = 0, waite_in_ms; // 时间戳变量
    
    DHT11_Data->isvalid = 0;            // 初始化数据有效性标志
    
    /* 启动信号阶段 */
    /* 主机拉低总线启动通信 */
    DHT11_Dout_0;                       // 设置总线为低电平
    HAL_Delay(dht11_delay_ms);          // 保持低电平18ms（DHT11规范要求）
    DHT11_Dout_1;                       // 释放总线（拉高电平）

    /* 等待传感器响应 */
    waite_in_ms = HAL_GetTick();        // 记录当前系统时间
    while (DHT11_Dout_IN() && HAL_GetTick() <= waite_in_ms+1); // 等待传感器拉低总线（超时1ms）

    /* 数据采集阶段（采集41个边沿变化）*/
    for (i = 0; i < 41; i++)
    {
        /* 等待总线上升沿 */
        waite_in_ms = HAL_GetTick();
        while (!DHT11_Dout_IN() && HAL_GetTick() <= waite_in_ms+1); // 等待低电平结束
        
        /* 记录上升沿时间戳 */
        last_up_tick_us = GetSysTime_us(); // 获取当前微秒级时间
        
        /* 等待总线下降沿 */
        waite_in_ms = HAL_GetTick();
        while (DHT11_Dout_IN() && HAL_GetTick() <= waite_in_ms+1); // 等待高电平结束
        
        /* 计算脉冲持续时间 */
        present_dowm_tick_us = GetSysTime_us();
        if (last_up_tick_us > present_dowm_tick_us) // 处理计时器溢出
        {
            DHT11_us[i] =  present_dowm_tick_us + 1000 - last_up_tick_us; // 1ms周期补偿
        }
        else
        {
            DHT11_us[i] =  present_dowm_tick_us - last_up_tick_us; // 正常时间差计算
        }
    }
    DHT11_Dout_1; // 恢复总线高电平

    /* 数据解析阶段 */
    DHT11_Data->humi_int = time_to_data(&DHT11_us[1]);   // 解析湿度整数位（第1-8个脉冲）
    DHT11_Data->humi_deci = time_to_data(&DHT11_us[9]);  // 解析湿度小数位（第9-16个脉冲）
    DHT11_Data->temp_int = time_to_data(&DHT11_us[17]);  // 解析温度整数位（第17-24个脉冲）
    DHT11_Data->temp_deci = time_to_data(&DHT11_us[25]); // 解析温度小数位（第25-32个脉冲）
    DHT11_Data->check_sum = time_to_data(&DHT11_us[33]); // 解析校验和（第33-40个脉冲）

    /* 数据校验 */
    if (DHT11_Data->check_sum == DHT11_Data->humi_int + DHT11_Data->humi_deci 
                               + DHT11_Data->temp_int + DHT11_Data->temp_deci)
    {
        DHT11_Data->isvalid = 1; // 校验成功标记有效数据
    }

    return 0; // 固定返回0（实际状态通过结构体字段体现）
}

