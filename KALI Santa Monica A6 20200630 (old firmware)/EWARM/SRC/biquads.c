/*--------------------------------------------------------------------------------------------------------*/
/*																																																				*/
/* Copyright(c) 2019 ~ 2020 Dongyuan Electronics Corp. All rights reserved.															  */
/*																																																				*/
/*--------------------------------------------------------------------------------------------------------*/

//**********************************************************************************************************
//
//	APPLICATION: SANTA MONICA SERIES
//	Project: kali_sm_a5_20200608
//	Source: biquads.c
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

#include "stm32f107xc.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "biquads.h"
#include "misc.h"


//double FreqSp = 96000;
#define FreqSp						(double)96000


extern uint16_t DspEqOffset;
extern uint16_t DspEqAddrStart[2][8];
extern EQ_STRUCT PresetEQ[8][8];


EQ_STRUCT UserEQ[8][8];
BQ_TYPE UserBQ[8][8];

EQ_STRUCT *ActiveEQ = NULL;
BQ_TYPE *ActiveBQ = NULL;
const BQ_TYPE FlatBQ = {0, 0, 0x01000000, 0, 0};				// b2, b1, b0, a1, a0
const EQ_STRUCT FlatEQ = {1, FIR_PEQ, 1000, 141, 0};


BQ_TYPE bqst;

double omega0;
double gainLinear;
double TmpK;
double Alpha;
double Beta;
double BigW;
double BigA;
double DE;

double b0_fl;
double b1_fl;
double b2_fl;
double a1_fl;
double a2_fl;
//double b0H;
//double b1H;
//double b2H;
//double a1H;
//double a2H;


void Flat_bq_coef(void)
{
	memmove((void *)ActiveBQ, (void *)&FlatBQ, sizeof(BQ_TYPE));
	__NOP();
}


void Flat_eq_param(void)
{
	memmove((void *)ActiveEQ, (void *)&FlatEQ, sizeof(EQ_STRUCT));
	__NOP();
}


uint8_t Point2EqCell(uint8_t EQ_NO, uint8_t BD_NO)
{
	if ((EQ_NO<8) || (EQ_NO>15)) return (0);
	if (BD_NO>7) return (0);

	ActiveEQ = &UserEQ[EQ_NO-8][BD_NO];
	ActiveBQ = &UserBQ[EQ_NO-8][BD_NO];
	DspEqOffset = DspEqAddrStart[1][EQ_NO-8]+BD_NO*5;
	return (1);
}


double validate_Qf_input(double Qf)
{	uint16_t RET_i;
	double RET_f;

	if (Qf<0.1)
	{
		RET_i = 10;
	}
	else if (Qf>50.0)
	{
		RET_i = 5000;
	}
	else
	{
		RET_i = (uint16_t)round(Qf*100);
	}
	RET_f = (double)RET_i/100;
	return (RET_f);
}


double validate_gain_input(double Gain)
{	int16_t RET_i;
	double RET_f;

	if (Gain<-24)
	{
		RET_i = -480;
	}
	else if (Gain>18.0)
	{
		RET_i = 360;
	}
	else
	{
		RET_i = (uint16_t)round(Gain*20);
	}
	RET_f = (double)RET_i/20;
	return (RET_f);
}


double validate_freq_input(double Fc)
{	uint16_t RET_i;
	double RET_f;

	if (Fc<100)
	{
		RET_i = 100;
	}
	else if (Fc>40000)
	{
		RET_i = 40000;
	}
	else
	{
		if (Fc>=10000)
		{
			RET_i = (uint16_t)round(Fc/100)*100;
		}
		else if (Fc>=1000)
		{
			RET_i = (uint16_t)round(Fc/10)*10;
		}
		else
		{
			RET_i = (uint16_t)Fc;
		}
	}
	RET_f = RET_i;
	return (RET_f);
}


void LowPass_I_ST(double Fc1)								// Freq, Q, gain, gainlinear
{
	if (ActiveEQ==NULL) return;

	TmpK = tan(Fc1*pi()/FreqSp);
	Alpha = 1 + TmpK;

	ActiveBQ->a2 = 0;
	ActiveBQ->b2 = 0;
	ActiveBQ->a1 = (int32_t)((1 - TmpK)/Alpha*ONE_8_24);
	ActiveBQ->b0 = (int32_t)(TmpK/Alpha*ONE_8_24);
	ActiveBQ->b1 = ActiveBQ->b0;

/*
	a2_fl = 0;
	b2_fl = 0;
	a1_fl = (-1)*(1 - TmpK)/Alpha;
	b0_fl = TmpK/Alpha;
	b1_fl = b0_fl;

	bqst.a2 = (int32_t)a2_fl;
	bqst.b2 = (int32_t)b2_fl;
	bqst.a1 = (int32_t)((-1)*a1_fl/2*ONE_0_24) & 0x00FFFFFF;
	bqst.b0 = (int32_t)(b0_fl/2*ONE_0_24) & 0x00FFFFFF;
	bqst.b1 = (int32_t)(b1_fl/2*ONE_0_24) & 0x00FFFFFF;
*/

	__NOP();
}


void LowPass_I_ADI(double Fc1)
{
	if (ActiveEQ==NULL) return;

	omega0 = 2*pi()*Fc1/FreqSp;
	Alpha = pow(2.7, (-1)*omega0)*ONE_8_24;

	ActiveBQ->a2 = 0;
	ActiveBQ->b2 = 0;
//	ActiveBQ->a1 = (int32_t)(pow(2.7, (-1)*omega0)*ONE_8_24);
	ActiveBQ->a1 = (int32_t)round(Alpha);
	ActiveBQ->b0 = (int32_t)round(ONE_8_24-Alpha);
	ActiveBQ->b1 = 0;

	a2_fl = 0;
	b2_fl = 0;
	a1_fl = pow(2.7, (-1)*omega0);
	b0_fl = 1-a1_fl;
	b1_fl = 0;

/*
	bqst.a2 = (int32_t)a2_fl;
	bqst.b2 = (int32_t)b2_fl;
	bqst.a1 = (int32_t)((-1)*a1_fl/2*ONE_0_24) & 0x00FFFFFF;
	bqst.b0 = (int32_t)(b0_fl/2*ONE_0_24) & 0x00FFFFFF;
	bqst.b1 = (int32_t)(b1_fl/2*ONE_0_24) & 0x00FFFFFF;
*/

	__NOP();
}


void HighPass_I_ST(double Fc1)
{
	if (ActiveEQ==NULL) return;

	TmpK = tan(Fc1*pi()/FreqSp);
	Alpha = 1 + TmpK;

	ActiveBQ->a2 = 0;
	ActiveBQ->b2 = 0;
	ActiveBQ->a1 = (int32_t)((1-TmpK)/Alpha*ONE_8_24);
	ActiveBQ->b0 = (int32_t)(ONE_8_24/Alpha);
	ActiveBQ->b1 = ActiveBQ->b0*(-1);

	a2_fl = 0;
	b2_fl = 0;
	a1_fl = (-1)*(1 - TmpK)/Alpha;
	b0_fl = 1/Alpha;
	b1_fl = (-1)*b0_fl;

/*
	bqst.a2 = (int32_t)a2H;
	bqst.b2 = (int32_t)b2H;
	bqst.a1 = (int32_t)((-1)*a1H/2*ONE_0_24) & 0x00FFFFFF;
	bqst.b0 = (int32_t)(b0H/2*ONE_0_24) & 0x00FFFFFF;
	bqst.b1 = (int32_t)(b1H/2*ONE_0_24) & 0x00FFFFFF;
*/

	__NOP();
}


void HighPass_I_ADI(double Fc1)
{
	if (ActiveEQ==NULL) return;

	omega0 = 2*Fc1*pi()/FreqSp;
	Alpha = pow(2.7, (-1)*omega0)*ONE_8_24;

	ActiveBQ->a2 = 0;
	ActiveBQ->b2 = 0;
	ActiveBQ->a1 = (int32_t)round(Alpha);
	ActiveBQ->b0 = (int32_t)round((ONE_8_24+Alpha)/2);
	ActiveBQ->b1 = ActiveBQ->b0*(-1);

	__NOP();
}


// ST document
void LowPass_II_ST(double Fc1, double Qf1)
{
	TmpK = tan(Fc1*pi()/FreqSp);
	Alpha = 1 + TmpK;
	BigW = TmpK * TmpK;
	DE = (TmpK / Qf1 + BigW + 1);

	ActiveBQ->a2 = (int32_t)((-1)*((1-TmpK/Qf1+BigW)/DE)*ONE_8_24);
	ActiveBQ->b2 = (int32_t)(BigW/DE*ONE_8_24);
	ActiveBQ->a1 = (int32_t)(2*(1-BigW)/DE*ONE_8_24);
	ActiveBQ->b0 = (int32_t)(BigW/DE*ONE_8_24);
	ActiveBQ->b1 = 2*ActiveBQ->b0;

/*
	a1_fl = 2 * ( BigW - 1) / DE;
	a2_fl = (1 - TmpK / Qf1 + BigW) / DE;
	b0_fl = BigW / DE;
	b1_fl = 2 * BigW / DE;
	b2_fl = BigW / DE;

	bqst.a2 = (int32_t)((-1)*a2_fl*ONE_0_24) & 0x00FFFFFF;
	bqst.b2 = (int32_t)(b2_fl*ONE_0_24) & 0x00FFFFFF;
	bqst.a1 = (int32_t)((-1)*a1_fl/2*ONE_0_24) & 0x00FFFFFF;
	bqst.b0 = (int32_t)(b0_fl/2*ONE_0_24) & 0x00FFFFFF;
	bqst.b1 = (int32_t)(b1_fl/2*ONE_0_24) & 0x00FFFFFF;
*/

	__NOP();
}


// ADI document - SStudio help
void LowPass_IIA(double Fc1, double Qf1)
{
	omega0 = Fc1 * pi() * 2/ FreqSp;
	Alpha = sin(omega0)/(2*Qf1);
	BigW = (double)ONE_8_24 / (1+Alpha);
	BigA = cos(omega0)*BigW;

// equations - SStudio Help
/*
	a2 = (1-Alpha)/(1+Alpha);
	b2 = (1-cos(omega0))/(1+Alpha)/2;
	a1 = -2*cos(omega0)/(1+Alpha);
	b0 = (1-cos(omega0))/(1+Alpha)/2;
	b1 = (1-cos(omega0))/(1+Alpha);
*/
	ActiveBQ->a2 = (int32_t)((Alpha-1)*BigW);
	ActiveBQ->b2 = (int32_t)((BigW-BigA)*0.5);
	ActiveBQ->a1 = (int32_t)(2*BigA);
	ActiveBQ->b0 = (int32_t)((BigW-BigA)*0.5);
	ActiveBQ->b1 = 2*ActiveBQ->b0;

/*
	a1_fl = (-2)*cos(omega0)/(1+Alpha);
	a2_fl = (1-Alpha)/(1+Alpha);
	b0_fl = (1-cos(omega0))/(2*(1+Alpha));
	b1_fl = 2*b0_fl;
	b2_fl = (1-cos(omega0))/(2*(1+Alpha));

	bqst.a2 = (int32_t)((-1)*a2_fl*ONE_0_24) & 0x00FFFFFF;
	bqst.b2 = (int32_t)(b2_fl*ONE_0_24) & 0x00FFFFFF;
	bqst.a1 = (int32_t)((-1)*a1_fl/2*ONE_0_24) & 0x00FFFFFF;
	bqst.b0 = (int32_t)(b0_fl/2*ONE_0_24) & 0x00FFFFFF;
	bqst.b1 = (int32_t)(b1_fl/2*ONE_0_24) & 0x00FFFFFF;
*/

	__NOP();
}


// ST document
void HighPass_II_ST(double Fc1, double Qf1)
{
	TmpK = tan(Fc1*pi()/FreqSp);
	Alpha = 1 + TmpK;
	BigW = TmpK * TmpK;
	DE = (TmpK / Qf1 + BigW + 1);

	ActiveBQ->a2 = (int32_t)((TmpK/Qf1-BigW-1)/DE*ONE_8_24);
	ActiveBQ->b2 = (int32_t)((double)ONE_8_24/DE);
	ActiveBQ->a1 = (int32_t)(2*(1-BigW)/DE*ONE_8_24);
	ActiveBQ->b0 = ActiveBQ->b2;
	ActiveBQ->b1 = (int32_t)((double)ONE_8_24*(-2)/DE);

/*
	a1_fl = 2 * ( BigW - 1) / DE;
	a2_fl = (1 - TmpK / Qf1 + BigW) / DE;
	b0_fl = 1 / DE;
	b1_fl = (-2) / DE;
	b2_fl = 1 / DE;

	bqst.a2 = (int32_t)((-1)*a2_fl*ONE_0_24) & 0x00FFFFFF;
	bqst.b2 = (int32_t)(b2_fl*ONE_0_24) & 0x00FFFFFF;
	bqst.a1 = (int32_t)((-1)*a1_fl/2*ONE_0_24) & 0x00FFFFFF;
	bqst.b0 = (int32_t)(b0_fl/2*ONE_0_24) & 0x00FFFFFF;
	bqst.b1 = (int32_t)(b1_fl/2*ONE_0_24) & 0x00FFFFFF;
*/

	__NOP();
}


// ADI document - SStudio help
void HighPass_II_ADI(double Fc1, double Qf1)
{
	omega0 = Fc1 * pi() * 2/ FreqSp;
	Alpha = sin(omega0)/(2*Qf1);
	BigW = (double)ONE_8_24 / (1+Alpha);
	BigA = cos(omega0)*BigW;

// equations - SStudio Help
/*
	a2 = (1-Alpha)/(1+Alpha);
	b2 = (1+cos(omega0))/(1+Alpha)/2;
	a1 = -2*cos(omega0)/(1+Alpha);
	b0 = (1+cos(omega0))/(1+Alpha)/2;
	b1 = -(1+cos(omega0))/(1+Alpha);
*/
	ActiveBQ->a2 = (int32_t)((Alpha-1)*BigW);
	ActiveBQ->b2 = (int32_t)((BigW+BigA)*0.5);
	ActiveBQ->a1 = (int32_t)(2*BigA);
	ActiveBQ->b0 = (int32_t)((BigW+BigA)*0.5);
	ActiveBQ->b1 = (-2)*ActiveBQ->b0;

// reference
/*
	a1_fl = (-2)*cos(omega0)/(1+Alpha);
	a2_fl = (1-Alpha)/(1+Alpha);
	b0_fl = (1+cos(omega0))/(2*(1+Alpha));
	b1_fl = (-2)*b0_fl;
	b2_fl = (1+cos(omega0))/(2*(1+Alpha));

	bqst.a2 = (int32_t)((-1)*a2_fl*ONE_0_24) & 0x00FFFFFF;
	bqst.b2 = (int32_t)(b2_fl*ONE_0_24) & 0x00FFFFFF;
	bqst.a1 = (int32_t)((-1)*a1_fl/2*ONE_0_24) & 0x00FFFFFF;
	bqst.b0 = (int32_t)(b0_fl/2*ONE_0_24) & 0x00FFFFFF;
	bqst.b1 = (int32_t)(b1_fl/2*ONE_0_24) & 0x00FFFFFF;
*/

	__NOP();
}


//double Gain2;

void LowShelf(double Fc1, double Slp, double Gain1)
{	double GpCOS;
	double GnCOS;

	BigA = pow(10, Gain1/40);
	omega0 = Fc1 * pi() * 2/ FreqSp;
	GpCOS = (BigA+1)*cos(omega0);
	GnCOS = (BigA-1)*cos(omega0);
	Alpha = sin(omega0)/2*sqrt((BigA+(1/BigA))*(1/Slp-1)+2);
	Beta = 2 * Alpha *sqrt(BigA);
	BigW = (double)ONE_8_24/((BigA+1)+GnCOS+Beta);

	ActiveBQ->a2 = (int32_t)((-1)*((BigA+1)+GnCOS-Beta)*BigW);
	ActiveBQ->b2 = (int32_t)(BigA*((BigA+1)-GnCOS-Beta)*BigW);
	ActiveBQ->a1 = (int32_t)(2*((BigA-1)+GpCOS)*BigW);
	ActiveBQ->b0 = (int32_t)(BigA*((BigA+1)-GnCOS+Beta)*BigW);
	ActiveBQ->b1 = (int32_t)(2*BigA*((BigA-1)-GpCOS)*BigW);

	TmpK = (BigA+1)+GnCOS+Beta;
	a1_fl = (-2) * ((BigA-1) + (BigA+1)*cos(omega0))/TmpK;
	a2_fl = ((BigA+1)+(BigA-1)*cos(omega0)-Beta)/TmpK;
	b0_fl = BigA*((BigA+1)-(BigA-1)*cos(omega0)+Beta)/TmpK;
	b1_fl = 2 * BigA*((BigA-1)-(BigA+1)*cos(omega0))/TmpK;
	b2_fl = BigA*((BigA+1)-(BigA-1)*cos(omega0)-Beta)/TmpK;

/*
	bqst.a2 = (int32_t)((-1)*a2_fl*ONE_0_24) & 0x00FFFFFF;
	bqst.b2 = (int32_t)(b2_fl*ONE_0_24) & 0x00FFFFFF;
	bqst.a1 = (int32_t)((-1)*a1_fl/2*ONE_0_24) & 0x00FFFFFF;
	bqst.b0 = (int32_t)(b0_fl/2*ONE_0_24) & 0x00FFFFFF;
	bqst.b1 = (int32_t)(b1_fl/2*ONE_0_24) & 0x00FFFFFF;
*/

	__NOP();
}


void HighShelf(double Fc1, double Slp, double Gain1)
{	double GpCOS;
	double GnCOS;

	BigA = pow(10, Gain1/40);
	omega0 = Fc1 * pi() * 2/ FreqSp;
	GpCOS = (BigA+1)*cos(omega0);
	GnCOS = (BigA-1)*cos(omega0);
	Alpha = sin(omega0)/2*sqrt((BigA+(1/BigA))*(1/Slp-1)+2);
	Beta = 2 * Alpha *sqrt(BigA);
	BigW = (double)ONE_8_24/((BigA+1)-GnCOS+Beta);

	ActiveBQ->a2 = (int32_t)((-1)*((BigA+1)-GnCOS-Beta)*BigW);
	ActiveBQ->b2 = (int32_t)(BigA*((BigA+1)+GnCOS-Beta)*BigW);
	ActiveBQ->a1 = (int32_t)((-2)*((BigA-1)-GpCOS)*BigW);
	ActiveBQ->b0 = (int32_t)(BigA*((BigA+1)+GnCOS+Beta)*BigW);
	ActiveBQ->b1 = (int32_t)((-2)*BigA*((BigA-1)+GpCOS)*BigW);

	TmpK = (BigA+1)-GnCOS+Beta;
	a1_fl = 2 * ((BigA-1) - (BigA+1)*cos(omega0))/TmpK;
	a2_fl = ((BigA+1)-(BigA-1)*cos(omega0)-Beta)/TmpK;
	b0_fl = BigA*((BigA+1)+(BigA-1)*cos(omega0)+Beta)/TmpK;
	b1_fl = (-2) * BigA*((BigA-1)+(BigA+1)*cos(omega0))/TmpK;
	b2_fl = BigA*((BigA+1)+(BigA-1)*cos(omega0)-Beta)/TmpK;

/*
	bqst.a2 = (int32_t)((-1)*a2_fl*ONE_0_24) & 0x00FFFFFF;
	bqst.b2 = (int32_t)(b2_fl*ONE_0_24) & 0x00FFFFFF;
	bqst.a1 = (int32_t)((-1)*a1_fl/2*ONE_0_24) & 0x00FFFFFF;
	bqst.b0 = (int32_t)(b0_fl/2*ONE_0_24) & 0x00FFFFFF;
	bqst.b1 = (int32_t)(b1_fl/2*ONE_0_24) & 0x00FFFFFF;
*/

	__NOP();
}


void PeakFilter(double Fc1, double Qf1, double Gain1, double gLine)
{
	if (round(Gain1)==0)
	{
		Flat_bq_coef();
		return;
	}

	omega0 = Fc1*pi()*2/FreqSp;
	gainLinear = pow(10, gLine/20);

	BigA = pow(10, Gain1/40);
	Alpha = sin(omega0)/(2*Qf1);

	TmpK = 1+Alpha/BigA;						// coefficient a0
	BigW = (double)ONE_8_24/TmpK;

	ActiveBQ->a2 = (int32_t)round((Alpha/BigA-1)*BigW);
	ActiveBQ->b2 = (int32_t)round((1-Alpha*BigA)*BigW);
	ActiveBQ->b1 = (int32_t)round((-2)*cos(omega0)*BigW);
	ActiveBQ->a1 = (-1)*ActiveBQ->b1;
	ActiveBQ->b0 = (int32_t)round((1+Alpha*BigA)*BigW);

/*
	b0_fl = (1+Alpha*BigA)/TmpK;
	b1_fl = (-2)*cos(omega0)/TmpK;
	b2_fl = (1-Alpha*BigA)/TmpK;
	a1_fl = b1_fl;
	a2_fl = (1-Alpha/BigA)/TmpK;

	bqst.a2 = (int32_t)((-1)*a2_fl*ONE_0_24) & 0x00FFFFFF;
	bqst.b2 = (int32_t)(b2_fl*ONE_0_24) & 0x00FFFFFF;
	bqst.a1 = (int32_t)((-1)*a1_fl/2*ONE_0_24) & 0x00FFFFFF;
	bqst.b0 = (int32_t)(b0_fl/2*ONE_0_24) & 0x00FFFFFF;
	bqst.b1 = (int32_t)(b1_fl/2*ONE_0_24) & 0x00FFFFFF;
*/

	__NOP();
}


void PeakFilter_ST(double Fc1, double Qf1, double Gain1, double gLine)
{
	omega0 = Fc1*pi()*2/FreqSp;
	gainLinear = pow(10, gLine/20);

	BigA = pow(10, Gain1/40);
	Alpha = sin(omega0)/(2*BigA*Qf1);

	TmpK = 1+Alpha/BigA;						// coefficient a0
	BigW = (double)ONE_8_24/TmpK;

	ActiveBQ->a2 = (int32_t)((Alpha/BigA-1)*BigW);
	ActiveBQ->b2 = (int32_t)((1-Alpha*BigA)*BigW);
	ActiveBQ->b1 = (int32_t)((-2)*cos(omega0)*BigW);
	ActiveBQ->a1 = (-1)*ActiveBQ->b1;
	ActiveBQ->b0 = (int32_t)((1+Alpha*BigA)*BigW);

/*
	b0_fl = (1+Alpha*BigA)/TmpK;
	b1_fl = (-2)*cos(omega0)/TmpK;
	b2_fl = (1-Alpha*BigA)/TmpK;
	a1_fl = b1_fl;
	a2_fl = (1-Alpha/BigA)/TmpK;

	bqst.a2 = (int32_t)((-1)*a2_fl*ONE_0_24) & 0x00FFFFFF;
	bqst.b2 = (int32_t)(b2_fl*ONE_0_24) & 0x00FFFFFF;
	bqst.a1 = (int32_t)((-1)*a1_fl/2*ONE_0_24) & 0x00FFFFFF;
	bqst.b0 = (int32_t)(b0_fl/2*ONE_0_24) & 0x00FFFFFF;
	bqst.b1 = (int32_t)(b1_fl/2*ONE_0_24) & 0x00FFFFFF;
*/

	__NOP();
}


void NotchFilter(double Fc1, double Qf1)
{
//	Gain2 = pow(10, Gain1/40);
	omega0 = Fc1*pi()*2/FreqSp;
	Alpha = sin(omega0)/(2*Qf1);
	TmpK = 1+Alpha;
	BigW = (double)ONE_8_24/TmpK;

	ActiveBQ->b0 = (int32_t)(BigW);
	ActiveBQ->b2 = ActiveBQ->b0;
	ActiveBQ->b1 = (int32_t)((-2)*cos(omega0)*BigW);
	ActiveBQ->a1 = (-1)*ActiveBQ->b1;
	ActiveBQ->a2 = (int32_t)((Alpha-1)*BigW);

	b0_fl = 1/TmpK;
	b2_fl = b0_fl;
	b1_fl = (-2)*cos(omega0)/TmpK;
	a1_fl = b1_fl;
	a2_fl = (1-Alpha)/TmpK;

/*
	bqst.a2 = (int32_t)((-1)*a2_fl*ONE_0_24) & 0x00FFFFFF;
	bqst.b2 = (int32_t)(b2_fl*ONE_0_24) & 0x00FFFFFF;
	bqst.a1 = (int32_t)((-1)*a1_fl/2*ONE_0_24) & 0x00FFFFFF;
	bqst.b0 = (int32_t)(b0_fl/2*ONE_0_24) & 0x00FFFFFF;
	bqst.b1 = (int32_t)(b1_fl/2*ONE_0_24) & 0x00FFFFFF;
*/

	__NOP();
}


void AllPass_II(double Fc1, double Qf1)
{
//	Gain2 = pow(10, Gain1/40);
	omega0 = Fc1*pi()*2/FreqSp;
	Alpha = sin(omega0)/(2*Qf1);
	TmpK = 1+Alpha;
	BigW = (double)ONE_8_24/TmpK;

	ActiveBQ->b1 = (int32_t)((-2)*cos(omega0)*BigW);
	ActiveBQ->a1 = (-1)*ActiveBQ->b1;
	ActiveBQ->b0 = (int32_t)((1-Alpha)*BigW);
	ActiveBQ->a2 = (-1)*ActiveBQ->b0;
	ActiveBQ->b2 = ONE_8_24;

	b0_fl = (1-Alpha)/TmpK;
	b2_fl = 1;
	b1_fl = (-2)*cos(omega0)/TmpK;
	a1_fl = b1_fl;
	a2_fl = b0_fl;

/*
	bqst.a2 = (int32_t)((-1)*a2_fl*ONE_0_24) & 0x00FFFFFF;
	bqst.b2 = (int32_t)(b2_fl*ONE_0_24) & 0x00FFFFFF;
	bqst.a1 = (int32_t)((-1)*a1_fl/2*ONE_0_24) & 0x00FFFFFF;
	bqst.b0 = (int32_t)(b0_fl/2*ONE_0_24) & 0x00FFFFFF;
	bqst.b1 = (int32_t)(b1_fl/2*ONE_0_24) & 0x00FFFFFF;
*/

	__NOP();
}


double Gain1;
double Fc1;
double Qf1;

void Calculate_biquads(uint8_t EQ_NO, uint8_t BD_NO, uint8_t EqType, double Fc, double Qf, double Gain)
{
	if (!Point2EqCell(EQ_NO, BD_NO)) return;

	Fc1 = validate_freq_input(Fc);
	Qf1 = validate_Qf_input(Qf);
	Gain1 = validate_gain_input(Gain);
//	Fc1 = Fc;
//	Qf1 = Qf;
//	Gain1 = Gain;
	
	ActiveEQ->Freq = (uint16_t)Fc1;
	ActiveEQ->Qfx100 = (uint16_t)(Qf1*100);
	ActiveEQ->Gain = (int16_t)(Gain1*100);
	ActiveEQ->Type = EqType;

	switch (EqType)
	{
		case FIR_LPI:
			LowPass_I_ST(Fc1);
		break;

		case FIR_HPI:
			HighPass_I_ST(Fc1);
		break;

		case FIR_LPII:
			LowPass_IIA(Fc1, Qf1);
			__NOP();
		break;

		case FIR_HPII:
			HighPass_II_ADI(Fc1, Qf1);
			__NOP();
		break;

		case FIR_PEQ:
			PeakFilter(Fc1, Qf1, Gain1, 0);				// Freq, Q, gain, gainlinear
		break;

		case FIR_LSHELF:
			LowShelf(Fc1, Qf1, Gain1);
		break;

		case FIR_HSHELF:
			HighShelf(Fc1, Qf1, Gain1);
		break;

		case FIR_NOTCH:
			NotchFilter(Fc1, Qf1);
		break;

		case FIR_AllPass_II:
			AllPass_II(Fc1, Qf1);
		break;

		case FIR_FLAT:
			Flat_bq_coef();
		break;

		case FIR_HPI_ADI:
			HighPass_I_ADI(Fc1);
		break;

		case FIR_LPI_ADI:
			LowPass_I_ADI(Fc1);
		break;

		case FIR_LPII_ST:
			LowPass_II_ST(Fc1, Qf1);
			__NOP();
		break;

		case FIR_HPII_ST:
			HighPass_II_ST(Fc1, Qf1);
			__NOP();
		break;

		case FIR_PEQ_ST:
			PeakFilter_ST(Fc1, Qf1, Gain1, 0);
		break;
	}
}


void Load_User_Biquads(void)
{	uint8_t ii;

	ActiveBQ = (BQ_TYPE *)&UserBQ;
	ActiveEQ = (EQ_STRUCT *)&UserEQ;
	
	for (ii=0;ii<64;ii++)
	{
		Flat_bq_coef();
		Flat_eq_param();
		ActiveBQ++;
		ActiveEQ++;
		__NOP();
/*
			UserEQ[ii][jj].En = 0;
			UserEQ[ii][jj].Type = 4;
			UserEQ[ii][jj].Freq = 1000;
			UserEQ[ii][jj].Qfx10 = 14;
			UserEQ[ii][jj].Gain = 0;

			UserBQ[ii][jj].b2 = 0;
			UserBQ[ii][jj].b1 = 0;
			UserBQ[ii][jj].b0 = 0x01000000;
			UserBQ[ii][jj].a2 = 0;
			UserBQ[ii][jj].a1 = 0;
*/
	}
}


