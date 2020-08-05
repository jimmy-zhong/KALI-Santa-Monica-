#ifndef __CONFIG_H__
#define __CONFIG_H__


#include "stm32f107xc.h"
#include "device.h"
#define __GNUC__


#define SOCK_TCPS							0
#define SOCK_PING							0
//#define SOCK_HUMTEM						0
#define SOCK_TCPC             1
#define SOCK_UDPS							2
#define SOCK_WEIBO						2
#define SOCK_DHCP							3
#define SOCK_HTTP							3							 
#define SOCK_HTTPS						4
#define SOCK_DNS							5
#define SOCK_SMTP							6
#define SOCK_NTP							7

#define ON	                 	1
#define OFF	                 	0
#define HIGH	           	 		1
#define LOW		             		0

#define CONFIG_MSG_LEN        sizeof(CONFIG_MSG) - 4 // the 4 bytes OP will not save to EEPROM

#define MAX_BUF_SIZE		 			1460
#define KEEP_ALIVE_TIME	     	30	// 30sec
// SRAM address range is 0x2000 0000 ~ 0x2000 BFFF (48KB)
#define SOCK_BUF_ADDR 	      0x20000000
#define AppBackAddress        0x08020000 //from 128K
#define ConfigAddr		      	0x0800FC00

#define NORMAL_STATE          0
#define NEW_APP_IN_BACK       1 //there is new app in back address
#define CONFIGTOOL_FW_UP      2 //configtool update f/w in app


#pragma pack(1)
typedef struct _CONFIG_MSG
{
  uint8_t op[4];//header: FIND;SETT;FACT...
  uint8_t mac[6];
  uint8_t sw_ver[2];
  uint8_t lip[4];
  uint8_t sub[4];
  uint8_t gw[4];
  uint8_t dns[4];	
  uint8_t dhcp;
  uint8_t debug;

  uint16_t fw_len;
  uint8_t state;
  
}CONFIG_MSG;
#pragma pack()
//extern CONFIG_MSG  ConfigMsg, RecvMsg;
extern CONFIG_MSG  ConfigMsg;

#endif


