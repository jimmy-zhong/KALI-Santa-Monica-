/*--------------------------------------------------------------------------------------------------------*/
/*																																																				*/
/* Copyright(c) 2019 ~ 2020 Dongyuan Electronics Corp. All rights reserved.															  */
/*																																																				*/
/*--------------------------------------------------------------------------------------------------------*/

//**********************************************************************************************************
//
//	APPLICATION: SANTA MONICA SERIES
//	Project: kali_sm_a5_20200608
//	Source: misc.c 
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


/* Includes ------------------------------------------------------------------ */
#include "stm32f1xx_ll_tim.h"
#include "app.h"
#include "misc.h"
#include "usbh_def.h"
#include "spix.h"
#include "stm32f1xx_ll_adc.h"
#include "stm32f1xx_ll_dma.h"
//#include "Santa_Monica_IC_1.h"
//#include "Santa_Monica_IC_1_PARAM.h"
#include "biquads.h"


/************ FOR "Santa_Monica_IC_1.h"

#include "Santa_Monica_IC_1_REG.h"

const ADI_REG_TYPE DM1_DATA_Data_IC_1[DM1_DATA_SIZE_IC_1] = {

const ADI_REG_TYPE Program_Data_IC_1[PROGRAM_SIZE_IC_1] = {

const ADI_REG_TYPE Param_Data_IC_1[PARAM_SIZE_IC_1] = {

	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, PROGRAM_ADDR_IC_1, PROGRAM_SIZE_IC_1, (uint8_t *)&Program_Data_IC_1 );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, PARAM_ADDR_IC_1, PARAM_SIZE_IC_1, (uint8_t *)&Param_Data_IC_1 );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, DM1_DATA_ADDR_IC_1, DM1_DATA_SIZE_IC_1, (uint8_t *)&DM1_DATA_Data_IC_1 );

***********/

#define MAX_ADC_PTR									1


extern int16_t ENC_Timeout;
extern GPIO_TypeDef *GPIOCx;
extern GPIO_TypeDef *GPIOEx;
extern USBH_HandleTypeDef hUSBHost;
extern MSC_ApplicationTypeDef Appli_state;
extern uint8_t LocStatus;
extern BQ_TYPE UserBQ[8][8];
extern uint16_t dip_volndx[8];

extern int16_t count_OnOff;
extern int16_t count_TestVol;
extern int16_t count_EQ;
extern int16_t count_EQ1;
extern int16_t count_HF;
extern int16_t count_MF;
extern int16_t count_LIM;
extern int16_t count_testmode;
extern uint16_t count_wait;
extern uint16_t count_usb_bypass;
extern uint32_t Volume_Table[MAXVOLSTEP+1];


/* Private function prototypes ----------------------------------------------- */
//TIM_TypeDef *tim2x = ((TIM_TypeDef *)TIM2_BASE);
//TIM_TypeDef *timx = ((TIM_TypeDef *)TIM1_BASE);

volatile uint8_t CurrentEventId = EVENT_NULL;
volatile uint8_t SystemLedColor = 0;


volatile uint8_t OldOnOffSW = 0xFF;
volatile uint8_t OldtVolSW = 0xFF;
volatile uint8_t OldEQSW = 0xFF;
volatile uint8_t OldEQSW1 = 0xFF;
volatile uint8_t OldHFSW = 0xFF;
volatile uint8_t OldMFSW = 0xFF;
volatile uint8_t OldLIMSW = 0xFF;

volatile uint8_t OldTestMode = 0xFF;

volatile uint8_t LocalCtrlEnabled = TRUE;
//volatile int8_t CurrentVolStep = MAXVOLSTEP / 2;
//volatile int8_t CurrentVolStep = TrimLevel_n12dB;
//volatile int8_t CurrentVolStep = TrimLevel_n6dB;
volatile int8_t CurrentVolStep = TrimLevel_0dB;
//volatile int8_t CurrentVolStep = TrimLevel_p6dB;
//volatile int8_t CurrentVolStep = TrimLevel_p12dB;
volatile int8_t CurrentDIM = 0;

/*
//volatile uint8_t LEDidx = 7;


volatile int8_t FlashColor[3] = {-1, -1, -1};
volatile uint16_t FlashPeriod[3] = {0, 0, 0};
volatile int8_t FlashCycle = -1;
volatile int8_t FlashStep = -1;
volatile int16_t count_led_step = STOP_COUNTING;
volatile int16_t count_local_status = STOP_COUNTING;
volatile uint16_t DspEqOffset = 0;


char PREFIX_FIRMWARE_UPGRADE[] = "kali_sm_";
*/


/* Private functions --------------------------------------------------------- */


/*
void TIM1_Init(void)
{
	timx->PSC = 0x0E80;			// (or 0x0EA6)
	timx->ARR = 0x0100;

	timx->CCER = TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC3E;									// 0x0111
//	timx->CCER = TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC3E |
//							 TIM_CCER_CC1P | TIM_CCER_CC2P | TIM_CCER_CC3P;									// 0x0333

	timx->CCMR1 = TIM_CCMR1_OC1PE + TIM_CCMR1_OC1M_1 + TIM_CCMR1_OC1M_2 +				// 0x6868
								TIM_CCMR1_OC2PE + TIM_CCMR1_OC2M_1 + TIM_CCMR1_OC2M_2;
	timx->CCMR2 = TIM_CCMR2_OC3PE + TIM_CCMR2_OC3M_1 + TIM_CCMR2_OC3M_2;				// 0x0068
	timx->BDTR = TIM_BDTR_AOE | TIM_BDTR_MOE;																		// 0xC000;
	timx->CR1 = TIM_CR1_ARPE;
	timx->CR1 |= TIM_CR1_CEN;
}


void TIM1_UP_IRQHandler(void)
{
	timx->SR = 0;
}


void tim2_init(void)
{
	LL_TIM_SetPrescaler(tim2x, 47);									// prescaler = 47+1 --> one tick = 1us
	LL_TIM_SetAutoReload(tim2x, 1000);							// 1000 ticks = 1ms
	LL_TIM_SetClockDivision(tim2x, LL_TIM_CLOCKDIVISION_DIV1);
	LL_TIM_EnableIT_UPDATE(tim2x);
	LL_TIM_EnableARRPreload(tim2x);
	LL_TIM_EnableCounter(tim2x);
}


void wait_1ms(uint16_t ms)
{
	count_wait = ms;
	while (count_wait);
}


void TIM2_IRQHandler(void)
{
	if (LL_TIM_IsActiveFlag_UPDATE(tim2x)) LL_TIM_ClearFlag_UPDATE(tim2x);
	if (count_wait>0) count_wait--;
	if (ENC_Timeout>0) ENC_Timeout--;
	if (count_usb_bypass>0) count_usb_bypass--;
	if (last_adc_read>0) last_adc_read--;
	if (por_ms>0) por_ms--;
	if (count_mPower>0) count_mPower--;
	if (count_EQ>0) count_EQ--;
	if (count_EQ1>0) count_EQ1--;
	if (count_HF>0) count_HF--;
	if (count_MF>0) count_MF--;
	if (count_testmode>0) count_testmode--;
	if (count_led_step>0) count_led_step--;
	if (count_local_status>0) count_local_status--;
	if (count_OnOff>0) count_OnOff--;
	if (count_TestVol>0) count_TestVol--;
	if (count_LIM>0) count_LIM--;
}


void Adjust_RGBLED(uint8_t idx)
{
	timx->CCR1 = RGB_DUTY[idx][0];
	timx->CCR2 = RGB_DUTY[idx][1];
	timx->CCR3 = RGB_DUTY[idx][2];
}


void Error_LED(void)
{
	Adjust_RGBLED(LED_RED);
}

void Linked_LED(void)
{
//	LEDidx = 1;
	Adjust_RGBLED(LED_GREEN);
}


void Switch_LED(uint8_t lled)
{
//	LEDidx = lled;
	Adjust_RGBLED(lled);
}


void Blink_EventLED(uint8_t event_id)
{
	switch (event_id)
	{
		case EVENT_VOL0DB:
			FlashColor[0] = LED_WHITE;
			FlashColor[1] = SystemLedColor;
			FlashPeriod[0] = 1000;
			FlashPeriod[1] = 1000;
			FlashCycle = 1;
		break;
		case EVENT_VOLMAXMIN:
			FlashColor[0] = LED_WHITE;
			FlashColor[1] = SystemLedColor;
			FlashPeriod[0] = 100;
			FlashPeriod[1] = 400;
			FlashCycle = 8;
		break;
	}
	Adjust_RGBLED(FlashColor[0]);
	FlashStep = 0;
	count_led_step = FlashPeriod[0];
}


void Init_ADC1x(void)
{
	LL_ADC_DeInit(ADC1x);

	LL_DMA_ConfigAddresses(DMA1, LL_DMA_CHANNEL_1,
												 LL_ADC_DMA_GetRegAddr(ADC1x, LL_ADC_DMA_REG_REGULAR_DATA),
												 (uint32_t)&ADC_INPUT_Buffer,
												 LL_DMA_DIRECTION_PERIPH_TO_MEMORY);

	LL_DMA_ConfigTransfer(DMA1, LL_DMA_CHANNEL_1, LL_DMA_DIRECTION_PERIPH_TO_MEMORY
																							| LL_DMA_MODE_CIRCULAR
																							| LL_DMA_PERIPH_NOINCREMENT
																							| LL_DMA_MEMORY_INCREMENT
																							| LL_DMA_PDATAALIGN_WORD
																							| LL_DMA_MDATAALIGN_WORD
																							| LL_DMA_PRIORITY_LOW);

	LL_ADC_REG_SetSequencerRanks(ADC1x, LL_ADC_REG_RANK_1, LL_ADC_CHANNEL_10);

	LL_ADC_SetChannelSamplingTime(ADC1x, LL_ADC_CHANNEL_10, LL_ADC_SAMPLINGTIME_239CYCLES_5);

	LL_ADC_REG_SetTriggerSource(ADC1x, LL_ADC_REG_TRIG_SOFTWARE);
	LL_ADC_REG_SetDMATransfer(ADC1x, LL_ADC_REG_DMA_TRANSFER_UNLIMITED);
	LL_ADC_SetCommonPathInternalCh(ADC12_COMMON, LL_ADC_PATH_INTERNAL_VREFINT);

	LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_1, 1);
	LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_1);
	LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_1);

	LL_ADC_EnableIT_EOS(ADC1x);
	LL_ADC_EnableIT_AWD1(ADC1x);

	LL_ADC_Enable(ADC1x);
	wait_1ms(1);

	LL_ADC_StartCalibration(ADC1x);
	while (LL_ADC_IsCalibrationOnGoing(ADC1x));

	ADC_FLAG = 0;
	LL_ADC_REG_StartConversionSWStart(ADC1x);
}


void ADC1_2_IRQHandler(void)
{
	LL_ADC_ClearFlag_EOS(ADC1x);
	LL_ADC_ClearFlag_AWD1(ADC1x);
}


void DMA1_Channel1_IRQHandler(void)
{
	LL_DMA_ClearFlag_GI1(DMA1);
	ADC_FLAG = 1;
	last_adc_read = 5;
}


void renew_main_voltage(void)
{	uint8_t mm;
	uint32_t TempADC = 0;

	ADC_FLAG=0;
	last_adc_read=2;

	ADC_value[adc_ptr] = ADC_INPUT_Buffer >> 4;
	adc_ptr++;
	if (adc_ptr>31) adc_ptr = 0;

	for (mm=0; mm<32; mm++)
		TempADC += ADC_value[mm];

	mPower_status = (uint8_t)(TempADC > 0x1D00);
}


void check_adc_input_data(void)
{
	if (last_adc_read==0)
	{
		LL_ADC_REG_StartConversionSWStart(ADC1);
		last_adc_read = STOP_COUNTING;
	}

	if (ADC_FLAG==1)
	{
		renew_main_voltage();
		if (mPower_status == old_mPower_status)
		{
			count_mPower = STOP_COUNTING;
			return;
		}
		else
		{
			if (count_mPower>0) return;
			if (count_mPower<0)
			{
				count_mPower = 25;
				return;
			}
			old_mPower_status = mPower_status;
		}
	}
}


char rbuf[256] = {'\0',};
char nbuf[256] = {'\0',};


FRESULT Explore_Disk(char *path, uint8_t recu_level)
{
	FRESULT res = FR_OK;
	FILINFO fno;
	DIR dir;
	char *fn;
	static char lfn[_MAX_LFN + 1];			// Buffer to store the LFN
	FIL fp;
	UINT numread;
	uint16_t Num2Read;
	uint16_t NumRemained;
	uint32_t w_addr;

	fno.lfname = lfn;
	fno.lfsize = sizeof lfn;

	res = f_opendir(&dir, path);
	if (res == FR_OK)
	{
		while (USBH_MSC_IsReady(&hUSBHost))
		{
			res = f_readdir(&dir, &fno);
			if ((res != FR_OK) || (fno.fname[0] == 0))
			{
				break;
			}
			if (fno.fname[0] == '.')
			{
				continue;
			}

			fn = *fno.lfname ? fno.lfname : fno.fname;

			if (strncmp(fn, PREFIX_FIRMWARE_UPGRADE, 8)==0)									// "kali_sm_"
			{
				NumRemained = fno.fsize;
				ClearMemorySpaceFor(NumRemained);
				f_open(&fp,	fno.lfname,	FA_READ);

				w_addr = 0x040000;
				while (NumRemained>0)
				{
					if (NumRemained>256)
						Num2Read = 256;
					else
						Num2Read = NumRemained;
					NumRemained -= Num2Read;

					f_read(&fp, rbuf,	Num2Read, &numread);								// testfile size 53328 Bytes
					spi3_flash_write((uint8_t *)&rbuf, w_addr, Num2Read);
					w_addr += 256;
				}
				f_close(&fp);
			}
		}
		f_closedir(&dir);

		spi3_flash_read((uint8_t *)&nbuf, w_addr-256, 256);
	}
	return res;
}


void wait_300ms_power_stable(void)
{
	por_ms = 2000;

	while (1)
	{
		check_adc_input_data();

// USB Host Background task
		USBH_Process(&hUSBHost);
		if (Appli_state == APPLICATION_READY)
		{
			Explore_Disk("0:/", 1);
			break;
		}
		if (por_ms>0) continue;

		if (old_mPower_status == 1) break;
	}

	por_ms = STOP_COUNTING;
}


void check_power_status(void)
{
	if (old_mPower_status==1) return;

	Switch_LED(LED_AMBER);
	GPIOCx->ODR &= ~GPIO_PIN_10;

	while (1);
}


void SIGMA_MODIFY_EQ(uint32_t *pp)
{	uint32_t num = 5;
//	uint32_t addr = NO*8+BAND+MOD_EQ_CONTROL_DIPSW_EQ00_ALG0_EQS300MULTIDPHWSLEWP1ALG2TARGB210_ADDR;
//	uint32_t *pp=(uint32_t *)&UserBQ[NO][BAND];

//#define MOD_SAFELOADMODULE_DATA_SAFELOAD0_ADDR         24576
//#define MOD_SAFELOADMODULE_DATA_SAFELOAD1_ADDR         24577
//#define MOD_SAFELOADMODULE_DATA_SAFELOAD2_ADDR         24578
//#define MOD_SAFELOADMODULE_DATA_SAFELOAD3_ADDR         24579
//#define MOD_SAFELOADMODULE_DATA_SAFELOAD4_ADDR         24580
//#define MOD_SAFELOADMODULE_ADDRESS_SAFELOAD_ADDR       24581
//#define MOD_SAFELOADMODULE_NUM_SAFELOAD_ADDR           24582
	if (DspEqOffset==0) return;

	SIGMA_WRITE_REGISTER_UINT32(DEVICE_ADDR_IC_1, MOD_SAFELOADMODULE_DATA_SAFELOAD0_ADDR, 4, (uint8_t *)pp++);		// UserBQ[NO][BAND].b2
	SIGMA_WRITE_REGISTER_UINT32(DEVICE_ADDR_IC_1, MOD_SAFELOADMODULE_DATA_SAFELOAD1_ADDR, 4, (uint8_t *)pp++);		// UserBQ[NO][BAND].b1
	SIGMA_WRITE_REGISTER_UINT32(DEVICE_ADDR_IC_1, MOD_SAFELOADMODULE_DATA_SAFELOAD2_ADDR, 4, (uint8_t *)pp++);		// UserBQ[NO][BAND].b0
	SIGMA_WRITE_REGISTER_UINT32(DEVICE_ADDR_IC_1, MOD_SAFELOADMODULE_DATA_SAFELOAD3_ADDR, 4, (uint8_t *)pp++);		// UserBQ[NO][BAND].a2
	SIGMA_WRITE_REGISTER_UINT32(DEVICE_ADDR_IC_1, MOD_SAFELOADMODULE_DATA_SAFELOAD4_ADDR, 4, (uint8_t *)pp++);		// UserBQ[NO][BAND].a1
	SIGMA_WRITE_REGISTER_UINT32(DEVICE_ADDR_IC_1, MOD_SAFELOADMODULE_ADDRESS_SAFELOAD_ADDR, 4, (uint8_t *)&DspEqOffset);
	SIGMA_WRITE_REGISTER_UINT32(DEVICE_ADDR_IC_1, MOD_SAFELOADMODULE_NUM_SAFELOAD_ADDR, 4, (uint8_t *)&num);
}


void SIGMA_SAFELOAD_4BYTES(uint32_t Addr, uint32_t pp)
{	uint32_t num = 1;
//	uint32_t addr = NO*8+BAND+MOD_EQ_CONTROL_DIPSW_EQ00_ALG0_EQS300MULTIDPHWSLEWP1ALG2TARGB210_ADDR;
//	uint32_t *pp=(uint32_t *)&UserBQ[NO][BAND];

//#define MOD_SAFELOADMODULE_DATA_SAFELOAD0_ADDR         24576
//#define MOD_SAFELOADMODULE_DATA_SAFELOAD1_ADDR         24577
//#define MOD_SAFELOADMODULE_DATA_SAFELOAD2_ADDR         24578
//#define MOD_SAFELOADMODULE_DATA_SAFELOAD3_ADDR         24579
//#define MOD_SAFELOADMODULE_DATA_SAFELOAD4_ADDR         24580
//#define MOD_SAFELOADMODULE_ADDRESS_SAFELOAD_ADDR       24581
//#define MOD_SAFELOADMODULE_NUM_SAFELOAD_ADDR           24582
	SIGMA_WRITE_REGISTER_UINT32(DEVICE_ADDR_IC_1, MOD_SAFELOADMODULE_DATA_SAFELOAD0_ADDR, 4, (uint8_t *)&pp);
	SIGMA_WRITE_REGISTER_UINT32(DEVICE_ADDR_IC_1, MOD_SAFELOADMODULE_ADDRESS_SAFELOAD_ADDR, 4, (uint8_t *)&Addr);
	SIGMA_WRITE_REGISTER_UINT32(DEVICE_ADDR_IC_1, MOD_SAFELOADMODULE_NUM_SAFELOAD_ADDR, 4, (uint8_t *)&num);
}
*/


void Select_Test_Volume(uint16_t TmpVol)
{//	uint32_t TmpIdx;

	if (TmpVol == OldtVolSW)
	{
		count_TestVol = STOP_COUNTING;
		return;
	}
	if (count_TestVol>0) return;
	if (count_TestVol<0)
	{
		count_TestVol = 20;
		return;
	}
	OldtVolSW = TmpVol;

	CurrentVolStep = dip_volndx[OldtVolSW];
	switch_dsp_volume(0);
//	TmpIdx = (15 - OldtVolSW);
//	SIGMA_WRITE_REGISTER_UINT32(DEVICE_ADDR_IC_1, MOD_EQ_CONTROL_DIPSW0_4_MONOMUXSIGMA300NS2INDEX_ADDR, 4, (uint8_t *)&TmpIdx);
}


// adau1452 Cell name: EQ Control.TM0_1
// EQ Control.TM0_1 = 0: Select signals from boundary EQ
// EQ Control.TM0_1 = 1: Select signals bypass boundary EQ
//
// It needs bypass Boundary EQ while entering TestMode 5
//
void Set_Boundary_OnOff(uint32_t OnOff)
{
	SIGMA_WRITE_REGISTER_UINT32(DEVICE_ADDR_IC_1, MOD_EQ_CONTROL_TM0_1_MONOMUXSIGMA300NS5INDEX_ADDR, 4, (uint8_t *)&OnOff);
}


void Test_EQ_OnOff(uint32_t TmpOnOff)
{
	if (TmpOnOff == OldOnOffSW)
	{
		count_OnOff = STOP_COUNTING;
		return;
	}
	if (count_OnOff>0) return;
	if (count_OnOff<0)
	{
		count_OnOff = 20;
		return;
	}
	OldOnOffSW = TmpOnOff;

	TmpOnOff = TmpOnOff*4;
	SIGMA_WRITE_REGISTER_UINT32(DEVICE_ADDR_IC_1, MOD_CROSSOVER_TM0_2_STEREOMUXSIGMA300NS41INDEX_ADDR, 4, (uint8_t *)&TmpOnOff);
}


void Select_New_Boundary(uint16_t TmpEQ)
{	uint32_t TmpIdx;

	if (TmpEQ == OldEQSW)
	{
		count_EQ = STOP_COUNTING;
		return;
	}
	if (count_EQ>0) return;
	if (count_EQ<0)
	{
		count_EQ = 20;
		return;
	}
	OldEQSW = TmpEQ;

	TmpIdx = (15 - OldEQSW);
	SIGMA_WRITE_REGISTER_UINT32(DEVICE_ADDR_IC_1, MOD_EQ_CONTROL_DIPSW0_4_MONOMUXSIGMA300NS2INDEX_ADDR, 4, (uint8_t *)&TmpIdx);
}


void check_hf_switch(uint16_t TmpHF)						// DIPSW 7, 8 (PE6/7)
{	uint32_t TmpIdx;

	if (!LocalCtrlEnabled) return;								// Controls are overridden by the HOST
	if (OldTestMode != NON_TEST_MODE) return;

	if (TmpHF == OldHFSW)
	{
		count_HF = STOP_COUNTING;
		return;
	}
	if (count_HF>0) return;
	if (count_HF<0)
	{
		count_HF = 20;
		return;
	}
	OldHFSW = TmpHF;
	TmpIdx = OldHFSW;
	SIGMA_WRITE_REGISTER_UINT32(DEVICE_ADDR_IC_1, MOD_CROSSOVER_DIPSW7_8_MONOMUXSIGMA300NS1INDEX_ADDR, 4, (uint8_t *)&TmpIdx);
}


void check_mf_switch(uint16_t TmpMF)						// DIPSW 5, 6 (PE4/5)
{	uint32_t TmpIdx;

	if (!LocalCtrlEnabled) return;								// Controls are overridden by the HOST
	if (OldTestMode != NON_TEST_MODE) return;

	if (TmpMF == OldMFSW)
	{
		count_MF = STOP_COUNTING;
		return;
	}
	if (count_MF>0) return;
	if (count_MF<0)
	{
		count_MF = 20;
		return;
	}
	OldMFSW = TmpMF;
	TmpIdx = OldMFSW;
	SIGMA_WRITE_REGISTER_UINT32(DEVICE_ADDR_IC_1, MOD_CROSSOVER_DIPSW5_6_MONOMUXSIGMA300NS4INDEX_ADDR, 4, (uint8_t *)&TmpIdx);
}


/*
void check_mf_switch_1(uint16_t TmpMF)						// DIPSW 5, 6 (PE4/5)
{	uint32_t TmpIdx;

	if (!LocalCtrlEnabled) return;								// Controls are overridden by the HOST
	if (OldTestMode != NON_TEST_MODE) return;

	if (TmpMF == OldMFSW)
	{
		count_MF = STOP_COUNTING;
		return;
	}
	if (count_MF>0) return;
	if (count_MF<0)
	{
		count_MF = 20;
		return;
	}
	OldMFSW = TmpMF;
	TmpIdx = OldMFSW*2;
	SIGMA_WRITE_REGISTER_UINT32(DEVICE_ADDR_IC_1, MOD_DCB_SW_STEREOMUXSIGMA300NS1INDEX_ADDR, 4, (uint8_t *)&TmpIdx);
}


void DspMasterVolume(int8_t vtblndx)
{
	SIGMA_WRITE_REGISTER_UINT32(DEVICE_ADDR_IC_1, MOD_MASTERVOLUME_ALG0_GAINALGNS145X2GAIN_ADDR, 4, (uint8_t *)&Volume_Table[vtblndx]);
	SIGMA_WRITE_REGISTER_UINT32(DEVICE_ADDR_IC_1, MOD_MASTERVOLUME_ALG0_GAINALGNS145X2GAIN_ADDR, 4, (uint8_t *)&Volume_Table[vtblndx]);
	SIGMA_WRITE_REGISTER_UINT32(DEVICE_ADDR_IC_1, MOD_MASTERVOLUME_ALG0_GAINALGNS145X2GAIN_ADDR, 4, (uint8_t *)&Volume_Table[vtblndx]);
}
*/


void switch_limiter(uint32_t TmpLIM)
{	uint32_t TmpIdx;

	if ((!LocalCtrlEnabled) ||								// Controls are overridden by the HOST
			(OldTestMode == NON_TEST_MODE) ||
			(TmpLIM == OldLIMSW))
	{
		count_LIM = STOP_COUNTING;
		return;
	}
	if (count_LIM>0) return;
	if (count_LIM<0)
	{
		count_LIM = 20;
		return;
	}
	OldLIMSW = TmpLIM;
	TmpIdx = OldLIMSW*4;
	SIGMA_WRITE_REGISTER_UINT32(DEVICE_ADDR_IC_1, MOD_LIMITERS_TM0_3_STEREOMUXSIGMA300NS42INDEX_ADDR, 4, (uint8_t *)&TmpIdx);
}



	uint32_t TmpVolValue;
	int32_t TmpVV;
	double kkp;

void switch_dsp_volume(int8_t dir)
{
	if (!LocalCtrlEnabled) return;								// Controls are overridden by the HOST
//	if ((OldTestMode != 0xFF) && (OldTestMode != NON_TEST_MODE)) return;
	if (OldTestMode == TESTMODE_6) return;

	if (((CurrentVolStep == TrimLevel_p6dB) && (dir==1)) ||
			((CurrentVolStep == TrimLevel_n12dB) && (dir==-1)))
	{
		__NOP();
	}
	else
	{
	if ((CurrentVolStep > TrimLevel_p6dB) || (CurrentVolStep < TrimLevel_n12dB))
	{
		__NOP();
	}
		CurrentVolStep += dir;
//		CurrentVolStep = 24;						// using OP
//		CurrentVolStep = 36;						// no OP
		DspMasterVolume(CurrentVolStep+CurrentDIM);
	}

	if (CurrentVolStep==TrimLevel_0dB)
	{
		Blink_EventLED(EVENT_VOL0DB);
	}
	else if ((CurrentVolStep == TrimLevel_p6dB) || (CurrentVolStep == TrimLevel_n12dB))
	{
		Blink_EventLED(EVENT_VOLMAXMIN);
	}
}


void Switch_PE0_7(void)
{	uint16_t TmpKey;

	if (!LocalCtrlEnabled) return;								// Controls are overridden by the HOST

	TmpKey = GPIOEx->IDR & 0x01FF;

	// DIPSW 1,2,3,4 (PE0/1/2/3)
	if (OldTestMode == TESTMODE_5)
	{
		Select_Test_Volume(TmpKey & 0x07);
		Test_EQ_OnOff((TmpKey >> 3) & 0x01);
		switch_limiter((TmpKey >> 4) & 0x01);
	}
	else if (OldTestMode == NON_TEST_MODE)
	{
		Select_New_Boundary(TmpKey & 0x0F);
	}
//	check_eq_switch_1((TmpKey & 0x0008) >> 3);


//	check_hf_switch((TmpKey & 0x00C0) >> 6);						// DIPSW 7, 8
//	check_mf_switch((TmpKey & 0x0030) >> 4);
//	check_mf_switch_1((TmpKey & 0x0010) >> 4);						// DIPSW 5
}


void Release_DSP_Control(void)
{
	if (!LocalCtrlEnabled) return;								// Controls are overridden by the HOST

	DeRegulate_ADAU1452_SPI();
}


void Recover_DSP_Control(void)
{
	if (!LocalCtrlEnabled) return;								// Controls are overridden by the HOST

	GPIOCx->ODR &= ~GPIO_PIN_10;
	ADAU1452_SPI_Init();
	default_download_IC_1();
	GPIOCx->ODR |= GPIO_PIN_10;
	count_EQ = STOP_COUNTING;
	OldEQSW = 0xFF;
//	switch_limiter();
}


void Switch_Testpin(void)
{	uint16_t TmpKey;

	TmpKey = (GPIOCx->IDR & 0xE000) >> 13;

	if (OldTestMode==TmpKey)
	{
		count_testmode = STOP_COUNTING;
		return;
	}
	if (count_testmode>0) return;
	if (count_testmode<0)
	{
		count_testmode = 20;
		return;
	}

	switch (TmpKey)
	{
		case TESTMODE_0:
		case TESTMODE_1:
		case TESTMODE_2:
		case TESTMODE_3:
		case TESTMODE_4:
		break;

// Test mode 5: test mode of dip switch
// DSP Controls
// No Boundary
		case TESTMODE_5:
			if ((OldTestMode==0xFF) || (OldTestMode==TESTMODE_6))
			{
				Recover_DSP_Control();
				switch_dsp_volume(0);
				Set_Boundary_OnOff(0);					// Disable all boundary EQ
			}
		break;

		case TESTMODE_6:									// Test mode 6: release control of DSP
			 Release_DSP_Control();
		break;

		case NON_TEST_MODE:
			if ((OldTestMode==0xFF) || (OldTestMode==TESTMODE_6))
			{
				Recover_DSP_Control();
//				Set_Boundary_OnOff(1);					// Enable all boundary EQ
				switch_dsp_volume(0);
			}
		break;
	}
	OldTestMode = TmpKey;
	
}


/*
void Examine_LED_Status(void)
{
	if (count_led_step != 0) return;

	if (count_led_step == 0)
	{
		if (FlashStep==1)
		{
			FlashCycle--;
			if (FlashCycle==0)
			{
				count_led_step = STOP_COUNTING;
				return;
			}
		}

		FlashStep = 1 - FlashStep;
		Switch_LED(FlashColor[FlashStep]);
		count_led_step = FlashPeriod[FlashStep];
	}
}
*/



