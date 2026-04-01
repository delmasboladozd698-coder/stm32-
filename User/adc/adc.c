#include "sys.h"

/*-------------------------------------------
 * 函数名：Adc_Init
 * 功能：初始化ADC1模块及相关GPIO
 * 特点：
 *   - 配置指定GPIO为模拟输入模式
 *   - ADC时钟12MHz（72MHz/6分频）
 *   - 单通道单次转换模式
 *-------------------------------------------*/
void Adc_Init(void)
{
    ADC_InitTypeDef ADC_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;

    /* 时钟配置 */
    RCC_APB2PeriphClockCmd(ADC_RCC, ENABLE); // 使能GPIO和ADC1时钟

    /* ADC时钟分频配置 */
    RCC_ADCCLKConfig(RCC_PCLK2_Div6); // 72MHz主频6分频得12MHz ADC时钟

    /* GPIO初始化（模拟输入模式） */
    GPIO_InitStructure.GPIO_Pin = ADC_GPIO_PIN;    // 通过宏定义选择ADC通道对应引脚
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;  // 模拟输入模式（关闭施密特触发器）
    GPIO_Init(ADC_GPIO_PORT, &GPIO_InitStructure); // 应用GPIO配置

    /* ADC模块复位 */
    ADC_DeInit(ADC1); // 将ADC1寄存器恢复默认值

    /* ADC工作参数配置 */
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;                  // 独立工作模式
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;                       // 禁用扫描模式（单通道）
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;                 // 单次转换模式
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; // 软件触发转换
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;              // 数据右对齐（12位有效）
    ADC_InitStructure.ADC_NbrOfChannel = 1;                             // 1个转换通道
    ADC_Init(ADC1, &ADC_InitStructure);                                 // 应用ADC配置

    /* 校准流程 */
    ADC_Cmd(ADC1, ENABLE);      // 使能ADC1模块
    ADC_ResetCalibration(ADC1); // 启动复位校准
    while (ADC_GetResetCalibrationStatus(ADC1))
        ; // 等待复位校准完成（硬件自动清除标志）

    ADC_StartCalibration(ADC1); // 开始AD校准
    while (ADC_GetCalibrationStatus(ADC1))
        ; // 等待校准完成（约1ms）
}

/*-------------------------------------------
 * 函数名：Get_Adc
 * 功能：获取指定通道的单次ADC采样值
 * 参数：ch - ADC通道号（0-17对应ADC_IN0-ADC_IN17）
 * 返回：12位ADC转换结果（0-4095）
 * 注：采样时间239.5周期（12MHz下约20us）
 *-------------------------------------------*/
u16 Get_Adc(u8 ch)
{
    /* 配置转换通道参数 */
    ADC_RegularChannelConfig(ADC1, ch, 1, ADC_SampleTime_239Cycles5); // 通道/序列/采样时间

    /* 启动转换并等待完成 */
    ADC_SoftwareStartConvCmd(ADC1, ENABLE); // 软件触发转换
    while (!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC))
        ; // 等待转换结束（EOC标志置位）

    return ADC_GetConversionValue(ADC1); // 返回转换结果（自动清除EOC标志）
}

/*-------------------------------------------
 * 函数名：Get_Adc_Average
 * 功能：获取指定通道的多次采样平均值
 * 参数：
 *   ch    - ADC通道号
 *   times - 采样次数（1-65535）
 * 返回：12位ADC平均值（0-4095）
 * 特点：
 *   - 每次采样间隔5ms降低噪声
 *   - 自动处理采样值溢出（u32累加）
 *-------------------------------------------*/
u16 Get_Adc_Average(u8 ch, u8 times)
{
    u32 temp_val = 0; // 32位累加器防止溢出
    u8 t;

    for (t = 0; t < times; t++)
    {
        temp_val += Get_Adc(ch); // 累加单次采样值
        delay_ms(5);             // 间隔5ms（允许信号稳定）
    }
    return temp_val / times; // 计算算术平均值
}
