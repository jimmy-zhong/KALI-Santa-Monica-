/**
  ******************************************************************************
  * @file    USB_Host/MSC_Standalone/Inc/main.h 
  * @author  MCD Application Team
  * @brief   Header for main.c module
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2016 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                      www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __APP_H__
#define __APP_H__

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"
#include "stdio.h"
//#include "stm3210c_eval.h"
#include "usbh_core.h"
//#include "lcd_log.h"
#include "usbh_msc.h" 
#include "ff.h"


#define STOP_COUNTING													-1
/* Exported types ------------------------------------------------------------*/

// In input mode (MODE[1:0] = 00b)
#define INP_ANALOG_CONF				(0x00 << 2)					// Analog mode - 00b
#define INP_FLOAT_CONF				(0x01 << 2)					// Floating input (reset state) - 01b
#define INP_PUPD_CONF					(0x02 << 2)					// Input with pull-up / pull-down - 10b
																							// 11: Reserved
// In output mode (MODE[1:0] > 00):
#define OUT_PP_MODE						(0x00 << 2)					// General purpose output push-pull - 00b
#define OUT_OD_MODE						(0x01 << 2)					// General purpose output Open-drain - 01b
#define AF_PP_MODE						(0x02 << 2)					// Alternate function output Push-pull - 10b
#define AF_OD_MODE						(0x03 << 2)					// Alternate function output Open-drain - 11b

// MODE[1:0]
#define INPUT_MODE						0x00							// Input mode (reset state) - 00b
#define OUT10M_MODE						0x01							// Output mode, max speed 10 MHz. - 01b
#define OUT02M_MODE						0x02							// Output mode, max speed 2 MHz. - 10b
#define OUT50M_MODE						0x03							// Output mode, max speed 50 MHz. - 11b


#define INPUT_ANALOG							(uint32_t)INP_ANALOG_CONF + INPUT_MODE			// 0x00	(0000b)
#define INPUT_FLOAT								(uint32_t)INP_FLOAT_CONF + INPUT_MODE				// 0x04	(0100b)
#define INPUT_PUPD								(uint32_t)INP_PUPD_CONF + INPUT_MODE				// 0x08	(1000b)

#define OUTPP_MODE_10M						(uint32_t)OUT_PP_MODE + OUT10M_MODE					// 0x01	(0001b)
#define OUT_OD_MODE_10M						(uint32_t)OUT_OD_MODE + OUT10M_MODE					// 0x05	(0101b)
#define AF_PP_MODE_10M						(uint32_t)AF_PP_MODE + OUT10M_MODE					// 0x09	(1001b)
#define AF_OD_MODE_10M						(uint32_t)AF_OD_MODE + OUT10M_MODE					// 0x0D	(1101b)

#define OUTPP_MODE_02M						(uint32_t)OUT_PP_MODE + OUT02M_MODE					// 0x02	(0010b)
#define OUT_OD_MODE_02M						(uint32_t)OUT_OD_MODE + OUT02M_MODE					// 0x06	(0110b)
#define AF_PP_MODE_02M						(uint32_t)AF_PP_MODE + OUT02M_MODE					// 0x0A	(1010b)
#define AF_OD_MODE_02M						(uint32_t)AF_OD_MODE + OUT02M_MODE					// 0x0E	(1110b)

#define OUTPP_MODE_50M						(uint32_t)OUT_PP_MODE + OUT50M_MODE					// 0x03	(0011b)
#define OUT_OD_MODE_50M						(uint32_t)OUT_OD_MODE + OUT50M_MODE					// 0x07	(0111b)
#define AF_PP_MODE_50M						(uint32_t)AF_PP_MODE + OUT50M_MODE					// 0x0B	(1011b)
#define AF_OD_MODE_50M						(uint32_t)AF_OD_MODE + OUT50M_MODE					// 0x0F	(1111b)


#define GPIO_CR_MASK_0						0x0000000F
#define GPIO_CR_MASK_1						0x000000F0
#define GPIO_CR_MASK_2						0x00000F00
#define GPIO_CR_MASK_3						0x0000F000
#define GPIO_CR_MASK_4						0x000F0000
#define GPIO_CR_MASK_5						0x00F00000
#define GPIO_CR_MASK_6						0x0F000000
#define GPIO_CR_MASK_7						0xF0000000

#define GPIO_CR_MASK_8						0x0000000F
#define GPIO_CR_MASK_9						0x000000F0
#define GPIO_CR_MASK_10						0x00000F00
#define GPIO_CR_MASK_11						0x0000F000
#define GPIO_CR_MASK_12						0x000F0000
#define GPIO_CR_MASK_13						0x00F00000
#define GPIO_CR_MASK_14						0x0F000000
#define GPIO_CR_MASK_15						0xF0000000


#define Release_Amplifier()				GPIOCx->ODR |= GPIO_PIN_10;
#define Release_Peripherals()			GPIOEx->ODR |= GPIO_PIN_10;

typedef enum {
  APPLICATION_IDLE = 0,  
  APPLICATION_READY,    
  APPLICATION_DISCONNECT,
}MSC_ApplicationTypeDef;


extern USBH_HandleTypeDef hUSBHost;
extern FATFS USBH_fatfs;
extern MSC_ApplicationTypeDef Appli_state;


/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void Power_Up_Initialize(void);
void Main_Loop_Proc(void);
//FRESULT Explore_Disk(char *path, uint8_t recu_level);

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
