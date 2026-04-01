#ifndef __MOTOR_H
#define __MOTOR_H

#include "sys.h"

// ????
void Motor_PWM_Init(u16 arr, u16 psc);
void Set_Fan_Speed(u16 speed);
void Set_Pump_Speed(u16 speed);

#endif