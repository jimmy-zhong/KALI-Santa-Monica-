/*--------------------------------------------------------------------------------------------------------*/
/*																																																				*/
/* Copyright(c) 2019 ~ 2020 Dongyuan Electronics Corp. All rights reserved.															  */
/*																																																				*/
/*--------------------------------------------------------------------------------------------------------*/

//**********************************************************************************************************
//
//	APPLICATION: SANTA MONICA SERIES
//	Project: kali_sm_a5_20200608
//	Source: tcp_proc.c
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


#include <stdlib.h>
#include <string.h>
#include "device.h"
#include "spix.h"
#include "ult.h"
#include "W5500.h"
#include "tcp_proc.h"
#include "misc.h"
#include "biquads.h"
#include "stm32f1xx_ll_crc.h"


extern uint8_t DHCP_allocated_ip[4];   							 // IP address from DHCP
extern uint8_t HOST_NAME[13]; 
extern int16_t count_local_status;
extern EQ_STRUCT UserEQ[8][8];
extern BQ_TYPE UserBQ[8][8];
extern EQ_STRUCT PresetEQ[8][8];
extern EQ_STRUCT *ActiveEQ;
extern BQ_TYPE *ActiveBQ;
extern uint16_t DspEqOffset;
extern uint16_t DspEqAddrStart[2][8];
extern uint8_t LocalCtrlEnabled;
extern uint8_t OldEQSW;


#define DATA_BUF_SIZE											2048
#define TRIM_LEVEL_DB(VALUE)							TrimLevel_0dB + (VALUE * 2)

uint8_t gDATABUF[2][DATA_BUF_SIZE];
volatile uint8_t CONTROLLER_FOUND = 0;

uint8_t Control_IP[4] = {0,0,0,0};
uint16_t CtrlRxPort = 0;
uint16_t CtrlTxPort = 0;
volatile uint16_t len = 0;

volatile uint16_t udp_localport = 38000;						// UDP LOCAL PORT

volatile uint16_t tcp_listenport = 38002;						// TCP LISTENING PORT
volatile uint16_t tcp_outputport = 56914;						// TCP OUTPUT PORT

volatile uint8_t tcp_state = 0;
volatile uint8_t tcplisten_opened = 0x00;

volatile uint8_t network_cmd = 0xFF;
volatile uint8_t KSM_cmd[32] = {0, };
volatile uint32_t cmd_idx = 0;
volatile int8_t local_status_step = -1;


const char EqTypeStr[7][5] = {
"LPF1\0",				// Low-pass 1st order
"LPF2\0",				// Low-pass 2nd order
"HPF1\0",				// High-pass 1st order
"HPF2\0",				// High-pass 2nd order
"PEQ\0",				// Peaking EQ
"LSH\0",				// Low-shelf
"HSH\0"};				// High-shelf

const char SpkMessageStr[3][17]={"LSM_AMPFAULT;\0", "LSM_LIMITERENG;\0", "LSM_PARAMCHANGE\0"};
const char OnOffStr[2][4] = {"OFF\0", "ON\0"};

const char KsmOnOffResponse[2][16] = {"KSM;ACK;OFF;11\0", "KSM;ACK;ON;205\0"};
const char KsmAckStr1[] = "KSM;ACK;\0";
const char KsmAck245[] = "KSM;ACK;245\0";												// "KSM;ACK;245"
const char E8001_Checksum[] = "KSM;NACK;32769;137\0";						// Error code (0x8001): checksum error
const char E8002_Unknown[] = "KSM;NACK;32770;129\0";						// Error code (0x8002): unknown message
const char E8003_Malform[] = "KSM;NACK;32771;130\0";						// Error code (0x8003): malformed message
const char E4001_InvalidData[] = "KSM;NACK;16385;133\0";				// Error code (0x4001): Invalid data
const char E4002_OutOfRange[] = "KSM;NACK;16386;134\0";					// Error code (0x4002): Out of range
const char E4003_DataToolong[] = "KSM;NACK;16387;135\0";				// Error code (0x4003): Data too long
const char E4004_Crc32Error[] = "KSM;NACK;16388;136\0";					// Error code (0x4004): CRC32 Error
const char KsmAckFirmwareInfo[] = "KSM;ACK;57X,0.5;179\0";			// Firmware version: A5 (A is 0)

volatile SYSTEM_CONFIG_STRUCT SystemConfig;
volatile uint8_t LocStatus = 0xFF;


void tcp_init(void)
{
	WIZ_SPI_Init();
	set_network();

	SystemConfig.CTRL_SpkControls = 1;								// Controls Enabled
	SystemConfig.CTRL_mute = 0;												// Unmute
	SystemConfig.CTRL_led_en = 1;											// Led enabled
	SystemConfig.CTRL_trimlevel = TrimLevel_0dB;			// 0dB
	SystemConfig.CTRL_dim = 0;												// Undim
	SystemConfig.CTRL_delay = 0;											// 0ms
	SystemConfig.CTRL_stdby_delay = 20;								// 20 minutes
	SystemConfig.CTRL_dipsw_en = 1;										// dip override = false

/*
	if (SystemConfig.CTRL_dipsw_en==1)
	{		
		SystemConfig.CTRL_preset = (15 - OldEQSW);
	}
	else
	{
*/

		SystemConfig.CTRL_preset = 4;
//	}
}


int8_t Validate_OnOff_Cmmd(char *pps)
{
	if (strcmp(pps, "ON")==0)
	{
		return(1);
	}
	else if (strcmp(pps, "OFF")==0)
	{
		return(0);
	}
	else
	{
		return(-1);
	}
}


uint8_t calculate_checksum(uint16_t num)
{	uint8_t *pp = gDATABUF[SOCK_TCPC];
	uint16_t ii;
	uint32_t cksum = 0;

	for (ii=0;ii<num;ii++)
	{
		cksum += *pp++;
	}
	cksum %= 256;

	return (cksum);
}


// Controller's broadcast string ---
// KSM;CONTAVAIL;<ip>,<port>;<csum>
//
void Validate_Controller(void)
{	char *ppc;
	char *qqip;
	char *qqsock;

// Abstract Header "KSM"
	ppc = strtok((char *)gDATABUF[SOCK_TCPC], ";");
	if (strcmp(ppc, "KSM")!=0)
	{
		return;
	}

// Abstract broadcast message "CONTAVAIL"
	ppc = strtok(NULL, ";");
	if (strcmp(ppc, "CONTAVAIL")!=0)
	{
		return;
	}

// Abstract Controller's IP, socket
	ppc = strtok(NULL, ";");
	if (ppc==NULL)
	{
		return;
	}

	qqip = strtok(ppc, ",");
	qqsock = strtok(NULL, ",");
	CtrlRxPort = ATOI(qqsock, 10);

	ppc = qqip;
	qqip = strtok(ppc, ".");
	Control_IP[0] = ATOI(qqip, 10);
	qqip = strtok(NULL, ".");
	Control_IP[1] = ATOI(qqip, 10);
	qqip = strtok(NULL, ".");
	Control_IP[2] = ATOI(qqip, 10);
	qqip = strtok(NULL, ".");
	Control_IP[3] = ATOI(qqip, 10);
	CONTROLLER_FOUND = 1;
}


uint16_t num = 0;

void tcp_send_speaker_available(void)
{	uint8_t *ppc = gDATABUF[SOCK_TCPC];

	strcpy((char *)ppc, "KSM;SPKAVAIL;");
	ppc += 13;
	sprintf((char *)ppc, "%d.%d.%d.%d,%d", DHCP_allocated_ip[0], DHCP_allocated_ip[1],
																				 DHCP_allocated_ip[2], DHCP_allocated_ip[3],
																				 tcp_listenport);
	num = strlen((char *)ppc);
	ppc += num;
	num += 13;
	
	sprintf((char *)ppc, ";%d", calculate_checksum(num));
	send(SOCK_TCPC, gDATABUF[SOCK_TCPC], strlen((char *)gDATABUF[SOCK_TCPC]));

	tcp_state = 1;
}


void tcp_send_speaker_name(void)
{	uint8_t *ppc = gDATABUF[SOCK_TCPC];

	strcpy((char *)ppc, KsmAckStr1);														// "KSM;ACK;"
	strcat((char *)ppc, (char *)HOST_NAME);
	num = strlen((char *)ppc);
	ppc += num;

	sprintf((char *)ppc, ";%d", calculate_checksum(num));
	send(SOCK_TCPC, gDATABUF[SOCK_TCPC], strlen((char *)gDATABUF[SOCK_TCPC]));
}


void tcp_send_preset_select(void)
{	uint8_t *ppc = gDATABUF[SOCK_TCPC];

	strcpy((char *)ppc, KsmAckStr1);														// "KSM;ACK;"
	ppc += strlen((char *)KsmAckStr1);
	if (SystemConfig.CTRL_preset>7)
	{
		*ppc++ = 'U';
		*ppc++ = ',';
		*ppc++ = SystemConfig.CTRL_preset-8+0x31;
	}
	else
	{
		*ppc++ = 'F';
		*ppc++ = ',';
		*ppc++ = SystemConfig.CTRL_preset+0x31;
	}
	*ppc = '\0';
	num = strlen((char *)gDATABUF[SOCK_TCPC]);

	sprintf((char *)ppc, ";%d", calculate_checksum(num));
	send(SOCK_TCPC, gDATABUF[SOCK_TCPC], strlen((char *)gDATABUF[SOCK_TCPC]));
}


void tcp_send_standby_delay(void)
{	uint8_t *ppc = gDATABUF[SOCK_TCPC];

	strcpy((char *)ppc, KsmAckStr1);														// "KSM;ACK;"
	ppc += strlen((char *)KsmAckStr1);
	sprintf((char *)ppc, "%d", SystemConfig.CTRL_stdby_delay);
	num = strlen((char *)ppc);
	ppc += num;
	num = strlen((char *)gDATABUF[SOCK_TCPC]);

	sprintf((char *)ppc, ";%d", calculate_checksum(num));
	send(SOCK_TCPC, gDATABUF[SOCK_TCPC], strlen((char *)gDATABUF[SOCK_TCPC]));
}


void tcp_send_speaker_delay(void)
{	uint8_t *ppc = gDATABUF[SOCK_TCPC];

	strcpy((char *)ppc, KsmAckStr1);														// "KSM;ACK;"
	ppc += strlen((char *)KsmAckStr1);
	sprintf((char *)ppc, "%d", SystemConfig.CTRL_delay);
	num = strlen((char *)ppc);
	ppc += num;
	num = strlen((char *)gDATABUF[SOCK_TCPC]);

	sprintf((char *)ppc, ";%d", calculate_checksum(num));
	send(SOCK_TCPC, gDATABUF[SOCK_TCPC], strlen((char *)gDATABUF[SOCK_TCPC]));
}


void tcp_send_trim_level(void)
{	uint8_t *ppc = gDATABUF[SOCK_TCPC];

	strcpy((char *)ppc, KsmAckStr1);														// "KSM;ACK;"
	ppc += strlen((char *)KsmAckStr1);
	sprintf((char *)ppc, "%.1f", (double)SystemConfig.CTRL_trimlevel/10);
	num = strlen((char *)ppc);
	ppc += num;
	num = strlen((char *)gDATABUF[SOCK_TCPC]);

	sprintf((char *)ppc, ";%d", calculate_checksum(num));
	send(SOCK_TCPC, gDATABUF[SOCK_TCPC], strlen((char *)gDATABUF[SOCK_TCPC]));
}


EQ_STRUCT *Locate_UserEQ(char Pre, char *pps)
{	int8_t NN=-1;
	int8_t BB=-1;

	if (*(pps+1)==',')
	{
		if ((*pps>='1') && (*pps<='8'))
		{
			NN = *pps - 0x31;
			if ((*(pps+2)>='1') && (*(pps+2)<='8'))
			{
				BB = *(pps+2) - 0x31;
			}
		}
	}
	if ((NN>=0) && (BB>=0))
	{
		if (Pre=='U')
		{
			ActiveEQ = &UserEQ[NN][BB];
			ActiveBQ = &UserBQ[NN][BB];
			DspEqOffset = DspEqAddrStart[1][NN]+BB*5;
		}
		else if (Pre=='F')
		{
			ActiveEQ = &PresetEQ[NN][BB];
			ActiveBQ = NULL;
			DspEqOffset = 0;
		}
		else
		{
			ActiveEQ = NULL;
			ActiveBQ = NULL;
			DspEqOffset = 0;
		}
	}
	else
	{
		ActiveEQ = NULL;
		ActiveBQ = NULL;
		DspEqOffset = 0;
	}
	return (ActiveEQ);
}


void tcp_send_eqen_status(char *pps)
{	char *ppc = (char *)gDATABUF[SOCK_TCPC];

	if (Locate_UserEQ('U', pps)!=0)
	{
		strcpy((char *)ppc, KsmOnOffResponse[ActiveEQ->En]);
	}
	else
	{	
		strcpy((char *)ppc, E4002_OutOfRange);										// Error code (0x4002): Out of range
	}
	send(SOCK_TCPC, gDATABUF[SOCK_TCPC], strlen((char *)gDATABUF[SOCK_TCPC]));
}


void tcp_send_speaker_eqparams(char *pps)
{	char *ppc = (char *)gDATABUF[SOCK_TCPC];

	if (((*pps=='U') || (*pps=='F')) && (*(pps+1)==','))
	{
		if (Locate_UserEQ(*pps, pps+2)!=0)
		{
			sprintf((char *)ppc, "%s%s,%d,%.2f,%.1f",
							KsmAckStr1, EqTypeStr[ActiveEQ->Type],
							ActiveEQ->Freq, (double)ActiveEQ->Gain/100,
							(double)ActiveEQ->Qfx100/100);

			num = strlen((char *)ppc);
			ppc += num;
			num = strlen((char *)gDATABUF[SOCK_TCPC]);

			sprintf((char *)ppc, ";%d", calculate_checksum(num));
		}
		else
		{
			strcpy((char *)ppc, E4002_OutOfRange);									// Error code (0x4002): Out of range
		}
	}
	else
	{
		strcpy((char *)ppc, E4002_OutOfRange);										// Error code (0x4002): Out of range
	}
	send(SOCK_TCPC, gDATABUF[SOCK_TCPC], strlen((char *)gDATABUF[SOCK_TCPC]));
}


void tcp_send_firmware_info(void)
{	uint8_t *ppc = gDATABUF[SOCK_TCPC];

	strcpy((char *)ppc, KsmAckFirmwareInfo);										// "KSM;ACK;0.5;195"
	send(SOCK_TCPC, gDATABUF[SOCK_TCPC], strlen((char *)gDATABUF[SOCK_TCPC]));
}


void tcp_change_speaker_name(char *pps)
{	char *ppc = (char *)gDATABUF[SOCK_TCPC];

	if (strlen(pps)<=12)
	{
		strcpy((char *)HOST_NAME, (char *)pps);
		strcpy((char *)ppc, KsmAck245);														// "KSM;ACK;245"
	}
	else
	{	
		strcpy((char *)ppc, E4003_DataToolong);										// Error code (0x4003): Data too long
	}
	send(SOCK_TCPC, gDATABUF[SOCK_TCPC], strlen((char *)gDATABUF[SOCK_TCPC]));
}


void tcp_change_standby_delay(char *pps)
{	char *ppc = (char *)gDATABUF[SOCK_TCPC];
	uint16_t TmpDly;

	TmpDly = ATOI(pps, 10);
	if ((TmpDly>=5) && (TmpDly<=120))
	{
		SystemConfig.CTRL_stdby_delay = TmpDly;
		strcpy((char *)ppc, KsmAck245);														// "KSM;ACK;245"
	}
	else
	{	
		strcpy((char *)ppc, E4002_OutOfRange);										// Error code (0x4002): Out of range
	}
	send(SOCK_TCPC, gDATABUF[SOCK_TCPC], strlen((char *)gDATABUF[SOCK_TCPC]));
	if (TmpDly==56)
	{
		__NOP();
	}
	if (TmpDly==57)
	{
		__NOP();
	}
}


void tcp_change_speaker_delay(char *pps)
{	char *ppc = (char *)gDATABUF[SOCK_TCPC];
	uint16_t TmpDly;

	TmpDly = ATOI(pps, 10);
	if (TmpDly<=12)
	{
		SystemConfig.CTRL_delay = TmpDly;
		strcpy((char *)ppc, KsmAck245);														// "KSM;ACK;245"
	}
	else
	{	
		strcpy((char *)ppc, E4002_OutOfRange);										// Error code (0x4002): Out of range
	}
	send(SOCK_TCPC, gDATABUF[SOCK_TCPC], strlen((char *)gDATABUF[SOCK_TCPC]));
}


void tcp_change_trim_level(char *pps, uint8_t OneStep)
{	char *ppc = (char *)gDATABUF[SOCK_TCPC];
	double TmpLevel;

	TmpLevel = atof(pps);
	if (OneStep==1)
	{
		if ((TmpLevel!=-0.5) && (TmpLevel!=0.5))
		{
			strcpy((char *)ppc, E4002_OutOfRange);									// Error code (0x4002): Out of range
		}
		else
		{
			SystemConfig.CTRL_trimlevel += (int16_t)(TmpLevel*10);
			if (SystemConfig.CTRL_trimlevel<CTRL_MINVOL)
			{
				SystemConfig.CTRL_trimlevel=CTRL_MINVOL;
			}
			else if (SystemConfig.CTRL_trimlevel>CTRL_MAXVOL)
			{
				SystemConfig.CTRL_trimlevel=CTRL_MAXVOL;
			}
			tcp_send_trim_level();
			return;
		}
	}
	else
	{
		if ((TmpLevel>=CTRL_MINVOL/10) && (TmpLevel<=CTRL_MAXVOL/10))
		{
			SystemConfig.CTRL_trimlevel = (int16_t)(TmpLevel*10);
			strcpy((char *)ppc, KsmAck245);														// "KSM;ACK;245"
		}
		else
		{	
			strcpy((char *)ppc, E4002_OutOfRange);										// Error code (0x4002): Out of range
		}
	}
	send(SOCK_TCPC, gDATABUF[SOCK_TCPC], strlen((char *)gDATABUF[SOCK_TCPC]));
}


uint8_t check_float_digits(char *pps)
{	uint16_t res = 0;
	uint8_t psign=0;

	while (1)
	{
		if (*pps=='\0') break;
		if (*pps=='+')
		{
			if (psign!=0)
			{
				res = 0;
				break;
			}
			else
			{
				psign |= 0x01;
			}
		}
		else if (*pps=='-')
		{
			if (psign!=0)
			{
				res = 0;
				break;
			}
			else
			{
				psign |= 0x02;
			}
		}
		else if (*pps=='.')
		{
			if ((psign & 4)!=0)
			{
				res = 0;
				break;
			}
			else
			{
				psign |= 0x04;
			}
		}
		else if ((*pps>='0') && (*pps<='9'))
		{
			psign |= 0x08;
			res = 1;
		}
		else
		{
			res = 0;
			break;
		}
		pps++;
	}
	return (res);
}


uint8_t Validate_EQ_Params(char *pps)
{	char *ppx;
	uint16_t ii;
	uint8_t TmpType;
	int32_t TmpFreq;
	int16_t TmpGain;
	uint16_t TmpQ;

	ppx = strtok(pps, ",");
	
	for (ii=0;ii<7;ii++)
	{
		if (strcmp(ppx, EqTypeStr[ii])==0)
			break;
	}
	if (ii>=7)
	{
		return (0);
	}
	else
	{
		TmpType = ii;
	}

	ppx = strtok(NULL, ",");
	if (check_float_digits(ppx)==1)
	{
		TmpFreq = (uint16_t)atof(ppx);
		if ((TmpFreq<10) || (TmpFreq>40000))
		{
			return (0);
		}
	}
	else
	{
		return (0);
	}

	ppx = strtok(NULL, ",");
	if (check_float_digits(ppx)==1)
	{
		TmpGain = (uint16_t)(atof(ppx)*100);
		if ((TmpGain<-2400) || (TmpGain>1800))
		{
			return (0);
		}
		if ((TmpGain % 5)!=0)
		{
			return (0);
		}
	}
	else
	{
		return (0);
	}

	ppx = strtok(NULL, ",");
	if (cmd_idx==351)
	{
		__NOP();
	}
	if (check_float_digits(ppx)==1)
	{
		TmpQ = (uint16_t)(atof(ppx)*100);
		if ((TmpQ<10) || (TmpQ>5000))
		{
			
			return (0);
		}
	}
	else
	{
		return (0);
	}

	ActiveEQ->Type = TmpType;
	ActiveEQ->Freq = TmpFreq;
	ActiveEQ->Gain = TmpGain;
	ActiveEQ->Qfx100 = TmpQ;
	return (1);
}


void tcp_change_speaker_eqparams(char *pps)
{	char *ppc = (char *)gDATABUF[SOCK_TCPC];

	if (Locate_UserEQ('U', pps)!=0)
	{
		if (Validate_EQ_Params(pps+4)==1)
		{
			strcpy((char *)ppc, KsmAck245);														// "KSM;ACK;245"
		}
		else
		{
			strcpy((char *)ppc, E4002_OutOfRange);										// Error code (0x4002): Out of range
		}
	}
	send(SOCK_TCPC, gDATABUF[SOCK_TCPC], strlen((char *)gDATABUF[SOCK_TCPC]));
	SIGMA_MODIFY_EQ((uint32_t *)ActiveBQ);
}


void tcp_change_eqen_status(char *pps)
{	char *ppc = (char *)gDATABUF[SOCK_TCPC];
	int8_t res = -1;

	if (cmd_idx==207)
	{
		__NOP();
	}
	if (Locate_UserEQ('U', pps)!=0)
	{
		pps += 4;
	// res=1:ON, res=0:OFF, res=-1:invalid data
		res = Validate_OnOff_Cmmd(pps);

		if ((res==0) || (res==1))
		{
			ActiveEQ->En = res;
			strcpy((char *)ppc, KsmAck245);														// "KSM;ACK;245"
		}
		else
		{
			strcpy((char *)ppc, E4001_InvalidData);										// Error code (0x4001): Invalid data
		}
	}
	else
	{	
		strcpy((char *)ppc, E4002_OutOfRange);										// Error code (0x4002): Out of range
	}
	send(SOCK_TCPC, gDATABUF[SOCK_TCPC], strlen((char *)gDATABUF[SOCK_TCPC]));
}


void tcp_change_preset_select(char *pps)
{	char *ppc = (char *)gDATABUF[SOCK_TCPC];
	uint16_t TmpPreset;

	if (*(pps+1)==',')
	{
		TmpPreset = *(pps+2);
		if ((TmpPreset>=0x31) && (TmpPreset<=0x38))
		{
			TmpPreset -= 0x31;
			if ((*pps == 'U') || (*pps == 'F'))
			{
				if (*pps == 'U')
					TmpPreset += 8;
				SystemConfig.CTRL_preset = TmpPreset;
				strcpy((char *)ppc, KsmAck245);													// "KSM;ACK;245"
			}
			else
			{
				strcpy((char *)ppc, E4002_OutOfRange);									// Error code (0x4002): Out of range
			}
		}
		else
		{
			strcpy((char *)ppc, E4002_OutOfRange);									// Error code (0x4002): Out of range
		}
	}
	else
	{
		strcpy((char *)ppc, E4001_InvalidData);										// Error code (0x4001): Invalid data
	}
	send(SOCK_TCPC, gDATABUF[SOCK_TCPC], strlen((char *)gDATABUF[SOCK_TCPC]));
}


uint8_t Cmd_OnOff_Get(uint8_t cmmd)
{
	switch (cmmd)
	{
		case CMD_SPKSTANDBYENABLE_GET:
			return (SystemConfig.CTRL_stdby_en);
		break;

		case CMD_SPKMUTE_GET:
			return (SystemConfig.CTRL_mute);
		break;

		case CMD_SPKDIM_GET:
			return (SystemConfig.CTRL_dim);
		break;

		case CMD_SPKLEDENABLE_GET:
			return (SystemConfig.CTRL_led_en);
		break;

		case CMD_SPKCONTROL_GET:
			return (SystemConfig.CTRL_SpkControls);
		break;

		case CMD_SPKDIPSWENABLE_GET:
			return (SystemConfig.CTRL_dipsw_en);
		break;

	}
	return (-1);
}


void Cmd_OnOff_Set(uint8_t cmmd, int8_t res)
{
	switch (cmmd)
	{
		case CMD_SPKSTANDBYENABLE_SET:
			SystemConfig.CTRL_stdby_en = res;
		break;

		case CMD_SPKMUTE_SET:
			SystemConfig.CTRL_mute = res;
		break;

		case CMD_SPKDIM_SET:
			SystemConfig.CTRL_dim = res;
		break;

		case CMD_SPKLEDENABLE_SET:
			SystemConfig.CTRL_led_en = res;
		break;

		case CMD_SPKCONTROL_SET:
			SystemConfig.CTRL_SpkControls = res;
		break;

		case CMD_SPKDIPSWENABLE_SET:
		SystemConfig.CTRL_dipsw_en = res;
		break;
	}
}


void tcp_send_onoff_status(uint8_t cmmd)
{	uint8_t *ppc = gDATABUF[SOCK_TCPC];

/*
	strcpy((char *)ppc, KsmAckStr1);														// "KSM;ACK;"
	ppc += strlen(KsmAckStr1);

	strcpy((char *)ppc, OnOffStr[Cmd_OnOff_Get(cmmd)]);
	
	num = strlen((char *)ppc);
	ppc += num;
	num += strlen(KsmAckStr1);

	sprintf((char *)ppc, ";%d", calculate_checksum(num));
	num = strlen((char *)gDATABUF[SOCK_TCPC]);
*/

	strcpy((char *)ppc, KsmOnOffResponse[Cmd_OnOff_Get(cmmd)]);
	send(SOCK_TCPC, gDATABUF[SOCK_TCPC], num);
}


void tcp_change_onoff_status(char *pps, uint8_t cmmd)
{	uint8_t *ppc = gDATABUF[SOCK_TCPC];
	int8_t res = -1;

// res=1:ON, res=0:OFF, res=-1:invalid data
	res = Validate_OnOff_Cmmd(pps);

	if ((res==0) || (res==1))
	{
		Cmd_OnOff_Set(cmmd, res);
		strcpy((char *)ppc, KsmAck245);														// "KSM;ACK;245"
	}
	else
	{
		strcpy((char *)ppc, E4001_InvalidData);										// Error code (0x4001): Invalid data
	}
	send(SOCK_TCPC, gDATABUF[SOCK_TCPC], strlen((char *)&gDATABUF[SOCK_TCPC]));
}


uint32_t dwPolynomial = 0x04c11db7;

uint32_t CRC_cal_proc(const uint32_t *ptr, int len)
{
	uint32_t xbit;
	uint32_t data;
	uint32_t CRCn = 0xFFFFFFFF;    // init

	while (len--)
	{
		xbit = 1 << 31;

		data = *ptr++;
		for (int bits = 0; bits < 32; bits++)
		{
			if (CRCn & 0x80000000)
			{
				CRCn <<= 1;
				CRCn ^= dwPolynomial;
			}
			else
			{
				CRCn <<= 1;
			}
			if (data & xbit)
			{
				CRCn ^= dwPolynomial;
			}

			xbit >>= 1;
		}
	}
	return CRCn;
}

	uint32_t CRC32;
	uint32_t CRC32A;
	uint32_t CRC32B;
	uint16_t blocknum;
	uint16_t crcsize;
	uint32_t errcnt = 0;

//	const uint8_t arr1[] = { 0x12, 0x34, 0x56, 0x78 };
	
void firmware_download_proc(char *pps, uint8_t cmd)
{	uint8_t *ppc = gDATABUF[SOCK_TCPC];
	uint32_t *di;
//	uint8_t *di;
//	uint8_t *di8;
//	uint32_t dix;
	uint16_t ii;
//	char *tmpstr;

	switch (cmd)
	{
		case CMD_SPKFWSTART:
			strcpy((char *)ppc, KsmAck245);														// "KSM;ACK;245"
		break;

		case CMD_FWDATA:
			LL_CRC_ResetCRCCalculationUnit(CRC);
			di = (uint32_t *)(pps + strlen(pps)+1);
			pps = strtok(pps, ",");
			blocknum = ATOI(pps, 10);
			pps = strtok(NULL, ",");
			crcsize = ATOI(pps, 10)/4;
			pps = strtok(NULL, ",");
			CRC32A = ATOI32(pps, 10);
			CRC32B = CRC_cal_proc(di, 256);
			for (ii=0;ii<crcsize;ii++)
			{
				LL_CRC_FeedData32(CRC, *di++);
			}

			CRC32 = LL_CRC_ReadData32(CRC);

			if (CRC32A!=CRC32)
			{
				strcpy((char *)ppc, E4004_Crc32Error);									// "KSM;NACK;16388;136" - Error code (0x4004): CRC32 Error
			}
			else
			{
				strcpy((char *)ppc, KsmAck245);													// "KSM;ACK;245"
			}
		break;

		case CMD_SPKFWCOMPLETE:
			strcpy((char *)ppc, KsmAck245);														// "KSM;ACK;245"
		break;
	}
	send(SOCK_TCPC, gDATABUF[SOCK_TCPC], strlen((char *)&gDATABUF[SOCK_TCPC]));
}


void sending_local_status(void)
{	uint8_t *ppc = gDATABUF[SOCK_TCPC];

	if (LocStatus > 2) return;
	if (count_local_status!=0) return;
	
	local_status_step++;

	strcpy((char *)ppc, "KSM;");														// "KSM;ACK;"
	strcat((char *)ppc, SpkMessageStr[LocStatus]);

	if (LocStatus != 2)
	{
		strcat((char *)ppc, OnOffStr[1-local_status_step]);
	}
	else
	{
		local_status_step++;
	}
	num = strlen((char *)ppc);
	ppc += num;

	sprintf((char *)ppc, ";%d", calculate_checksum(num));
	send(SOCK_TCPC, gDATABUF[SOCK_TCPC], strlen((char *)&gDATABUF[SOCK_TCPC]));

	if (local_status_step>=1)
	{
		local_status_step = -1;
		LocStatus = 0xFF;
		count_local_status = STOP_COUNTING;
	}
	else
	{
		count_local_status = 1500;
	}
}


void response_checksum_error(void)
{
	strcpy((char *)gDATABUF[SOCK_TCPC], E8001_Checksum);					// 0x8001: checksum error
	send(SOCK_TCPC, gDATABUF[SOCK_TCPC], strlen((char *)&gDATABUF[SOCK_TCPC]));
}


void response_malformat_error(void)
{
	strcpy((char *)gDATABUF[SOCK_TCPC], E8003_Malform);						// 0x8003: malformed message
	send(SOCK_TCPC, gDATABUF[SOCK_TCPC], strlen((char *)&gDATABUF[SOCK_TCPC]));
}


void tcp_response_unknown_message(void)
{
	strcpy((char *)gDATABUF[SOCK_TCPC], E8002_Unknown);						// Error code (0x8002): unknown message
	send(SOCK_TCPC, gDATABUF[SOCK_TCPC], strlen((char *)&gDATABUF[SOCK_TCPC]));
}


	uint16_t iid;
	uint16_t iie;
	uint32_t tsum = 0;
	uint32_t isum = 0;
	char *ppo;
	char *ppf;

uint8_t examine_checksum(void)
{
	if (len>256) return (1);
	tsum = 0;
	isum = 0;
	ppf = (char *)gDATABUF[SOCK_TCPS];
	ppo = ppf + len - 1;
	
	if (cmd_idx>35)
	{
		__NOP();
	}
	if ((*ppo==';') || (*ppo<'0') || (*ppo>'9'))
	{
		response_malformat_error();
		return (0);
	}
	while (1)
	{
		if (*(--ppo)==';') break;
		if ((*ppo<'0') || (*ppo>'9'))
		{
			response_malformat_error();
			return (0);
		}
	}
	
	while (1)
	{
		tsum += *ppf++;
		if (ppf==ppo) break;
	}
	tsum %= 256;
	
	isum = ATOI(ppo+1, 10);
	if (isum==tsum)
	{
		return(1);
	}
	else
	{
		response_checksum_error();
		return (0);
	}
}


void Locate_This_Speaker(void)
{	uint8_t *ppc = gDATABUF[SOCK_TCPC];

	strcpy((char *)ppc, KsmAck245);															// "KSM;ACK;245"
	send(SOCK_TCPC, gDATABUF[SOCK_TCPC], strlen((char *)gDATABUF[SOCK_TCPC]));
}


void enable_sending_local_status(char *pps)
{	uint8_t *ppc = gDATABUF[SOCK_TCPC];

	if (strcmp(pps, "AMP")==0)
	{
		LocStatus = 0;
	}
	else if (strcmp(pps, "LIM")==0)
	{
		LocStatus = 1;
	}
	else if (strcmp(pps, "PAR")==0)
	{
		LocStatus = 2;
	}

	if (LocStatus==0xFF)
	{
		strcpy((char *)ppc, E4001_InvalidData);										// Error code (0x4001): Invalid data
	}
	else
	{
		strcpy((char *)ppc, KsmAck245);														// "KSM;ACK;245"
	}
	send(SOCK_TCPC, gDATABUF[SOCK_TCPC], strlen((char *)gDATABUF[SOCK_TCPC]));
	count_local_status = 1000;
}


void Disconnect_Controller(void)
{	uint8_t *ppc = gDATABUF[SOCK_TCPC];

	strcpy((char *)ppc, KsmAck245);															// "KSM;ACK;245"
	send(SOCK_TCPC, gDATABUF[SOCK_TCPC], strlen((char *)gDATABUF[SOCK_TCPC]));

	disconnect(SOCK_TCPS);	
	disconnect(SOCK_TCPC);
	tcplisten_opened = 0;
	CONTROLLER_FOUND = 0;
	tcp_state = 0;
}


void Validate_Control_Message(void)
{	char *pps;

	network_cmd = 0xFF;

	if (examine_checksum()==1)
	{
		pps = strtok((char *)gDATABUF[SOCK_TCPS], ";");

		if (strcmp(pps, "KSM")==0)
		{
			pps = strtok(NULL, ";");

			if (strcmp(pps, "SPKCONTROL_GET")==0)								{ network_cmd = CMD_SPKCONTROL_GET; }						//
			else if (strcmp(pps, "SPKCONTROL_SET")==0)					{ network_cmd = CMD_SPKCONTROL_SET; }						//
			else if (strcmp(pps, "SPKDELAY_GET")==0)						{ network_cmd = CMD_SPKDELAY_GET; }
			else if (strcmp(pps, "SPKDELAY_SET")==0)						{ network_cmd = CMD_SPKDELAY_SET; }
			else if (strcmp(pps, "SPKDIM_GET")==0)							{ network_cmd = CMD_SPKDIM_GET; }								//
			else if (strcmp(pps, "SPKDIM_SET")==0)							{ network_cmd = CMD_SPKDIM_SET; }								//
			else if (strcmp(pps, "SPKEQPARAMS_GET")==0)					{ network_cmd = CMD_SPKEQPARAMS_GET; }
			else if (strcmp(pps, "SPKEQPARAMS_SET")==0)					{ network_cmd = CMD_SPKEQPARAMS_SET; }
			else if (strcmp(pps, "SPKLEDENABLE_GET")==0)				{ network_cmd = CMD_SPKLEDENABLE_GET; }					//
			else if (strcmp(pps, "SPKLEDENABLE_SET")==0)				{ network_cmd = CMD_SPKLEDENABLE_SET; }					//
			else if (strcmp(pps, "SPKLOCATE")==0)								{ network_cmd = CMD_SPKLOCATE; }								//
			else if (strcmp(pps, "SPKMUTE_GET")==0)							{ network_cmd = CMD_SPKMUTE_GET; }							//
			else if (strcmp(pps, "SPKMUTE_SET")==0)							{ network_cmd = CMD_SPKMUTE_SET; }							//
			else if (strcmp(pps, "SPKNAME_GET")==0)							{ network_cmd = CMD_SPKNAME_GET; }							//
			else if (strcmp(pps, "SPKNAME_SET")==0)							{ network_cmd = CMD_SPKNAME_SET; }							//
			else if (strcmp(pps, "SPKSTANDBYDLY_GET")==0)				{ network_cmd = CMD_SPKSTANDBYDLY_GET; }				//
			else if (strcmp(pps, "SPKSTANDBYDLY_SET")==0)				{ network_cmd = CMD_SPKSTANDBYDLY_SET; }				//
			else if (strcmp(pps, "SPKSTANDBYENABLE_GET")==0)		{ network_cmd = CMD_SPKSTANDBYENABLE_GET; }			//
			else if (strcmp(pps, "SPKSTANDBYENABLE_SET")==0)		{ network_cmd = CMD_SPKSTANDBYENABLE_SET; }			//
			else if (strcmp(pps, "SPKTRIM_INC")==0)							{ network_cmd = CMD_SPKTRIM_INC; }
			else if (strcmp(pps, "SPKTRIMLEVEL_GET")==0)				{ network_cmd = CMD_SPKTRIMLEVEL_GET; }
			else if (strcmp(pps, "SPKTRIMLEVEL_SET")==0)				{ network_cmd = CMD_SPKTRIMLEVEL_SET; }
			else if (strcmp(pps, "SPKPRESET_GET")==0)						{ network_cmd = CMD_SPKPRESET_GET; }						//
			else if (strcmp(pps, "SPKPRESET_SET")==0)						{ network_cmd = CMD_SPKPRESET_SET; }						//
			else if (strcmp(pps, "DISCONNECT")==0)							{ network_cmd = CMD_DISCONNECT; }								//
			else if (strcmp(pps, "SPKFIRMWAREVER_GET")==0)			{ network_cmd = CMD_FIRMWARE_INFO; }						//
			else if (strcmp(pps, "SPKTRIGGER")==0)							{ network_cmd = CMD_SPKTRIGGER; }								//
			else if (strcmp(pps, "SPKEQENABLE_GET")==0)					{ network_cmd = CMD_SPKEQENABLE_GET; }
			else if (strcmp(pps, "SPKEQENABLE_SET")==0)					{ network_cmd = CMD_SPKEQENABLE_SET; }
			else if (strcmp(pps, "SPKDIPSWENABLE_GET")==0)			{ network_cmd = CMD_SPKDIPSWENABLE_GET; }				//
			else if (strcmp(pps, "SPKDIPSWENABLE_SET")==0)			{ network_cmd = CMD_SPKDIPSWENABLE_SET; }

			else if (strcmp(pps, "SPKFWSTART")==0)							{ network_cmd = CMD_SPKFWSTART; }
			else if (strcmp(pps, "FWDATA")==0)									{ network_cmd = CMD_FWDATA; }
			else if (strcmp(pps, "SPKFWCOMPLETE")==0)						{ network_cmd = CMD_SPKFWCOMPLETE; }

			else if (strcmp(pps, "ACK")==0)											{ network_cmd = CMD_RES; }
			else if (strcmp(pps, "NACK")==0)										{ network_cmd = CMD_RES; }
			else																								{ network_cmd = CMD_UNKNOWN; }

			pps = strtok(NULL, ";");

			switch (network_cmd)
			{
				case CMD_DISCONNECT:
					Disconnect_Controller();
				break;

				case CMD_SPKSTANDBYENABLE_GET:
				case CMD_SPKMUTE_GET:
				case CMD_SPKDIM_GET:
				case CMD_SPKLEDENABLE_GET:
				case CMD_SPKCONTROL_GET:
				case CMD_SPKDIPSWENABLE_GET:
					tcp_send_onoff_status(network_cmd);
				break;

				case CMD_SPKSTANDBYENABLE_SET:
				case CMD_SPKMUTE_SET:
				case CMD_SPKDIM_SET:
				case CMD_SPKLEDENABLE_SET:
				case CMD_SPKCONTROL_SET:
				case CMD_SPKDIPSWENABLE_SET:
					tcp_change_onoff_status(pps, network_cmd);
				break;

				case CMD_SPKEQENABLE_GET:
					tcp_send_eqen_status(pps);
				break;

				case CMD_SPKEQENABLE_SET:
					tcp_change_eqen_status(pps);
				break;

				case CMD_SPKDELAY_GET:
					tcp_send_speaker_delay();
				break;

				case CMD_SPKDELAY_SET:
					tcp_change_speaker_delay(pps);
				break;

				case CMD_SPKEQPARAMS_GET:
					tcp_send_speaker_eqparams(pps);
				break;

				case CMD_SPKEQPARAMS_SET:
					tcp_change_speaker_eqparams(pps);
				break;

				case CMD_SPKLOCATE:
					Locate_This_Speaker();
				break;

				case CMD_SPKNAME_GET:
					tcp_send_speaker_name();
				break;

				case CMD_SPKNAME_SET:
					tcp_change_speaker_name(pps);
				break;

				case CMD_SPKPRESET_GET:
					tcp_send_preset_select();
				break;

				case CMD_SPKPRESET_SET:
					tcp_change_preset_select(pps);
				break;

				case CMD_SPKSTANDBYDLY_GET:
					tcp_send_standby_delay();
				break;

				case CMD_SPKSTANDBYDLY_SET:
					tcp_change_standby_delay(pps);
				break;

				case CMD_SPKTRIMLEVEL_GET:
					tcp_send_trim_level();
				break;

				case CMD_SPKTRIM_INC:
					tcp_change_trim_level(pps, 1);
				break;

				case CMD_SPKTRIMLEVEL_SET:
					tcp_change_trim_level(pps, 0);
				break;

				case CMD_FIRMWARE_INFO:
					tcp_send_firmware_info();
				break;

				case CMD_SPKFWSTART:
				case CMD_FWDATA:
				case CMD_SPKFWCOMPLETE:
					firmware_download_proc(pps, network_cmd);
				break;

				case CMD_SPKTRIGGER:
					enable_sending_local_status(pps);
				break;

				case CMD_UNKNOWN:
					tcp_response_unknown_message();
				break;

				default:
					__NOP();
				break;
			}
		}
		else
		{
			network_cmd = CMD_ERROR254;
		}
	}
	else
	{
		network_cmd = CMD_ERROR253;
	}
	KSM_cmd[cmd_idx & 0x1F] = network_cmd;
	cmd_idx++;
	Adjust_RGBLED((cmd_idx & 0x00000380)>>7);
	if (cmd_idx>207)
	{
		__NOP();
	}
}


void UDP_LISTEN(void)
{
	switch (getSn_SR(SOCK_UDPS))																					// retrieve the status of socket 2
	{
		case SOCK_UDP:																											// Socket initialized (opened)
			if (getSn_IR(SOCK_UDPS) & Sn_IR_RECV)
			{
				setSn_IR(SOCK_UDPS, Sn_IR_RECV);																// Sn_IR的RECV位置1
			}

			if ((len=getSn_RX_RSR(SOCK_UDPS))>0)
			{ 
				memset(gDATABUF, 0, len+1);
				recvfrom(SOCK_UDPS, gDATABUF[SOCK_TCPC], len, Control_IP, &CtrlTxPort);		// Get data received by W5500 through SPI
				Validate_Controller();
			}
		break;

		case SOCK_CLOSED:
			socket(SOCK_UDPS, Sn_MR_UDP, udp_localport, 0);										// Open a localport as UDP mode on Socket 2
		break;
	}
	return;
}

uint8_t sir;

void tcp_listen_proc(void)
{
	switch (getSn_SR(SOCK_TCPS))													// 获取socket0的状态
	{
		case SOCK_INIT:																		// Socket处于初始化完成(打开)状态
			listen(SOCK_TCPS);
		break;

		case SOCK_LISTEN:																		// Socket处于初始化完成(打开)状态
			sir = getSn_IR(SOCK_TCPS);
			if (sir!=0)
			{
				__NOP();
			}
			tcplisten_opened = 1;
		break;

		case SOCK_SYNSENT:
			__NOP();
		break;

		case SOCK_SYNRECV:
			__NOP();
		break;

		case SOCK_ESTABLISHED:														// Socket处于连接建立状态
			if (getSn_IR(SOCK_TCPS) & Sn_IR_CON)
			{
				setSn_IR(SOCK_TCPS, Sn_IR_CON);								// Sn_IR的CON位置1，通知W5500连接已建立
			}

			len = getSn_RX_RSR(SOCK_TCPS);										// len=Socket0接收缓存中已接收和保存的数据大小
			if (len)
			{
				recv(SOCK_TCPS, gDATABUF[SOCK_TCPS], len);
				gDATABUF[SOCK_TCPS][len] = 0;
				__NOP();
				Validate_Control_Message();
			}
		break;

		case SOCK_CLOSE_WAIT:															// Socket处于等待关闭状态
			disconnect(SOCK_TCPS);	
		break;

		case SOCK_CLOSED:																	// Socket处于关闭状态
			socket(SOCK_TCPS, Sn_MR_TCP, tcp_listenport, 0x00);				// 打开Socket0，打开一个本地端口
		break;
	}
}


void tcp_client_proc(void)
{
	switch (getSn_SR(SOCK_TCPC))													// get socket 1 (tcp client) status
	{
		case SOCK_INIT:																			// Socket处于初始化完成(打开)状态
			connect(SOCK_TCPC, Control_IP, CtrlRxPort);
		break;

		case SOCK_ESTABLISHED:														// Socket处于连接建立状态
			if (getSn_IR(SOCK_TCPC) & Sn_IR_CON)
			{
				setSn_IR(SOCK_TCPC, Sn_IR_CON);								// Sn_IR的CON位置1，通知W5500连接已建立
			}

			if (tcp_state==0)
			{
				__NOP();
				if (tcplisten_opened == 1)
				{
					tcp_send_speaker_available();
				}
				__NOP();
			}
			else
			{
				__NOP();
			}											
		break;

		case SOCK_CLOSE_WAIT:															// Socket处于等待关闭状态
			disconnect(SOCK_TCPC);	
		break;

		case SOCK_CLOSED:																	// Socket处于关闭状态
			socket(SOCK_TCPC, Sn_MR_TCP, tcp_outputport, 0x00);				// 打开Socket0，打开一个本地端口
		break;
	}
}


void tcp_proc(void)
{
	tcp_listen_proc();
	tcp_client_proc();
}

