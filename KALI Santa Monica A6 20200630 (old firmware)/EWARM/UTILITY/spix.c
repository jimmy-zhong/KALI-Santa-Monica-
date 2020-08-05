
/*STM32的SPI串行外围总线接口，本程序，是将STM32的SPI配置为全双工模式，且NSS使用的软件模式。
在使用SPI前，下面的这个过程我们必须理解，即STM32作为主机发送一个字节数据时，必然能接收到一个数据，
至于数据是否处理，由程序操作。
● 全双工模式(BIDIMODE=0并且RXONLY=0) 
─  当写入数据到SPI_DR寄存器(发送缓冲器)后，传输开始； 
─  在传送第一位数据的同时，数据被并行地从发送缓冲器传送到8位的移位寄存器中，
然后按顺序被串行地移位送到MOSI引脚上； 
─  与此同时，在MISO引脚上接收到的数据，按顺序被串行地移位进入8位的移位寄存器
中，然后被并行地传送到SPI_DR寄存器(接收缓冲器)中。 
注意：也就是说，在主机模式下，发送和接收是同时进行的，所以我们发送了一个数据，也就能接收到一个数据。
而STM32内部硬件是这个过程的支撑*/


#include "stm32f1xx_hal.h"
#include "config.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "misc.h"
#include "SPIX.h"


SPI_HandleTypeDef SPI1_W5500;
SPI_HandleTypeDef SPI2_adau1452;
SPI_HandleTypeDef SPI3_FLASH;

#define FLASH_SELECTED()				GPIO_ResetBits(GPIOD, GPIO_PIN_7)
#define FLASH_UNSELECTED()			GPIO_SetBits(GPIOD, GPIO_PIN_7)
#define DSP_SELECTED()					GPIO_ResetBits(GPIOD, GPIO_PIN_8)
#define DSP_UNSELECTED()				GPIO_SetBits(GPIOD, GPIO_PIN_8)

//#define FLASH_SCS			    GPIO_PIN_4	// out
//#define FLASH_SCLK				GPIO_PIN_5	// out
//#define FLASH_MISO				GPIO_PIN_6	// in
//#define FLASH_MOSI				GPIO_PIN_7	// out

//#define FLASH_RESET		    GPIO_PIN_0	// out (PB0)
//#define FLASH_INT			    GPIO_PIN_3	// in (PA3)



void WIZ_SPI_Init(void)
{
// 初始化GPIO管脚和SPI的参数设置：建立SPI和GPIO的初始化结构体
	GPIO_InitTypeDef GPIO_InitStructure;

	__HAL_RCC_SPI1_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_AFIO_CLK_ENABLE();

// GPIO配置，设置为PA输出
  GPIO_InitStructure.Pin = GPIO_PIN_4;														// 选定PA4进行后续设置
  GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;								// PA4通信速度设为50MHz
  GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;									// PA4设为推挽输出
  HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);											// PA4完成配置
  GPIO_SetBits(GPIOA, GPIO_PIN_4);																// 配置FLASH的片选信号线PA4，并设为高电平，也就是不选中FLASH

// 配置SPI1的SCK, MISO and MOSI 
  GPIO_InitStructure.Pin = GPIO_PIN_5| GPIO_PIN_6| GPIO_PIN_7;		// 选定PA5/6/7进行后续设置
  GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;								// PA5/6/7通信速度设为50MHz
  GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;											// PA5/6/7设为复用推挽输出，输入输出的方向完全由内部控制，不需要程序处理
  HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);											// PA5/6/7完成配置

// SPI配置
	SPI1_W5500.Instance = SPI1;
	SPI1_W5500.Init.Direction = SPI_DIRECTION_2LINES;									// SPI设置为双线双向全双工
	SPI1_W5500.Init.Mode = SPI_MODE_MASTER;														// 设置SPI工作模式:设置为主SPI
	SPI1_W5500.Init.DataSize = SPI_DATASIZE_8BIT;											// 设置SPI的数据大小:SPI发送接收8位帧结构
	SPI1_W5500.Init.CLKPolarity = SPI_POLARITY_LOW;										// 选择了串行时钟的稳态:时钟低
	SPI1_W5500.Init.CLKPhase = SPI_PHASE_1EDGE;												// 数据捕获于第1个时钟沿
	SPI1_W5500.Init.NSS = SPI_NSS_SOFT;																// NSS信号由硬件（NSS管脚）还是软件（使用SSI位）控制
	SPI1_W5500.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;				// 定义波特率预分频的值为4
	SPI1_W5500.Init.FirstBit = SPI_FIRSTBIT_MSB;												// 指定数据传输从MSB位还是LSB位开始
	SPI1_W5500.Init.CRCPolynomial = 7;																	// CRC值计算的多项式

	HAL_SPI_Init(&SPI1_W5500);																					// 利用SPI结构体初始化函数初始化SPI结构体
	__HAL_SPI_ENABLE(&SPI1_W5500);																			// 使能SPI1外设
}


uint8_t spi2_one_byte_inout(uint8_t data)
{
	while ((SPI2_adau1452.Instance->SR & SPI_FLAG_TXE) == RESET);
	SPI2_adau1452.Instance->DR = data;
	while ((SPI2_adau1452.Instance->SR & SPI_FLAG_RXNE) == RESET); 	// 等待接收完一个字节
	return (SPI2_adau1452.Instance->DR);
}


uint16_t spi2_adau1452_readreg(uint16_t address)
{	uint16_t pp;

	DSP_SELECTED();

	spi2_one_byte_inout(0x01);
	spi2_one_byte_inout((address / 256) & 0x000000FF);
	spi2_one_byte_inout(address & 0x000000FF);

	pp = spi2_one_byte_inout(0x00)*256;
	pp += spi2_one_byte_inout(0x00);

	DSP_UNSELECTED();

	return (pp);
}


void SIGMA_WRITE_REGISTER_BLOCK(uint8_t dev_address, uint16_t CMD, uint16_t Num, uint8_t *data)
{
	DSP_SELECTED();
	spi2_one_byte_inout(dev_address);
	spi2_one_byte_inout((CMD / 256) & 0x00FF);
	spi2_one_byte_inout(CMD & 0x00FF);
	
	while (Num-->0)
	{
		spi2_one_byte_inout(*data++);
	}
	
	DSP_UNSELECTED();
}


void SIGMA_WRITE_REGISTER_UINT32(uint8_t dev_address, uint16_t CMD, uint16_t Num, uint8_t *data)
{
	DSP_SELECTED();
	spi2_one_byte_inout(dev_address);
	spi2_one_byte_inout((CMD / 256) & 0x00FF);
	spi2_one_byte_inout(CMD & 0x00FF);
	
	spi2_one_byte_inout(*(data+3));
	spi2_one_byte_inout(*(data+2));
	spi2_one_byte_inout(*(data+1));
	spi2_one_byte_inout(*data);
	
	DSP_UNSELECTED();
}


void SIGMA_WRITE_DELAY(uint8_t dev_address, uint16_t Num, uint8_t *data )
{
	wait_1ms((*data)*256+(*(data+1)));
}


void set_adau1452_SPI_mode(void)
{
	DSP_SELECTED();
	wait_1ms(1);
	DSP_UNSELECTED();
	wait_1ms(1);
	DSP_SELECTED();
	wait_1ms(1);
	DSP_UNSELECTED();
	wait_1ms(1);
	DSP_SELECTED();
	wait_1ms(1);
	DSP_UNSELECTED();
	wait_1ms(1);
}


uint16_t adi_reg1;
uint16_t adi_reg2;

void ADAU1452_SPI_Init(void)
{
// 初始化GPIO管脚和SPI的参数设置：建立SPI和GPIO的初始化结构体
	GPIO_InitTypeDef GPIO_InitStructure;

	__HAL_RCC_SPI2_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_AFIO_CLK_ENABLE();

// GPIO配置，设置为PA输出
  GPIO_InitStructure.Pin = GPIO_PIN_8;																	// 选定PB9进行后续设置
  GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;											// PB9通信速度设为50MHz
  GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;												// PB9设为推挽输出
  HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);														// PB9完成配置
	GPIO_SetBits(GPIOD, GPIO_PIN_8);																			// 配置FLASH的片选信号线PB9，并设为高电平，也就是不选中FLASH

// 配置SPI1的SCK, MISO and MOSI 
  GPIO_InitStructure.Pin = GPIO_PIN_13| GPIO_PIN_14| GPIO_PIN_15;				// 选定PB13/14/15进行后续设置
  GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;											// PB13/14/15通信速度设为50MHz
  GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;														// PB13/14/15设为复用推挽输出，输入输出的方向完全由内部控制，不需要程序处理
  HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);														// PB13/14/15完成配置

// SPI配置
	SPI2_adau1452.Instance = SPI2;
	SPI2_adau1452.Init.Direction = SPI_DIRECTION_2LINES;									// SPI设置为双线双向全双工
	SPI2_adau1452.Init.Mode = SPI_MODE_MASTER;														// 设置SPI工作模式:设置为主SPI
	SPI2_adau1452.Init.DataSize = SPI_DATASIZE_8BIT;											// 设置SPI的数据大小:SPI发送接收8位帧结构
	SPI2_adau1452.Init.CLKPolarity = SPI_POLARITY_LOW;										// 选择了串行时钟的稳态:时钟低
	SPI2_adau1452.Init.CLKPhase = SPI_PHASE_1EDGE;												// 数据捕获于第1个时钟沿
	SPI2_adau1452.Init.NSS = SPI_NSS_SOFT;																// NSS信号由硬件（NSS管脚）还是软件（使用SSI位）控制
	SPI2_adau1452.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;				// 定义波特率预分频的值为4
	SPI2_adau1452.Init.FirstBit = SPI_FIRSTBIT_MSB;												// 指定数据传输从MSB位还是LSB位开始
	SPI2_adau1452.Init.CRCPolynomial = 7;																	// CRC值计算的多项式

	HAL_SPI_Init(&SPI2_adau1452);																					// 利用SPI结构体初始化函数初始化SPI结构体
	__HAL_SPI_ENABLE(&SPI2_adau1452);																			// 使能SPI2外设

	set_adau1452_SPI_mode();
//	Configure_DSP();
//	adi_reg1 = spi2_adau1452_readreg(0xF020);

//	adi_reg = spi2_adau1452_readreg(0xF000);
//	adi_reg2 = spi2_adau1452_readreg(0xF020);
//	adi_reg = spi2_adau1452_readreg(0xF422);
}


void DeRegulate_ADAU1452_SPI(void)
{
//	HAL_SPI_DeInit(&SPI2_adau1452);
//	__HAL_SPI_DISABLE(&SPI2_adau1452);
	GPIOB->CRH &= 0x000FFFFF;
	GPIOB->CRH |= 0x44400000;
	GPIOD->CRH &= 0xFFFFFFF0;
	GPIOD->CRH |= 0x00000004;
}


uint8_t spi3_one_byte_inout(uint8_t data)
{
	while ((SPI3_FLASH.Instance->SR & SPI_FLAG_TXE) == RESET);
	SPI3_FLASH.Instance->DR = data;
	while ((SPI3_FLASH.Instance->SR & SPI_FLAG_RXNE) == RESET); 	// 等待接收完一个字节
	return (SPI3_FLASH.Instance->DR);
}


uint8_t M_ID;
uint8_t D_ID1;
uint8_t D_ID2;
uint8_t SR1;
uint8_t SR2;

void spi3_flash_device(void)
{	
	FLASH_SELECTED();
	spi3_one_byte_inout(0x9F);
	M_ID = spi3_one_byte_inout(0xFF);
	D_ID1 = spi3_one_byte_inout(0xFF);
	D_ID2 = spi3_one_byte_inout(0xFF);
	FLASH_UNSELECTED();
}


void spi3_flash_status1(void)
{	
	FLASH_SELECTED();
	spi3_one_byte_inout(0x05);
	SR1 = spi3_one_byte_inout(0xFF);
	FLASH_UNSELECTED();
}


uint8_t Flash_Is_Busy(void)
{
	spi3_flash_status1();
	return (SR1 & 0x01);
}


void spi3_flash_status2(void)
{	
	FLASH_SELECTED();
	spi3_one_byte_inout(0x35);
	SR2 = spi3_one_byte_inout(0xFF);
	FLASH_UNSELECTED();
}


void spi3_flash_enwrite(void)
{	
	FLASH_SELECTED();
	spi3_one_byte_inout(0x06);
	FLASH_UNSELECTED();
}


void spi3_flash_diswrite(void)
{	
	FLASH_SELECTED();
	spi3_one_byte_inout(0x04);
	FLASH_UNSELECTED();
}


uint8_t spi3_flash_erase(uint8_t block_erase, uint16_t page)
{	uint32_t address;

	spi3_flash_enwrite();

	switch (block_erase)
	{
		case FLASH_ERASE_4K:
			address = (uint32_t)page*FLASH_BLOCK_4K;
		break;

		case FLASH_ERASE_32K:
			address = (uint32_t)page*FLASH_BLOCK_32K;
		break;

		case FLASH_ERASE_64K:
			address = (uint32_t)page*FLASH_BLOCK_64K;
		break;

		case FLASH_ERASE_CHIP:
		case FLASH_ERASE_CHIP1:
			FLASH_SELECTED();
			spi3_one_byte_inout(block_erase);
			FLASH_UNSELECTED();
			return (1);

		default:
			return (0);
	}

	FLASH_SELECTED();
	spi3_one_byte_inout(block_erase);
	spi3_one_byte_inout((address / 65536) &  0x000000FF);
	spi3_one_byte_inout((address / 256) & 0x000000FF);
	spi3_one_byte_inout(address & 0x000000FF);
	FLASH_UNSELECTED();
//	wait_1ms(100);
	while (Flash_Is_Busy()==1);
/*	{
		spi3_flash_status1();
		if ((SR1 & 0x01) == 0) break;
	}*/
	return (1);
}


void ClearMemorySpaceFor(uint32_t memsize)
{	uint32_t page64k;
	uint32_t page32k;
	uint32_t page4k;
	uint32_t pageremained;
	uint32_t iix;

	page64k = memsize >> 16;			// FLASH_BLOCK_64K;
	pageremained = memsize & 0x0000FFFF;
	page32k = pageremained >> 15;
	pageremained = pageremained & 0x00007FFF;
	page4k = pageremained >> 12;
	pageremained = pageremained & 0x00000FFF;
	if (pageremained > 0) page4k++;
	for (iix=0; iix<page64k; iix++)
		spi3_flash_erase(FLASH_ERASE_64K, iix+4);
	for (iix=0; iix<page32k; iix++)
		spi3_flash_erase(FLASH_ERASE_32K, iix+8+page64k*2);
	for (iix=0; iix<page4k; iix++)
		spi3_flash_erase(FLASH_ERASE_4K, iix+64+page64k*16+page32k*8);
}


uint8_t spi3_flash_read(uint8_t *read_buf, uint32_t address, uint16_t no)
{	uint8_t *pp = read_buf;

	FLASH_SELECTED();
	spi3_one_byte_inout(0x0B);
	spi3_one_byte_inout((address / 65536) &  0x000000FF);
	spi3_one_byte_inout((address / 256) & 0x000000FF);
	spi3_one_byte_inout(address & 0x000000FF);
	spi3_one_byte_inout(0xFF);

	while (no>0)
	{
		*pp++ = spi3_one_byte_inout(0x00);
		no--;
	}
	FLASH_UNSELECTED();

	return (1);
}

uint8_t xrr;

uint8_t spi3_flash_write(uint8_t *write_buf, uint32_t address, uint16_t no)
{	uint8_t *pp = write_buf;
	uint16_t nos;

	while (no>0)
	{
		spi3_flash_enwrite();

		if (no>256)
		{
			nos = 256;
			no -= 256;
		}
		else
		{
			nos = no;
			no = 0;
		}
		FLASH_SELECTED();
		spi3_one_byte_inout(0x02);
		spi3_one_byte_inout((address / 65536) &  0x000000FF);
		spi3_one_byte_inout((address / 256) & 0x000000FF);
		spi3_one_byte_inout(address & 0x000000FF);

		while (nos>0)
		{
			spi3_one_byte_inout(*pp++);
			nos--;
		}
		FLASH_UNSELECTED();
		while (Flash_Is_Busy()==1);

/*		while (1)
		{
			spi3_flash_status1();
			if ((SR1 & 0x01) == 0) break;
		}
		address += 256;*/
	}
//	wait_1ms(1);

	return (1);
}


volatile uint8_t ddr[32] = {0 , };

void FLASH_SPI_Init(void)
{
// 初始化GPIO管脚和SPI的参数设置：建立SPI和GPIO的初始化结构体
	GPIO_InitTypeDef GPIO_InitStructure;

	__HAL_AFIO_REMAP_SWJ_NOJTAG();
	__HAL_RCC_SPI3_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
//	__HAL_RCC_AFIO_CLK_ENABLE();

// GPIO配置，设置为PB输出
  GPIO_InitStructure.Pin = GPIO_PIN_7;																	// 选定PD5进行后续设置
  GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;											// PD7通信速度设为50MHz
  GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;												// PD7设为推挽输出
  HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);														// PD7完成配置
  GPIO_SetBits(GPIOD, GPIO_PIN_7);																			// 配置FLASH的片选信号线PD7，并设为高电平，也就是不选中FLASH

// 配置SPI3的SCK, MISO and MOSI 
  GPIO_InitStructure.Pin = GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5;				// 选定PB3/4/5进行后续设置
  GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;											// PB3/4/5通信速度设为50MHz
  GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;														// PB3/4/5设为复用推挽输出，输入输出的方向完全由内部控制，不需要程序处理
  HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);														// PB3/4/5完成配置

/*
// 配置SPI1的SCK, MISO and MOSI 
  GPIO_InitStructure.Pin = GPIO_PIN_4;				// 选定PB3/4/5进行后续设置
  GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;											// PB3/4/5通信速度设为50MHz
  GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;														// PB3/4/5设为复用推挽输出，输入输出的方向完全由内部控制，不需要程序处理
  HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);														// PB3/4/5完成配置

// 配置SPI1的SCK, MISO and MOSI 
  GPIO_InitStructure.Pin = GPIO_PIN_5;				// 选定PB3/4/5进行后续设置
  GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;											// PB3/4/5通信速度设为50MHz
  GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;														// PB3/4/5设为复用推挽输出，输入输出的方向完全由内部控制，不需要程序处理
  HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);														// PB3/4/5完成配置
*/

// SPI配置
	SPI3_FLASH.Instance = SPI3;
	SPI3_FLASH.Init.Direction = SPI_DIRECTION_2LINES;									// SPI设置为双线双向全双工
	SPI3_FLASH.Init.Mode = SPI_MODE_MASTER;														// 设置SPI工作模式:设置为主SPI
	SPI3_FLASH.Init.DataSize = SPI_DATASIZE_8BIT;											// 设置SPI的数据大小:SPI发送接收8位帧结构
	SPI3_FLASH.Init.CLKPolarity = SPI_POLARITY_LOW;										// 选择了串行时钟的稳态:时钟低
	SPI3_FLASH.Init.CLKPhase = SPI_PHASE_1EDGE;												// 数据捕获于第1个时钟沿
	SPI3_FLASH.Init.NSS = SPI_NSS_SOFT;																// NSS信号由硬件（NSS管脚）还是软件（使用SSI位）控制
	SPI3_FLASH.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;				// 定义波特率预分频的值为4
	SPI3_FLASH.Init.FirstBit = SPI_FIRSTBIT_MSB;												// 指定数据传输从MSB位还是LSB位开始
	SPI3_FLASH.Init.CRCPolynomial = 7;																	// CRC值计算的多项式

	HAL_SPI_Init(&SPI3_FLASH);																					// 利用SPI结构体初始化函数初始化SPI结构体
	__HAL_SPI_ENABLE(&SPI3_FLASH);																			// 使能SPI2外设
	
	spi3_flash_device();
//	spi3_flash_read((uint8_t *)&ddr, 0, 16);
}


// Connected to Data Flash
void WIZ_CS(uint8_t val)
{
	if (val == LOW) 
	{
		GPIO_ResetBits(GPIOA, WIZ_SCS); 	// GPIOA的WIZ_SCS（GPIO_PIN_4）引脚清零拉低
	}
	else if (val == HIGH)
	{
		GPIO_SetBits(GPIOA, WIZ_SCS); 		// GPIOA的WIZ_SCS（GPIO_PIN_4）引脚置1拉高
	}
}


uint8_t SPI1_W5500_SendByte(uint8_t byte)
{
	while ((SPI1_W5500.Instance->SR & SPI_FLAG_TXE) == RESET);
	SPI1_W5500.Instance->DR = byte;
//	SPI_I2S_SendData(SPI2, byte);									   									// 发送一个字节

	while ((SPI1_W5500.Instance->SR & SPI_FLAG_RXNE) == RESET); 	// 等待接收完一个字节

//	return SPI_ReceiveData(SPI2);								   								// 返回收到的数据 
	return (SPI1_W5500.Instance->DR);								   								// 返回收到的数据 
}



