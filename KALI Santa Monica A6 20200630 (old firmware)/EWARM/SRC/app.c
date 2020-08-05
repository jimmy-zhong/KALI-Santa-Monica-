/*--------------------------------------------------------------------------------------------------------*/
/*																																																				*/
/* Copyright(c) 2019 ~ 2020 Dongyuan Electronics Corp. All rights reserved.															  */
/*																																																				*/
/*--------------------------------------------------------------------------------------------------------*/

//**********************************************************************************************************
//
//	APPLICATION: SANTA MONICA SERIES
//	Project: kali_sm_a5_20200608
//	Source: app.c
//
//	Version: A5
//	Programmer: Rick Weng
//	Date of project start: 2019.11.27
//	Date of project release: 2020.06.??
//
//	MCU: STM32F107VCT6
//	Compiler: IAR EWARM 7.80.2
//
//	FEATURES of PROJECT
//	===================
//
//
//	MODIFICATIONS of PROJECT
//	========================
//
//	------ Version A0 ------
//
//


/**
  ******************************************************************************
  * @file    USB_Host/MSC_Standalone/Src/main.c
  * @author  MCD Application Team
  * @brief   USB Host MSC application main file.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2016 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */


/* Includes ------------------------------------------------------------------ */
#include "app.h"
#include "httputil.h"
#include "netbios.h"
#include "spix.h"
#include "stm32f1xx_ll_tim.h"
#include "device.h"
#include "tcp_proc.h"
#include "misc.h"
#include "SigmaStudioFW.h"
#include "biquads.h"


//unsigned char code_version[12] __attribute__((section(".ARM.__AT_0x08001000")))= "kali_sm_";

USBH_HandleTypeDef hUSBHost;
MSC_ApplicationTypeDef Appli_state = APPLICATION_IDLE;


extern uint8_t RGB_LUMI;
//extern uint16_t count_usb_bypass;
extern uint8_t SystemLedColor;

volatile int16_t ENC_Timeout = STOP_COUNTING;
volatile uint32_t dd3=0;

RCC_TypeDef *RCCx = ((RCC_TypeDef *)RCC_BASE);
GPIO_TypeDef *GPIOAx = ((GPIO_TypeDef *)GPIOA_BASE);
GPIO_TypeDef *GPIOBx = ((GPIO_TypeDef *)GPIOB_BASE);
GPIO_TypeDef *GPIOCx = ((GPIO_TypeDef *)GPIOC_BASE);
GPIO_TypeDef *GPIODx = ((GPIO_TypeDef *)GPIOD_BASE);
GPIO_TypeDef *GPIOEx = ((GPIO_TypeDef *)GPIOE_BASE);
AFIO_TypeDef *AFIOx = ((AFIO_TypeDef *)AFIO_BASE);

uint32_t TmpENC = 0x0000C000;
uint32_t OldTmpENC = 0x0000C000;
uint32_t ENC_CODE = 0;
uint32_t Old_ENC_CODE = 0;
uint8_t ENC_CODE_NUM = 0;
uint32_t ii = 0;
uint32_t pos = 0;

//const uint8_t zero_mem[4] = {0, 0, 0, 0};
/* Private functions --------------------------------------------------------- */


void GPIO_Configuration(void)
{
	dd3 = GPIOAx->CRL;
	dd3 = GPIOBx->CRL;
	dd3 = GPIOCx->CRL;
	dd3 = GPIODx->CRL;
	dd3 = GPIOEx->CRL;

	GPIOAx->CRL &= ~(GPIO_CR_MASK_3);
	GPIOAx->CRL |= (INPUT_PUPD << (3*4));
	GPIOAx->ODR &= 0xFFFFFFF7;
	GPIOAx->ODR |= 0x00000008;

//	GPIOBx->CRL &= ~(GPIO_CR_MASK_0);
//	GPIOBx->CRL |= (OUTPP_MODE_50M << 0);

//	GPIOBx->CRH = 0x44434444;
	GPIOBx->CRH &= ~(GPIO_CR_MASK_12);
//	xx = OUTPP_MODE_50M;
//	xx = OUT_OD_MODE_50M;
	GPIOBx->CRH |= (OUTPP_MODE_50M << (12-8)*4);
//	GPIOBx->CRH |= xx;
	GPIOBx->ODR &= 0x0000EFFF;

//	GPIOCx->CRL = 0x44444440;
//	GPIOCx->CRH = 0x44444744;
	GPIOCx->CRL &= ~(GPIO_CR_MASK_0|GPIO_CR_MASK_1);
	GPIOCx->CRL |= (INPUT_ANALOG << 0*4)+(INPUT_ANALOG << 1*4);
	GPIOCx->CRH &= ~(GPIO_CR_MASK_8|GPIO_CR_MASK_10|GPIO_CR_MASK_13|GPIO_CR_MASK_14|GPIO_CR_MASK_15);
	GPIOCx->CRH |= (OUT_OD_MODE_50M << (8-8)*4)+(OUT_OD_MODE_50M << (10-8)*4)+(INPUT_PUPD << (13-8)*4)
									+(INPUT_PUPD << (14-8)*4)+(INPUT_PUPD << (15-8)*4);
	GPIOCx->ODR &= 0xFFFF1FFF;
	GPIOCx->ODR |= 0x0000E000;

	GPIODx->CRL &= (~GPIO_CR_MASK_4);
	GPIODx->CRL |= (OUTPP_MODE_50M << 4*4);
	GPIODx->ODR &= 0xFFFFFFEF;

//	GPIOEx->CRL = 0x88888888;
//	GPIOEx->CRH = 0x44B4B7B8;
//	GPIOEx->ODR = 0x000001FF;

	GPIOEx->CRL &= ~(GPIO_CR_MASK_0 | GPIO_CR_MASK_1 | GPIO_CR_MASK_2 | GPIO_CR_MASK_3 |
									 GPIO_CR_MASK_4 | GPIO_CR_MASK_5 | GPIO_CR_MASK_6 | GPIO_CR_MASK_7);
//	GPIOEx->CRL |= (INPUT_PUPD << 0) + (INPUT_PUPD << (1*4)) + (INPUT_PUPD << (2*4)) + (INPUT_PUPD << (3*4)) +
//								 (INPUT_PUPD << (4*4)) + (INPUT_PUPD << (5*4)) + (INPUT_PUPD << (6*4)) + (INPUT_PUPD << (7*4));
	GPIOEx->CRL |= (INPUT_FLOAT << 0) + (INPUT_FLOAT << (1*4)) + (INPUT_FLOAT << (2*4)) + (INPUT_FLOAT << (3*4)) +
								 (INPUT_FLOAT << (4*4)) + (INPUT_FLOAT << (5*4)) + (INPUT_FLOAT << (6*4)) + (INPUT_FLOAT << (7*4));
	GPIOEx->CRH &= ~(GPIO_CR_MASK_8 | GPIO_CR_MASK_9 | GPIO_CR_MASK_10 | GPIO_CR_MASK_11 | GPIO_CR_MASK_13);
	GPIOEx->CRH |= (INPUT_PUPD << ((8-8)*4)) + (AF_PP_MODE_50M << ((9-8)*4)) + (OUT_OD_MODE_50M << ((10-8)*4)) + (AF_PP_MODE_50M << ((11-8)*4)) +
								 (AF_PP_MODE_50M << ((13-8)*4));
//	GPIOEx->ODR &= 0xFFFFFE00;
//	GPIOEx->ODR |= 0x000001FF;

	__HAL_AFIO_REMAP_TIM1_ENABLE();
}


/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow : 
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 48000000
  *            HCLK(Hz)                       = 72000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 2
  *            APB2 Prescaler                 = 1
  *            HSE Frequency(Hz)              = 25000000
  *            HSE PREDIV1                    = 5
  *            HSE PREDIV2                    = 5
  *            PLL2MUL                        = 8
  *            Flash Latency(WS)              = 2
  * @param  None
  * @retval None
  */


void SystemClock_Config(void)
{
	RCC->CR |= RCC_CR_HSEON;											// 0x00010000
	while ((RCC->CR & RCC_CR_HSERDY)==0);					// 0x00020000

	RCCx->CFGR &= (uint32_t)~(RCC_CFGR_PLLXTPRE | RCC_CFGR_PLLSRC | RCC_CFGR_PLLMULL | RCC_CFGR_SW);

/*// HSI
	RCC->CFGR |= (uint32_t)(RCC_CFGR_PLLXTPRE_PREDIV1 | RCC_CFGR_SW_PLL | RCC_CFGR_OTGFSPRE |
													RCC_CFGR_PLLMULLx12 | RCC_CFGR_MCO_SYSCLK | RCC_HCLK_DIV1);
*/

// HSE
//	RCC->CFGR |= (uint32_t)(RCC_CFGR_PLLXTPRE_PREDIV1 | RCC_CFGR_SW_PLL | RCC_CFGR_OTGFSPRE |
//													RCC_CFGR_MCOSEL_SYSCLK | RCC_HCLK_DIV1 | RCC_CFGR_PLLSRC | RCC_CFGR_PLLMULL6);
	RCC->CFGR |= (uint32_t)(RCC_CFGR_PLLXTPRE_PREDIV1 | RCC_CFGR_SW_PLL | RCC_CFGR_OTGFSPRE |
													RCC_HCLK_DIV1 | RCC_CFGR_PLLSRC | RCC_CFGR_PLLMULL6);
	RCC->CFGR2 &= (uint32_t)~(RCC_CFGR2_PREDIV1 | RCC_CFGR2_PREDIV1SRC |
														RCC_CFGR2_PREDIV2 | RCC_CFGR2_PLL2MUL);
	RCC->CFGR2 |= (RCC_CFGR2_PREDIV1_DIV5 | RCC_CFGR2_PLL2MUL8 | RCC_CFGR2_PREDIV1SRC_PLL2 |
								 RCC_CFGR2_PREDIV2_DIV5);

	RCC->CR |= RCC_CR_PLL2ON;											// 0x04000000
	while ((RCC->CR & RCC_CR_PLL2RDY)==0);				// 0x08000000

	RCC->CR |= RCC_CR_PLLON;											// 0x01000000
	while ((RCC->CR & RCC_CR_PLLRDY)==0);					// 0x02000000

	RCC->AHBENR |= RCC_AHBENR_OTGFSEN;

	SystemCoreClockUpdate();
}


void Enable_Peripheral_Clocks(void)
{
	__HAL_RCC_TIM1_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOE_CLK_ENABLE();
	__HAL_RCC_AFIO_CLK_ENABLE();
	__HAL_RCC_ADC1_CLK_ENABLE();

	__HAL_RCC_TIM2_CLK_ENABLE();

	__HAL_RCC_DMA1_CLK_ENABLE();
	__HAL_RCC_CRC_CLK_ENABLE();
}
/*
RCC_APB2ENR_TIM1EN
RCC_APB2ENR_IOPAEN +
RCC_APB2ENR_IOPBEN +
RCC_APB2ENR_IOPCEN +
RCC_APB2ENR_IOPDEN +
RCC_APB2ENR_IOPEEN +
RCC_APB2ENR_AFIOEN +
RCC_APB2ENR_ADC1EN +
*/
/*
RCC_APB1ENR_TIM2EN
*/
/*
RCC_AHBENR_DMA1EN
*/

void NVIC_Enable(void)
{
//	HAL_NVIC_EnableIRQ(TIM1_UP_IRQn);
	HAL_NVIC_EnableIRQ(TIM2_IRQn);
	HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
	HAL_NVIC_EnableIRQ(ADC1_2_IRQn);
}


//ENC_CW =0x78, 0x87
//ENC_CCW = 0x4B, 0xB4

void Encoder_Loop(void)
{
	TmpENC = (GPIOEx->IDR & 0xC000) >> 14;
	if (TmpENC != OldTmpENC)
	{
		ENC_Timeout = 40;
		OldTmpENC = TmpENC;
		ENC_CODE = (ENC_CODE << 2) + OldTmpENC;
		pos++;
		if (pos>3)
		{
			if (ENC_CODE != Old_ENC_CODE)
			{
				ENC_CODE_NUM = 1;
				Old_ENC_CODE = ENC_CODE;
			}
			else
			{
				ENC_CODE_NUM++;
			}
			ENC_CODE = 0;
			pos = 0;
		}
		
		if (ENC_CODE_NUM < 4) return;

		if ((Old_ENC_CODE == 0x4B) || (Old_ENC_CODE == 0xB4))
		{
			switch_dsp_volume(+1);
		}
		else if ((Old_ENC_CODE == 0x78) || (Old_ENC_CODE == 0x87))
		{
			switch_dsp_volume(-1);
		}
	}
	else
	{
		if (ENC_Timeout==0)
		{
			ENC_Timeout = STOP_COUNTING;
			pos = 0;
			ENC_CODE_NUM = 0;
			ENC_CODE = 0;
		}
	}
}


/**
  * @brief  MSC application Init.
  * @param  None
  * @retval None
  */
static void MSC_InitApplication(void)
{
	Enable_Peripheral_Clocks();
	NVIC_Enable();

	GPIO_Configuration();
	TIM1_Init();
	SystemLedColor = LED_AMBER;
	Switch_LED(SystemLedColor);

	tcp_init();

	tim2_init();
	wait_1ms(2000);
	GPIODx->ODR |= 0x00000010;
//	__enable_irq();
	Init_ADC1x();
}


/**
	* @brief  User Process
	* @param  phost: Host Handle
	* @param  id: Host Library user message ID
	* @retval None
	*/
static void USBH_UserProcess(USBH_HandleTypeDef * phost, uint8_t id)
{
  switch (id)
  {
		case HOST_USER_SELECT_CONFIGURATION:
		break;

		case HOST_USER_DISCONNECTION:
			Appli_state = APPLICATION_DISCONNECT;
			if (f_mount(NULL, "", 0) != FR_OK)
			{
//				LCD_ErrLog("ERROR : Cannot DeInitialize FatFs!\n");
			}
		break;

		case HOST_USER_CLASS_ACTIVE:
			Appli_state = APPLICATION_READY;
		break;

		case HOST_USER_CONNECTION:
			if (f_mount(&USBH_fatfs, "", 0) != FR_OK)
			{
//				LCD_ErrLog("ERROR : Cannot Initialize FatFs!\n");
			}
		break;

		default:
		break;
  }
}


void Power_Up_Initialize(void)
{
	HAL_Init();																					/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	SystemClock_Config();																/* Configure the system clock to 48 Mhz */
	MSC_InitApplication();															/* Init MSC Application */

	FLASH_SPI_Init();

	USBH_Init(&hUSBHost, USBH_UserProcess, 0);					/* Init Host Library		*/
	USBH_RegisterClass(&hUSBHost, USBH_MSC_CLASS);			/* Add Supported Class	*/
	USBH_Start(&hUSBHost);															/* Start Host Process		*/

	wait_300ms_power_stable();
	
//	while (1);
	USBH_Stop(&hUSBHost);																/* Start Host Process		*/
	USBH_DeInit(&hUSBHost);															/* Init Host Library		*/
//	GPIOCx->ODR |= GPIO_PIN_10;
//	GPIOEx->ODR |= GPIO_PIN_10;
	Release_Amplifier();																// GPIOCx->ODR |= GPIO_PIN_10;
	Release_Peripherals();															// GPIOEx->ODR |= GPIO_PIN_10;

//  Calculate_biquads(uint8_t EQ_NO, uint8_t BD_NO, uint8_t EqType, double Fc, double Qf, double Gain)
//
//	Calculate_biquads(USER_EQ1, EQ_BAND1, FIR_LPI_ST, 1500, 0, 0);
//	Calculate_biquads(USER_EQ1, EQ_BAND2, FIR_LPI_ADI, 1060, 0, 0);
//	Calculate_biquads(USER_EQ1, EQ_BAND3, FIR_HPI_ST, 1000, 0, 0);
//	Calculate_biquads(USER_EQ1, EQ_BAND4, FIR_HPI_ADI, 250, 0, 0);
//	Calculate_biquads(USER_EQ2, EQ_BAND1, FIR_LPII, 1500, 0.7, 0);
//	Calculate_biquads(USER_EQ2, EQ_BAND1, FIR_HPII, 120, 0.9, 0);
//	Calculate_biquads(USER_EQ3, EQ_BAND1, FIR_LSHELF, 1500,2, 3);
//	Calculate_biquads(USER_EQ3, EQ_BAND2, FIR_HSHELF, 3000, 6, -3);
//	Calculate_biquads(USER_EQ4, EQ_BAND1, FIR_PEQ, 9600, 10, -3.75);
//	Calculate_biquads(USER_EQ4, EQ_BAND2, FIR_PEQ, 1800, 1.41, 0);
//	Calculate_biquads(USER_EQ4, EQ_BAND2, FIR_NOTCH, 1000, 1.41, -2);
//	Calculate_biquads(USER_EQ5, EQ_BAND1, FIR_AllPass, 350, 1.4, -2);

	Load_User_Biquads();

	SystemLedColor = LED_BLUE;
	Switch_LED(LED_GREEN);
//	init_dhcp_client();
}


void Main_Loop_Proc(void)
{
	while (1)
	{
		W5500_LOOP();
		Switch_Testpin();
		Switch_PE0_7();
		Encoder_Loop();
		check_adc_input_data();
		check_power_status();
		Examine_LED_Status();
		sending_local_status();
	}
}

