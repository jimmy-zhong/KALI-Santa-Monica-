/*--------------------------------------------------------------------------------------------------------*/
/*																																																				*/
/* Copyright(c) 2018 ~ 2020 Dongyuan Electronics Corp. All rights reserved.															  */
/*																																																				*/
/*--------------------------------------------------------------------------------------------------------*/

//**********************************************************************************************************
//
//	APPLICATION: LP SERIES (LP6/LP8/IN-8)
//	Project: lp_all_v2.00_20200330
//	Source: adc1.c 
//
//	Version: V2.0.0
//	Programmer: Rick Weng
//	Date of project start: 2020.03.30
//	Date of project release: 2020.03.31
//
//	MCU: STM32F030K6T6
//	Compiler: Keil uV5.17.0.0
//
//	FEATURES of THIS FILE
//	=====================
//
//
//	MODIFICATIONS of THIS FILE
//	==========================
//
//	------ Version V2.00 ------
//
//	(NONE)
//
//


#include <stm32f0xx_ll_adc.h>
#include <stm32f0xx_ll_dma.h>
#include "tim3.h"
#include "sys_def.h"
#include "sta3xx.h"


#define MAX_ADC_PTR									1

#define ADC_TOP_MAX									0x3FC0
//#define	ADC_CENTER_MAX							0x2040
//#define	ADC_CENTER_MIN							0x1FC0
#define	ADC_CENTER_MAX							0x2200
#define	ADC_CENTER_MIN							0x1E00
#define	ADC_LOW_MIN									0x0040

#define ADC_LEFT_RANGE							(ADC_CENTER_MIN-ADC_LOW_MIN)

#define ADC_CENTER_dB								13	// -6dB

//#define SMALLEST_dB_FACTOR					90					// -83.5dB
#define SMALLEST_dB_FACTOR					100					// -75.5dB
//#define SMALLEST_dB_FACTOR					125					// -62dB
//#define SMALLEST_dB_FACTOR					150					// -52.5dB


volatile uint32_t ADC_INPUT_Buffer[MAX_ADC_PTR+2];
volatile uint32_t ADC_value[MAX_ADC_PTR+1];
volatile uint8_t adc_ptr=0;
volatile uint16_t DC_VOL = 0;
volatile uint8_t volume_level;
volatile uint8_t temp_level[32];

volatile int8_t adc_precount=7;

volatile uint32_t temp_data[32];
volatile uint8_t temp_cnt=0;
volatile uint32_t adc_tt;
volatile _Bool ADC_FLAG = 0;

volatile uint8_t xx=0;
volatile uint32_t maxadcv=0;
volatile uint32_t minadcv=0;

volatile uint32_t long_adc_value=0;

extern int8_t last_adc_read;
extern uint8_t device_vol;
extern eSPEAKER_MODEL SPEAKER_MODEL;
extern uint8_t Tone_Control;
extern uint8_t Old_Tone_Control;
extern uint8_t actual_vol;

extern const uint8_t STA3xx_tone_control[3][2][3];
extern const uint8_t STA3xx_VOLUME_VALUE[19];


/*const uint16_t vol_adc_value[18] = {

0x0066,		// 1
0x0155,		// 2
0x02B7,		// 3
0x045C,		// 4
0x077F,		// 5
0x0E3D,		// 6
0x1650,		// 7
0x1CD5,		// 8
0x22A6,		// 9
0x281C,		// 10
0x2D4E,		// 11
0x324E,		// 12
0x372A,		// 13
0x3B5D,		// 14
0x3D8D,		// 15
0x3E7F,		// 16
0x3F41,		// 17
0x3FC2,		// 18
};*/

//volatile uint32_t volxx[32][2];


void renew_volume_level(void)
{	uint8_t i;
	uint32_t tmp;
	uint32_t tmpxx;
	uint32_t tmpx2;
	uint32_t tmpx3;

	temp_data[temp_cnt]=ADC_value[0];
	long_adc_value=long_adc_value*31/32+ADC_value[0];
	temp_cnt++;
	if (temp_cnt>31)
		temp_cnt=0;
	tmp = 0;
	for (i=0; i<32; i++)
	{
		tmp += temp_data[i];
	}
	
//	tmp = long_adc_value;

/*	if (tmp>=ADC_CENTER_MIN)
		__asm("NOP");

	if (tmp<=ADC_CENTER_MAX)
		__asm("NOP");

	if ((tmp>ADC_CENTER_MAX) || (tmp<ADC_CENTER_MIN))
	{
		__asm("NOP");
	}
	else
	{
		actual_vol = sta3xx_get_VOLUME();
		if (actual_vol != 0x0C)
			__asm("NOP");
	}*/
	
	if (tmp>ADC_TOP_MAX)
	{
		device_vol = 0x00;
	}
	else if (tmp<ADC_LOW_MIN)
			 {
				 device_vol = 0xFF;
			 }
			 else if (tmp<ADC_CENTER_MIN)
						{
							tmpxx=(ADC_CENTER_MIN-tmp)*300/ADC_CENTER_MIN;
							tmpx2=tmpxx*tmpxx;
							tmpx3=tmpx2*1000/8192+tmpxx*1000/100;
							device_vol = tmpx3/SMALLEST_dB_FACTOR+13;
						}
						else if (tmp>ADC_CENTER_MAX)
							{
								device_vol = (ADC_TOP_MAX-tmp)*12/(ADC_TOP_MAX-ADC_CENTER_MAX);
							}
							else device_vol = 0x0C;

//	if (device_vol!=0x0C)
//		tmp = 0;
}


void check_adc_input_data(void)
{
	if (last_adc_read==0)
	{
		LL_ADC_REG_StartConversion(ADC1);
//		last_adc_read=-1;
		Freeze_counter(last_adc_read);
	}

	if (ADC_FLAG==1)
	{
		ADC_FLAG=0;
		ADC_value[0] += ADC_INPUT_Buffer[0]*2/32;
		ADC_value[1] += ADC_INPUT_Buffer[1];

		switch (adc_ptr)
		{
			case 0:
				renew_volume_level();
				temp_level[temp_cnt]=volume_level;
//				device_vol = STA3xx_VOLUME_VALUE[volume_level];
			break;
			case 1: 
				DC_VOL = ADC_value[1];
			break;
		}
		if (adc_precount>=0) adc_precount--;

		ADC_value[adc_ptr] = 0;
		adc_ptr++;
		if (adc_ptr>MAX_ADC_PTR)
			adc_ptr = 0;
		last_adc_read=2;
	}
}


void adc_pre_run(void)
{
	while (adc_precount>=0)
	{
		check_adc_input_data();
	}
}


void Init_ADC1(void)
{//	uint8_t CALX;

	LL_ADC_DeInit(ADC1);

	LL_DMA_ConfigAddresses(DMA1, LL_DMA_CHANNEL_1,
												 LL_ADC_DMA_GetRegAddr(ADC1, LL_ADC_DMA_REG_REGULAR_DATA),
												 (uint32_t)&ADC_INPUT_Buffer[0],
												 LL_DMA_DIRECTION_PERIPH_TO_MEMORY);

	LL_DMA_ConfigTransfer(DMA1, LL_DMA_CHANNEL_1, LL_DMA_DIRECTION_PERIPH_TO_MEMORY
																							| LL_DMA_MODE_CIRCULAR
																							| LL_DMA_PERIPH_NOINCREMENT
																							| LL_DMA_MEMORY_INCREMENT
																							| LL_DMA_PDATAALIGN_WORD
																							| LL_DMA_MDATAALIGN_WORD
																							| LL_DMA_PRIORITY_LOW);

	LL_ADC_StartCalibration(ADC1);
	while (LL_ADC_IsCalibrationOnGoing(ADC1));
//	CALX = ADC1->DR;

	LL_ADC_REG_SetSequencerChannels(ADC1, ADC_CHSELR_CHSEL0 | ADC_CHSELR_CHSEL1
																| ADC_CHSELR_CHSEL17);

	LL_ADC_SetSamplingTimeCommonChannels(ADC1, ADC_SMPR_SMP_0 | ADC_SMPR_SMP_1 | ADC_SMPR_SMP_2);
	LL_ADC_REG_SetTriggerSource(ADC1, LL_ADC_REG_TRIG_SOFTWARE);
	LL_ADC_REG_SetDMATransfer(ADC1, LL_ADC_REG_DMA_TRANSFER_UNLIMITED);
	LL_ADC_SetCommonPathInternalCh(ADC, LL_ADC_PATH_INTERNAL_VREFINT);

	LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_1, 3);
	LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_1);
	LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_1);

	LL_ADC_EnableIT_ADRDY(ADC1);
	LL_ADC_EnableIT_EOC(ADC1);
	LL_ADC_EnableIT_EOS(ADC1);
	LL_ADC_EnableIT_OVR(ADC1);
	LL_ADC_EnableIT_EOSMP(ADC1);
	LL_ADC_EnableIT_AWD1(ADC1);

	LL_ADC_Enable(ADC1);
	ADC_FLAG = 0;
	LL_ADC_REG_StartConversion(ADC1);
	
//	equation_constant = (uint32_t)SMALLEST_DB_EQUATION;
//	equation_constant = equation_constant*100000;
//	equation_constant = equation_constant/ADC_LEFT_RANGE;
	adc_pre_run();
}


void ADC1_IRQHandler(void)
{
	LL_ADC_ClearFlag_ADRDY(ADC1);
	LL_ADC_ClearFlag_EOC(ADC1);
	LL_ADC_ClearFlag_EOS(ADC1);
	LL_ADC_ClearFlag_OVR(ADC1);
	LL_ADC_ClearFlag_EOSMP(ADC1);
	LL_ADC_ClearFlag_AWD1(ADC1);
}


void DMA1_Channel1_IRQHandler(void)
{
	LL_DMA_ClearFlag_GI1(DMA1);
	ADC_FLAG = 1;
	last_adc_read = 5;
}


uint8_t read_power_status(void)
{	uint8_t temp_key;

	if (DC_VOL>=POWER_OK_LOBOUND)
		temp_key=0;
	else
		temp_key=1;
	
	return (temp_key);
}


uint8_t Vol_level_IS_Zero(void)
{
	return (volume_level==0);
}


