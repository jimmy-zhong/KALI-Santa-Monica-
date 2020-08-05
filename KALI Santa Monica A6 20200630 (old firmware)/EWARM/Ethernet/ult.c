#include "ult.h"
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>


static uint8_t  fac_us=0;//us延时倍乘数
static uint16_t fac_ms=0;//ms延时倍乘数

//初始化延迟函数
//SYSTICK的时钟固定为HCLK时钟的1/8
//SYSCLK:系统时钟
void Systick_Init (uint8_t SYSCLK)	// 定义SYSCLK为系统AHB时钟
{
	SysTick->CTRL&=0xffffffff;	// SysTick的CTRL(控制及状态寄存器)的bit2(CLKSOURCE)置0,选择外部时钟(STCLK);若为1选择内部时钟(FCLK) 
	fac_us=SYSCLK/8;		    		// 在STM32中SysTick以HCLK(AHB时钟)或HCLK/8作为运行时钟
	fac_ms=(uint16_t)fac_us*1000;
}								    
//延时nms
//注意nms的范围
//SysTick->LOAD为24位寄存器,所以,最大延时为:
//nms<=0xffffff*8*1000/SYSCLK
//SYSCLK单位为Hz,nms单位为ms
//对72M条件下,time_ms<=1864 
//当需要延时s以上时，请调用Delay_s函数
void Delay_s(uint32_t time_s)
{
  for(;time_s>0;time_s--)
    Delay_ms(1000);
}
void Delay_ms(uint32_t time_ms )
{	uint32_t temp;

	SysTick->LOAD=(uint32_t)time_ms*fac_ms;//时间加载(SysTick->LOAD为24bit)
	SysTick->VAL =0x00;           //清空计数器
	SysTick->CTRL=0x01 ;          //开始倒数  
	do
	{
		temp=SysTick->CTRL;
	}
	while(temp&0x01&&!(temp&(1<<16)));//等待时间到达   
	SysTick->CTRL=0x00;       //关闭计数器
	SysTick->VAL =0X00;       //清空计数器	  	    
}   
//延时nus
//nus为要延时的us数.		    								   
void Delay_us(uint32_t time_us)
{	uint32_t temp;

	SysTick->LOAD=time_us*fac_us; //时间加载	  		 
	SysTick->VAL=0x00;        //清空计数器
	SysTick->CTRL=0x01 ;      //开始倒数 	 
	do
	{
		temp=SysTick->CTRL;
	}
	while(temp&0x01&&!(temp&(1<<16)));//等待时间到达   
	SysTick->CTRL=0x00;       //关闭计数器
	SysTick->VAL =0X00;       //清空计数器	 
}



uint16_t ATOI(char *str, uint16_t base)
{
  unsigned int num = 0;
  while (*str !=0)
          num = num * base + C2D(*str++);
  return num;
}

uint32_t ATOI32(char *str, uint16_t base)
{
  uint32_t num = 0;
  while (*str !=0)
          num = num * base + C2D(*str++);
  return num;
}


void itoa(uint16_t n, uint8_t str[5], uint8_t len)
{	uint8_t i=len-1;

  memset(str, 0x20, len);
  do{
		str[i--]=n%10+'0';
	}while((n/=10)>0);

 return;
}

int ValidATOI(char *str, int base, int *ret)
{
  int c;
  char *tstr = str;
  if(str == 0 || *str == '\0') return 0;
  while(*tstr != '\0')
  {
    c = C2D(*tstr);
    if( c >= 0 && c < base) tstr++;
    else    return 0;
  }
  
  *ret = ATOI(str,base);
  return 1;
}
 
void replacetochar(char *str,	char oldchar, char newchar)
{
  int x;
  for (x = 0; str[x]; x++) 
    if (str[x] == oldchar) str[x] = newchar;	
}

char C2D(uint8_t c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return 10 + c -'a';
	if (c >= 'A' && c <= 'F')
		return 10 + c -'A';

	return (char)c;
}

uint16_t swaps(uint16_t i)
{	uint16_t ret=0;

  ret = (i & 0xFF) << 8;
  ret |= ((i >> 8)& 0xFF);
  return ret;	
}

uint32_t swapl(uint32_t l)
{	uint32_t ret=0;

  ret = (l & 0xFF) << 24;
  ret |= ((l >> 8) & 0xFF) << 16;
  ret |= ((l >> 16) & 0xFF) << 8;
  ret |= ((l >> 24) & 0xFF);
  return ret;
}


//get mid str
void mid(int8_t *src, int8_t *s1, int8_t *s2, int8_t *sub)
{
	int8_t *sub1;
	int8_t *sub2;
	uint16_t n;

  sub1 = strstr(src, s1);
  sub1+=strlen(s1);
  sub2=strstr(sub1,s2);
  n=sub2-sub1;
  strncpy(sub,sub1,n);
  sub[n]=0;
}


void inet_addr_(unsigned char *addr, unsigned char *ip)
{	int i;
//	u_long inetaddr = 0;
	char taddr[30];
	char * nexttok;
	char num;

	strcpy(taddr,(char *)addr);
	
	nexttok = taddr;
	for(i = 0; i < 4 ; i++)
	{
		nexttok = strtok(nexttok,".");
		if(nexttok[0] == '0' && nexttok[1] == 'x') num = ATOI(nexttok+2,0x10);
		else num = ATOI(nexttok,10);
		
		ip[i] = num;
		nexttok = NULL;
	}
}	
#ifdef USE_FULL_ASSERT

void assert_failed(uint8_t *file, uint32_t line)
{ 
 
  while (1)
  {
  }
}

#endif



