#include "sys.h"

//调用光照转换算法需要先初始化ADC

/* 光强转换表（电阻值-照度对应表）
 * 格式：[光照强度值Lux, 对应电阻值R]
 * 注意：电阻值需按降序排列，用于分段线性插值 */
float Lux_Table[4][2] = {
    {10, 8.0},   // 当电阻=8.0Ω时，对应10 Lux照度
    {100, 1.0},  // 当电阻=1.0Ω时，对应100 Lux照度 
    {999, 0.1},  // 当电阻=0.1Ω时，对应999 Lux照度
    {1000, 0}    // 当电阻=0Ω时，对应1000 Lux照度（理论极限值）
};

/* ADC值转照度函数
 * 参数：adc_vel - ADC原始采样值（0-4095范围）
 * 返回：计算得到的照度值（单位：Lux） */
float R_to_Lux(unsigned short adc_vel) 
{
    float R;        // 计算得到的电阻值
    float R_kp;     // 电阻在区间内的比例系数（0.0-1.0）
    float Lux = 0;  // 最终光照强度值
    
    /* 通过ADC值计算电阻（基于分压电路公式）
     * 公式推导：R = (Vadc * Rref)/(Vref - Vadc)
     * 其中：Vadc=adc_vel/4095*Vref，简化得当前表达式 */
    R = (float)(adc_vel*10)/(4095-adc_vel);
    
    /* 分段查找电阻所在区间（倒序查找）*/
    for (int i = 0; i < sizeof(Lux_Table)/sizeof(Lux_Table[0]) - 1; i++) 
    {
        /* 检查电阻是否在当前区间（Lux_Table[i+1][1] < R <= Lux_Table[i][1]）*/
        if (R > Lux_Table[i+1][1] && R <= Lux_Table[i][1]) 
        {
            /* 线性插值计算
             * 比例系数 = (上限电阻 - 当前电阻)/(上限电阻 - 下限电阻)
             * 照度值 = 下限照度 + 比例系数*(上限照度 - 下限照度) */
            R_kp = (Lux_Table[i][1] - R)/(Lux_Table[i][1] - Lux_Table[i+1][1]);
            Lux = (Lux_Table[i+1][0]-Lux_Table[i][0])*R_kp + Lux_Table[i][0];
            return Lux; // 返回插值计算结果
        }
    }

    /* 未找到匹配区间时返回0（当R > 8Ω或R < 0时触发）*/
    return Lux; 
}
