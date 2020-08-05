#include <stdio.h> 
#include <string.h>
#include "stm32f1xx_hal.h"
#include "device.h"
#include "config.h"
#include "socket.h"
#include "dhcp.h"
#include "ult.h"
#include "app.h"
#include "w5500.h"
#include "tcp_proc.h"
#include "misc.h"


extern uint8_t MAC[6];								//public buffer for DHCP, DNS, HTTP
extern uint8_t CONTROLLER_FOUND;
extern uint8_t tcp_state;
extern uint8_t HOST_NAME[13]; 
extern uint8_t SystemLedColor;

//CONFIG_MSG ConfigMsg, RecvMsg;
CONFIG_MSG ConfigMsg;

uint8_t txsize[MAX_SOCK_NUM] = {4,2,2,2,2,2,2,0};
uint8_t rxsize[MAX_SOCK_NUM] = {4,2,2,2,2,2,2,0};
uint8_t pub_buf[1460];
uint8_t mac[6]={0x00,0x08,0xdc,0x11,0x11,0x11};
uint8_t lip[4]={192,168,1,110};
uint8_t sub[4]={255,255,255,0};
uint8_t gw[4]={192,168,1,1};


/*
void Reset_W5500(void)
{
  GPIO_ResetBits(GPIOB, WIZ_RESET);
  Delay_us(2);  
  GPIO_SetBits(GPIOB, WIZ_RESET);
  Delay_ms(1600);
}

void reboot(void)
{	pFunction Jump_To_Application;
  uint32_t JumpAddress;

  JumpAddress = *(volatile uint32_t *)(0x00000004);
  Jump_To_Application = (pFunction)JumpAddress;
  Jump_To_Application();
}
*/

uint8_t xx[3][4];


uint32_t* xx0 = (uint32_t *)xx[0];
uint32_t* xx1 = (uint32_t *)xx[1];
uint32_t* xx2 = (uint32_t *)xx[2];

void set_network(void)
{//	uint32_t tmp;

	*xx0 = HAL_GetUIDw0();
	*xx1 = HAL_GetUIDw1();
	*xx2 = HAL_GetUIDw2();
	
	mac[0] = ((uint32_t)xx[1][1]*xx[2][3]) & 0xFF;
	mac[1] = ((uint32_t)xx[0][0]*xx[1][2]) & 0xFF;
	mac[2] = ((uint32_t)xx[0][1]*xx[1][3]) & 0xFF;
	mac[3] = ((uint32_t)xx[1][0]*xx[2][2]) & 0xFF;
	mac[4] = ((uint32_t)xx[2][1]*xx[0][3]) & 0xFF;
	mac[5] = ((uint32_t)xx[2][0]*xx[0][2]) & 0xFF;

/*
	tmp = HAL_GetUIDw0();
	mac[0] = tmp & 0x000000FE;
	mac[1] = (tmp & 0x0000FF00) >> 8;
	mac[2] = (tmp & 0x00FF0000) >> 16;
	mac[3] = (tmp & 0xFF000000) >> 24;
	tmp = HAL_GetUIDw1();
	mac[4] = tmp & 0x000000FF;
	mac[5] = (tmp & 0x0000FF00) >> 8;
*/
  memcpy(ConfigMsg.mac, mac, 6);
//  memset(mac, 0, 6);

  setSHAR(ConfigMsg.mac);										/*配置Mac地址*/
  getSHAR(mac);															/*配置Mac地址*/
//  setSHAR(ConfigMsg.mac);										/*配置Mac地址*/
//  getSHAR(mac);															/*配置Mac地址*/

  sysinit(txsize, rxsize);									/*初始化8个socket*/

  setRTR(2000);															/*设置溢出时间值*/
  setRCR(3);																/*设置最大重新发送次数*/

	sprintf((char*)HOST_NAME, "%3s-%02X%02X%02X",DEVICE_TYPE, ConfigMsg.mac[3],
					                  ConfigMsg.mac[4], ConfigMsg.mac[5]); 
//	sprintf((char*)HOST_NAME, "%02X%02X%02X%02X%02X%02X", ConfigMsg.mac[5], ConfigMsg.mac[4],
//					                   ConfigMsg.mac[3], ConfigMsg.mac[2], ConfigMsg.mac[1], ConfigMsg.mac[0]); 
}


uint8_t W5500_LINKED = 0;
uint8_t W5500_ACTIVE = 0;
uint8_t tmp;

void W5500_LOOP(void)
{
	W5500_LINKED = IINCHIP_READ(PHYCFGR) & 0x01;
	if (W5500_LINKED==0)
	{
		if (W5500_ACTIVE==1)
		{
			Switch_LED(LED_GREEN);
			tcp_state = 0;
			CONTROLLER_FOUND = 0;
			W5500_ACTIVE=0;
		}
		return;
	}
	else
	{
		if (W5500_ACTIVE==0)
		{
			init_dhcp_client();
			W5500_ACTIVE=1;
		}
	}

	if ((tmp=DHCP_run()) == DHCP_IP_LEASED)
	{
		Switch_LED(SystemLedColor);
		
		if (CONTROLLER_FOUND==0)
		{
			UDP_LISTEN();
		}
		else
		{
			tcp_proc();
		}
	}
	else
	{
		Switch_LED(LED_GREEN);
	}
}


