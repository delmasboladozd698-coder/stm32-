#include "sys.h"
#include "motor.h" 
#include "pid.h"   

//???????
uint8_t mode_selection = 0;
//???????
uint8_t displayFlag = 0;
//???????
uint8_t thresholdFlag = 0;
//???????
uint8_t  manualFlag = 0;

static PID_TypeDef Fan_PID;
static PID_TypeDef Pump_PID;
static uint8_t pid_hardware_init_flag = 0; 
static uint16_t fan_pwm_output = 0;
static uint16_t pump_pwm_output = 0;

/*****************????????? ??????????????*/

void Mode_selection(void)
{
    //???????? ???????
    Action();

    //??1 ??????
    if ((isKey1) && (displayFlag == 0)) {
        isKey1 = isKey2 = isKey3 = isKey4 = 0;
        mode_selection++;
        oled_Clear();

        if (mode_selection == 2) { //???????,???????
            System.Switch1 = System.Switch2 = System.Switch3 = 0; // ???? Switch4
            fan_pwm_output = 0;
            pump_pwm_output = 0;
        }
    }

    if ((isKey1) && (displayFlag != 0)) {
        displayFlag = 0;
        isKey1 = 0;
        oled_Clear();
    }

    //?????????
    if (mode_selection >= 3) {
        mode_selection = 0;
    }

    //??????????????  ????????????????????
    switch (mode_selection) {
    case 0:
        Display();//???????
        Automatic();//????
        break;

    case 1:
        Threshold_Settings();//????
        Automatic();//????
        break;

    case 2:
        Manual();//????
        break;
    }
}

/****
????????
****/

void Automatic(void)
{
    static uint16_t pid_tick = 0;
    
    if (++pid_tick >= 50) { 
        pid_tick = 0;

        Fan_PID.target_val = Threshold.TempThreshold;
        Pump_PID.target_val = Threshold.SoilThreshold;

        // --- 1. ??????(??) ---
        float fan_error = SensorData.TempVal - Fan_PID.target_val;
        if (fan_error <= 0) fan_error = 0; 

        Fan_PID.target_val = fan_error;    
        fan_pwm_output = (uint16_t)PID_Compute(&Fan_PID, 0);

        // --- 2. ??????(??) ---
        float pump_error = Pump_PID.target_val - SensorData.SoilVal;
        if (pump_error <= 0) pump_error = 0; 

        Pump_PID.target_val = pump_error;  
        pump_pwm_output = (uint16_t)PID_Compute(&Pump_PID, 0);
    }

    System.Switch1 = (fan_pwm_output > 0) ? 1 : 0; 
    System.Switch2 = (pump_pwm_output > 0) ? 1 : 0; 

    // ??????
    if (SensorData.LightVal <= Threshold.LightThreshold) {
        System.Switch3 = 1;//??????1
    } else {
        System.Switch3 = 0;//??????0
    }
    
    // ?????????(Switch4)??????????
}

/****
?????????????????
****/

void Action(void)
{
    if (pid_hardware_init_flag == 0) {
        Motor_PWM_Init(1000, 71); 
        
        PID_Init(&Fan_PID, 50.0, 5.0, 1.0, 1000.0, 0.0);
        PID_Init(&Pump_PID, 40.0, 2.0, 0.5, 1000.0, 0.0);
        
        pid_hardware_init_flag = 1;
    }

    if (mode_selection == 2) {
        Set_Fan_Speed(System.Switch1 ? 1000 : 0);
        Set_Pump_Speed(System.Switch2 ? 1000 : 0);
    } else {
        Set_Fan_Speed(fan_pwm_output);
        Set_Pump_Speed(pump_pwm_output);
    }

    if (System.Switch3 == 1) {
        LED1_ON;//????
    } else {
        LED1_OFF;//????
    }
    
    // ?????????????????
}


void Display(void)
{
    if (isKey3) {
        isKey3 = 0;
        oled_Clear();
        displayFlag = 2;
    }

    if (isKey4) {
        isKey4 = 0;
        oled_Clear();
        displayFlag = 3;
    }

    if (displayFlag == 0) {
        oled_ShowCHinese(16 * 2, 2 * 0, 0);
        oled_ShowCHinese(16 * 3, 2 * 0, 1);
        oled_ShowCHinese(16 * 4, 2 * 0, 2);
        oled_ShowCHinese(16 * 5, 2 * 0, 3);
        //??
        oled_ShowCHinese(16 * 0, 2 * 1, 25);
        oled_ShowCHinese(16 * 1, 2 * 1, 26);
        oled_ShowString(16 * 3, 2 * 1, ":", 16);
        oled_ShowNum(16 * 4, 2 * 1, SensorData.TempVal, 2, 16);//??????
        oled_ShowCHinese(16 * 5, 2 * 1, 27);
        //??
        oled_ShowCHinese(16 * 0, 2 * 2, 20);
        oled_ShowCHinese(16 * 1, 2 * 2, 21);
        oled_ShowString(16 * 3, 2 * 2, ":", 16);
        oled_ShowNum(16 * 4, 2 * 2, SensorData.SoilVal, 2, 16);//????????
        oled_ShowString(16 * 5, 2 * 2, "%RH", 16);
        //??
        oled_ShowCHinese(16 * 0, 2 * 3, 17);
        oled_ShowCHinese(16 * 1, 2 * 3, 24);
        oled_ShowString(16 * 3, 2 * 3, ":", 16);
        oled_ShowNum(16 * 4, 2 * 3, SensorData.LightVal, 4, 16);//????????
        oled_ShowString(16 * 6, 2 * 3, "Lux", 16);
    }

    //????mqtt??????
    if (displayFlag == 2) {
        MqttCon_Display();
    }

    //???????????
    if (displayFlag == 3) {
        Topic_Display();
    }
}

/****
????
****/
void Threshold_Settings(void)
{
    //??2????????
    if (isKey2) {
        isKey2 = 0;
        thresholdFlag++;

        //???? ??-1
        if (thresholdFlag >= 3) {
            thresholdFlag = 0;
        }
    }

    //????????
    Asterisk(thresholdFlag);

    //??3?????
    if (isKey3) {
        isKey3 = 0;//????3???

        switch (thresholdFlag) {
        case 0:
            Threshold.TempThreshold ++;//????+1
            break;

        case 1:
            Threshold.SoilThreshold ++;//??????+1
            break;

        case 2:
            Threshold.LightThreshold += 10; //??????+10
            break;
        }
    }

    //??4?????
    if (isKey4) {
        isKey4 = 0;//????4???

        switch (thresholdFlag) {
        case 0:
            Threshold.TempThreshold --;//????-1
            break;

        case 1:
            Threshold.SoilThreshold --;//??????-1
            break;

        case 2:
            Threshold.LightThreshold -= 10; //??????-10
            break;
        }
    }

    //???????4 ?????
    if (thresholdFlag < 3) {
        //????
        oled_ShowCHinese(16 * 2, 2 * 0, 8);
        oled_ShowCHinese(16 * 3, 2 * 0, 9);
        oled_ShowCHinese(16 * 4, 2 * 0, 10);
        oled_ShowCHinese(16 * 5, 2 * 0, 11);
        //????
        oled_ShowCHinese(16 * 0, 2 * 1, 25);
        oled_ShowCHinese(16 * 1, 2 * 1, 26);
        oled_ShowCHinese(16 * 2, 2 * 1, 8);
        oled_ShowCHinese(16 * 3, 2 * 1, 9);
        oled_ShowString(16 * 4, 2 * 1, ":", 16);
        oled_ShowNum(16 * 5, 2 * 1, Threshold.TempThreshold, 2, 16);//????????
        //????
        oled_ShowCHinese(16 * 0, 2 * 2, 20);
        oled_ShowCHinese(16 * 1, 2 * 2, 21);
        oled_ShowCHinese(16 * 2, 2 * 2, 8);
        oled_ShowCHinese(16 * 3, 2 * 2, 9);
        oled_ShowString(16 * 4, 2 * 2, ":", 16);
        oled_ShowNum(16 * 5, 2 * 2, Threshold.SoilThreshold, 2, 16);//??????????
        //????
        oled_ShowCHinese(16 * 0, 2 * 3, 17);
        oled_ShowCHinese(16 * 1, 2 * 3, 24);
        oled_ShowCHinese(16 * 2, 2 * 3, 8);
        oled_ShowCHinese(16 * 3, 2 * 3, 9);
        oled_ShowString(16 * 4, 2 * 3, ":", 16);
        oled_ShowNum(16 * 5, 2 * 3, Threshold.LightThreshold, 4, 16);//??????????
    }

    /*????*/

    //??
    if (Threshold.TempThreshold > 99) {
        Threshold.TempThreshold = 99;
    }

    if (Threshold.TempThreshold < 1) {
        Threshold.TempThreshold = 0;
    }

    //????
    if (Threshold.SoilThreshold > 99) {
        Threshold.SoilThreshold = 99;
    }

    if (Threshold.SoilThreshold < 1) {
        Threshold.SoilThreshold = 0;
    }

    //????
    if (Threshold.LightThreshold > 999) {
        Threshold.LightThreshold = 990;
    }

    if (Threshold.LightThreshold < 1) {
        Threshold.LightThreshold = 0;
    }
}

/****
????
****/

void Manual(void)
{
    //????2??
    if (isKey2) {
        isKey2 = 0;
        manualFlag++;

        // ?????:??????4???3,????????
        if (manualFlag >= 3) {
            manualFlag = 0;
        }

        if (manualFlag == 0) {
            oled_Clear();
        }
    }

    //????????
    Asterisk(manualFlag);

    //????3????
    if (isKey3) {
        isKey3 = 0;

        switch (manualFlag) {
        case 0:
            System.Switch1 = !System.Switch1;//???-??????
            break;

        case 1:
            System.Switch2 = !System.Switch2;//???-??????
            break;

        case 2:
            System.Switch3 = !System.Switch3;//??????
            break;
        // ???? case 3 ???????
        }
    }

    // ??????? if/else ????,????????????
    
    //??????
    oled_ShowCHinese(16 * 2, 2 * 0, 4);
    oled_ShowCHinese(16 * 3, 2 * 0, 5);
    oled_ShowCHinese(16 * 4, 2 * 0, 6);
    oled_ShowCHinese(16 * 5, 2 * 0, 7);
    //??
    oled_ShowCHinese(16 * 0, 2 * 1, 28);
    oled_ShowCHinese(16 * 1, 2 * 1, 29);
    oled_ShowString(16 * 2, 2 * 1, ":", 16);

    if (System.Switch1 == 1) {
        oled_ShowCHinese(16 * 3, 2 * 1, 12);
        oled_ShowCHinese(16 * 4, 2 * 1, 13);//??
    } else {
        oled_ShowCHinese(16 * 3, 2 * 1, 14);
        oled_ShowCHinese(16 * 4, 2 * 1, 15);//??
    }

    //??
    oled_ShowCHinese(16 * 0, 2 * 2, 22);
    oled_ShowCHinese(16 * 1, 2 * 2, 23);
    oled_ShowString(16 * 2, 2 * 2, ":", 16);

    if (System.Switch2 == 1) {
        oled_ShowCHinese(16 * 3, 2 * 2, 12);
        oled_ShowCHinese(16 * 4, 2 * 2, 13);//??
    } else {
        oled_ShowCHinese(16 * 3, 2 * 2, 14);
        oled_ShowCHinese(16 * 4, 2 * 2, 15);//??
    }

    //??
    oled_ShowCHinese(16 * 0, 2 * 3, 16);
    oled_ShowCHinese(16 * 1, 2 * 3, 17);
    oled_ShowString(16 * 2, 2 * 3, ":", 16);

    if (System.Switch3 == 1) {
        oled_ShowCHinese(16 * 3, 2 * 3, 12);
        oled_ShowCHinese(16 * 4, 2 * 3, 13);//??
    } else {
        oled_ShowCHinese(16 * 3, 2 * 3, 14);
        oled_ShowCHinese(16 * 4, 2 * 3, 15);//??
    }
}

//?????? (????)
void Asterisk(uint8_t A)
{
    if (A == 3 || A == 7) {
        oled_ShowString(16 * 7, 2 * 0, "*", 16);
    } else {
        oled_ShowString(16 * 7, 2 * 0, " ", 16);
    }

    if (A == 0 || A == 4) {
        oled_ShowString(16 * 7, 2 * 1, "*", 16);
    } else {
        oled_ShowString(16 * 7, 2 * 1, " ", 16);
    }

    if (A == 1 || A == 5) {
        oled_ShowString(16 * 7, 2 * 2, "*", 16);
    } else {
        oled_ShowString(16 * 7, 2 * 2, " ", 16);
    }

    if (A == 2 || A == 6) {
        oled_ShowString(16 * 7, 2 * 3, "*", 16);
    } else {
        oled_ShowString(16 * 7, 2 * 3, " ", 16);
    }
}