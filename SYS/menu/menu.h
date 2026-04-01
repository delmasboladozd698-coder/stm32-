#ifndef __MENU_H
#define __MENU_H
#include "sys.h"
extern uint8_t mode_selection;
extern uint8_t displayFlag;
extern uint8_t thresholdFlag;
extern uint8_t  manualFlag;

void Display(void);
void Automatic(void);
void Threshold_Settings(void);
void Mode_selection(void);
void Manual(void);
void Action(void);
void Asterisk(uint8_t A);
#endif

