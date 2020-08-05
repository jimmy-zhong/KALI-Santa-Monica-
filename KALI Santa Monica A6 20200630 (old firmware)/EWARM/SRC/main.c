/*--------------------------------------------------------------------------------------------------------*/
/*																																																				*/
/* Copyright(c) 2019 ~ 2020 Dongyuan Electronics Corp. All rights reserved.															  */
/*																																																				*/
/*--------------------------------------------------------------------------------------------------------*/

//**********************************************************************************************************
//
//	APPLICATION: SANTA MONICA SERIES
//	Project: kali_sm_a5_20200608
//	Source: main.c 
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


#include "app.h"


int main(void)
{
	Power_Up_Initialize();
	Main_Loop_Proc();
}

