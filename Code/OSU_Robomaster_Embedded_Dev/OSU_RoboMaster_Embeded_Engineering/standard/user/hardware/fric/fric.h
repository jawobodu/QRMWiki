#ifndef FRIC_H
#define FRIC_H
#include "main.h"

#define Fric_UP 1400//Ħ�����Ҽ�ת��
#define Fric_DOWN 1355//Ħ�������ת��             1355 28m/s   1375 30//1350 27.5//1300 24.5
#define Fric_OFF 1000

extern void fric_PWM_configuration(void);
extern void fric_off(void);
extern void fric1_on(uint16_t cmd);
extern void fric2_on(uint16_t cmd);
#endif
