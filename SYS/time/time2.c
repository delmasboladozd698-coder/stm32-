#include "sys.h"

/********************
①使能定时器时钟。
RCC_APB1PeriphClockCmd();
② 初始化定时器，配置ARR,PSC。
TIM_TimeBaseInit();
③定时器中断配置，同时配置NVIC。
void TIM_ITConfig();
NVIC_Init();
④使能定时器。
TIM_Cmd();
⑤编写中断服务函数。
TIMx_IRQHandler();
*****************/

void TIM2_Init(u32 arr,u32 psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);
	
	TIM_TimeBaseStructure.TIM_Period = arr;
	TIM_TimeBaseStructure.TIM_Prescaler = psc;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM2,&TIM_TimeBaseStructure);
	
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);		
	TIM_Cmd(TIM2, ENABLE);
}


void TIM2_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)  
	{		
        KeyScan();//按键扫描
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update); 					
	}

}

