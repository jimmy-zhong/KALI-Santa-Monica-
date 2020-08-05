#ifndef __DEVICE_H__
#define __DEVICE_H__

#include "stm32f107xc.h"
#include "stdio.h"


#define WIZ_SCS			    GPIO_PIN_4	// out
#define WIZ_SCLK				GPIO_PIN_5	// out
#define WIZ_MISO				GPIO_PIN_6	// in
#define WIZ_MOSI				GPIO_PIN_7	// out

//#define WIZ_RESET		    GPIO_PIN_0	// out (PB0)
#define WIZ_INT			    GPIO_PIN_3	// in (PA3)

#define KSM_HOST_FOUND					1

#define CTRL_MAXVOL				+60
#define CTRL_MINVOL				-120


typedef  void (*pFunction)(void);
void GPIO_Configuration(void);
void set_network(void);
void Reset_W5500(void);
void W5500_LOOP(void);
void reboot(void);

#define GPIO_SetBits(GPIO_X, PIN_NO)						HAL_GPIO_WritePin(GPIO_X, PIN_NO, GPIO_PIN_SET)
#define GPIO_ResetBits(GPIO_X, PIN_NO)					HAL_GPIO_WritePin(GPIO_X, PIN_NO, GPIO_PIN_RESET)

#endif

