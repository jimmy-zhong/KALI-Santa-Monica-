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
#ifndef __MISC_H__
#define __MISC_H__

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"
#include "stdio.h"


#define STOP_COUNTING													-1


#define LED_BLACK															0
#define LED_GREEN															1
#define LED_BLUE															2
#define LED_RED																3
#define LED_AMBER															4
#define LED_CYAN															5
#define LED_MAGENTA														6
#define LED_WHITE															7


#define MINVOLSTEP														0								// STEP 0: -12dB
//#define MAXVOLSTEP														36
#define MAXVOLSTEP														88
#define VOLSTEP0dB														24

#define VOL_AVE_COUNT													TOTAL_VOLSTEPS - 1

#define EVENT_NULL														0
#define EVENT_VOL0DB													1
#define EVENT_VOLMAXMIN												2

#define TrimLevel_0dB													64
#define TrimLevel_p12dB												TrimLevel_0dB+24
#define TrimLevel_p6dB												TrimLevel_0dB+12
#define TrimLevel_n6dB												TrimLevel_0dB-12
#define TrimLevel_n12dB												TrimLevel_0dB-24
#define TrimLevel_n18dB												TrimLevel_0dB-36
#define TrimLevel_n24dB												TrimLevel_0dB-48
#define TrimLevel_n30dB												TrimLevel_0dB-60
#define TrimLevel_Min													-TrimLevel_0dB / 2
		
#define NO_DIM																0
#define DIM_minus20dB													-40
		
		
#define TESTMODE_0														0
#define TESTMODE_1														1
#define TESTMODE_2														2
#define TESTMODE_3														3
#define TESTMODE_4														4
#define TESTMODE_5														5
#define TESTMODE_6														6		// Test mode 6: release control of DSP
#define TESTMODE_7														7

#define NON_TEST_MODE													TESTMODE_7


#define MODE0TO1															(TESTMODE_0 << 8)+TESTMODE_1
#define MODE0TO2															(TESTMODE_0 << 8)+TESTMODE_2
#define MODE0TO3															(TESTMODE_0 << 8)+TESTMODE_3
#define MODE0TO4															(TESTMODE_0 << 8)+TESTMODE_4
#define MODE0TO5															(TESTMODE_0 << 8)+TESTMODE_5
#define MODE0TO6															(TESTMODE_0 << 8)+TESTMODE_6
#define MODE0TO7															(TESTMODE_0 << 8)+TESTMODE_7

#define MODE1TO0															(TESTMODE_1 << 8)+TESTMODE_0
#define MODE1TO2															(TESTMODE_1 << 8)+TESTMODE_2
#define MODE1TO3															(TESTMODE_1 << 8)+TESTMODE_3
#define MODE1TO4															(TESTMODE_1 << 8)+TESTMODE_4
#define MODE1TO5															(TESTMODE_1 << 8)+TESTMODE_5
#define MODE1TO6															(TESTMODE_1 << 8)+TESTMODE_6
#define MODE1TO7															(TESTMODE_1 << 8)+TESTMODE_7

#define MODE2TO0															(TESTMODE_2 << 8)+TESTMODE_0
#define MODE2TO1															(TESTMODE_2 << 8)+TESTMODE_1
#define MODE2TO3															(TESTMODE_2 << 8)+TESTMODE_3
#define MODE2TO4															(TESTMODE_2 << 8)+TESTMODE_4
#define MODE2TO5															(TESTMODE_2 << 8)+TESTMODE_5
#define MODE2TO6															(TESTMODE_2 << 8)+TESTMODE_6
#define MODE2TO7															(TESTMODE_2 << 8)+TESTMODE_7

#define MODE3TO0															(TESTMODE_3 << 8)+TESTMODE_0
#define MODE3TO1															(TESTMODE_3 << 8)+TESTMODE_1
#define MODE3TO2															(TESTMODE_3 << 8)+TESTMODE_2
#define MODE3TO4															(TESTMODE_3 << 8)+TESTMODE_4
#define MODE3TO5															(TESTMODE_3 << 8)+TESTMODE_5
#define MODE3TO6															(TESTMODE_3 << 8)+TESTMODE_6
#define MODE3TO7															(TESTMODE_3 << 8)+TESTMODE_7

#define MODE4TO0															(TESTMODE_4 << 8)+TESTMODE_0
#define MODE4TO1															(TESTMODE_4 << 8)+TESTMODE_1
#define MODE4TO2															(TESTMODE_4 << 8)+TESTMODE_2
#define MODE4TO3															(TESTMODE_4 << 8)+TESTMODE_3
#define MODE4TO5															(TESTMODE_4 << 8)+TESTMODE_5
#define MODE4TO6															(TESTMODE_4 << 8)+TESTMODE_6
#define MODE4TO7															(TESTMODE_4 << 8)+TESTMODE_7

#define MODE5TO0															(TESTMODE_5 << 8)+TESTMODE_0
#define MODE5TO1															(TESTMODE_5 << 8)+TESTMODE_1
#define MODE5TO2															(TESTMODE_5 << 8)+TESTMODE_2
#define MODE5TO3															(TESTMODE_5 << 8)+TESTMODE_3
#define MODE5TO4															(TESTMODE_5 << 8)+TESTMODE_4
#define MODE5TO6															(TESTMODE_5 << 8)+TESTMODE_6
#define MODE5TO7															(TESTMODE_5 << 8)+TESTMODE_7

#define MODE6TO0															(TESTMODE_6 << 8)+TESTMODE_0
#define MODE6TO1															(TESTMODE_6 << 8)+TESTMODE_1
#define MODE6TO2															(TESTMODE_6 << 8)+TESTMODE_2
#define MODE6TO3															(TESTMODE_6 << 8)+TESTMODE_3
#define MODE6TO4															(TESTMODE_6 << 8)+TESTMODE_4
#define MODE6TO5															(TESTMODE_6 << 8)+TESTMODE_5
#define MODE6TO7															(TESTMODE_6 << 8)+TESTMODE_7

#define MODE7TO0															(TESTMODE_7 << 8)+TESTMODE_0
#define MODE7TO1															(TESTMODE_7 << 8)+TESTMODE_1
#define MODE7TO2															(TESTMODE_7 << 8)+TESTMODE_2
#define MODE7TO3															(TESTMODE_7 << 8)+TESTMODE_3
#define MODE7TO4															(TESTMODE_7 << 8)+TESTMODE_4
#define MODE7TO5															(TESTMODE_7 << 8)+TESTMODE_5
#define MODE7TO6															(TESTMODE_7 << 8)+TESTMODE_6


typedef struct {
	uint8_t CTRL_SpkControls;
	uint8_t CTRL_delay;
	uint8_t CTRL_dim;
	uint8_t CTRL_SPKEQPARAMS;
	uint8_t CTRL_preset;
	uint8_t CTRL_led_en;
	uint8_t CTRL_mute;
	uint16_t CTRL_stdby_delay;
	uint8_t CTRL_stdby_en;
	int16_t CTRL_trimlevel;
	uint8_t CTRL_dipsw_en;
} SYSTEM_CONFIG_STRUCT;



/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void TIM1_Init(void);
void TIM1_UP_IRQHandler(void);
void tim2_init(void);
void wait_1ms(uint16_t ms);
void TIM2_IRQHandler(void);
void Adjust_RGBLED(uint8_t idx);
void Error_LED(void);
void Linked_LED(void);
void Switch_LED(uint8_t lled);
void Init_ADC1x(void);
void check_adc_input_data(void);
void Switch_PE0_7(void);
void Switch_Testpin(void);
void wait_300ms_power_stable(void);
void check_power_status(void);
//void check_eq_switch(uint16_t TmpEQ);
//void check_eq_switch_1(uint16_t TmpEQ);
//void check_hf_switch(uint16_t TmpHF);
//void check_mf_switch(uint16_t TmpMF);
//void check_mf_switch_1(uint16_t TmpMF);
//void check_reldsp_switch(uint16_t TmpRelDSP);
void DspMasterVolume(int8_t dir);
void switch_dsp_volume(int8_t dir);
void Examine_LED_Status(void);
void SIGMA_MODIFY_EQ(uint32_t *pp);


#endif /* __MISC_H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
