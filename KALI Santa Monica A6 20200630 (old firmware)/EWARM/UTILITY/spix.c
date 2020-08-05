
/*STM32��SPI������Χ���߽ӿڣ��������ǽ�STM32��SPI����Ϊȫ˫��ģʽ����NSSʹ�õ����ģʽ��
��ʹ��SPIǰ�����������������Ǳ�����⣬��STM32��Ϊ��������һ���ֽ�����ʱ����Ȼ�ܽ��յ�һ�����ݣ�
���������Ƿ����ɳ��������
�� ȫ˫��ģʽ(BIDIMODE=0����RXONLY=0) 
��  ��д�����ݵ�SPI_DR�Ĵ���(���ͻ�����)�󣬴��俪ʼ�� 
��  �ڴ��͵�һλ���ݵ�ͬʱ�����ݱ����еشӷ��ͻ��������͵�8λ����λ�Ĵ����У�
Ȼ��˳�򱻴��е���λ�͵�MOSI�����ϣ� 
��  ���ͬʱ����MISO�����Ͻ��յ������ݣ���˳�򱻴��е���λ����8λ����λ�Ĵ���
�У�Ȼ�󱻲��еش��͵�SPI_DR�Ĵ���(���ջ�����)�С� 
ע�⣺Ҳ����˵��������ģʽ�£����ͺͽ�����ͬʱ���еģ��������Ƿ�����һ�����ݣ�Ҳ���ܽ��յ�һ�����ݡ�
��STM32�ڲ�Ӳ����������̵�֧��*/


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
// ��ʼ��GPIO�ܽź�SPI�Ĳ������ã�����SPI��GPIO�ĳ�ʼ���ṹ��
	GPIO_InitTypeDef GPIO_InitStructure;

	__HAL_RCC_SPI1_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_AFIO_CLK_ENABLE();

// GPIO���ã�����ΪPA���
  GPIO_InitStructure.Pin = GPIO_PIN_4;														// ѡ��PA4���к�������
  GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;								// PA4ͨ���ٶ���Ϊ50MHz
  GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;									// PA4��Ϊ�������
  HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);											// PA4�������
  GPIO_SetBits(GPIOA, GPIO_PIN_4);																// ����FLASH��Ƭѡ�ź���PA4������Ϊ�ߵ�ƽ��Ҳ���ǲ�ѡ��FLASH

// ����SPI1��SCK, MISO and MOSI 
  GPIO_InitStructure.Pin = GPIO_PIN_5| GPIO_PIN_6| GPIO_PIN_7;		// ѡ��PA5/6/7���к�������
  GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;								// PA5/6/7ͨ���ٶ���Ϊ50MHz
  GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;											// PA5/6/7��Ϊ���������������������ķ�����ȫ���ڲ����ƣ�����Ҫ������
  HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);											// PA5/6/7�������

// SPI����
	SPI1_W5500.Instance = SPI1;
	SPI1_W5500.Init.Direction = SPI_DIRECTION_2LINES;									// SPI����Ϊ˫��˫��ȫ˫��
	SPI1_W5500.Init.Mode = SPI_MODE_MASTER;														// ����SPI����ģʽ:����Ϊ��SPI
	SPI1_W5500.Init.DataSize = SPI_DATASIZE_8BIT;											// ����SPI�����ݴ�С:SPI���ͽ���8λ֡�ṹ
	SPI1_W5500.Init.CLKPolarity = SPI_POLARITY_LOW;										// ѡ���˴���ʱ�ӵ���̬:ʱ�ӵ�
	SPI1_W5500.Init.CLKPhase = SPI_PHASE_1EDGE;												// ���ݲ����ڵ�1��ʱ����
	SPI1_W5500.Init.NSS = SPI_NSS_SOFT;																// NSS�ź���Ӳ����NSS�ܽţ����������ʹ��SSIλ������
	SPI1_W5500.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;				// ���岨����Ԥ��Ƶ��ֵΪ4
	SPI1_W5500.Init.FirstBit = SPI_FIRSTBIT_MSB;												// ָ�����ݴ����MSBλ����LSBλ��ʼ
	SPI1_W5500.Init.CRCPolynomial = 7;																	// CRCֵ����Ķ���ʽ

	HAL_SPI_Init(&SPI1_W5500);																					// ����SPI�ṹ���ʼ��������ʼ��SPI�ṹ��
	__HAL_SPI_ENABLE(&SPI1_W5500);																			// ʹ��SPI1����
}


uint8_t spi2_one_byte_inout(uint8_t data)
{
	while ((SPI2_adau1452.Instance->SR & SPI_FLAG_TXE) == RESET);
	SPI2_adau1452.Instance->DR = data;
	while ((SPI2_adau1452.Instance->SR & SPI_FLAG_RXNE) == RESET); 	// �ȴ�������һ���ֽ�
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
// ��ʼ��GPIO�ܽź�SPI�Ĳ������ã�����SPI��GPIO�ĳ�ʼ���ṹ��
	GPIO_InitTypeDef GPIO_InitStructure;

	__HAL_RCC_SPI2_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_AFIO_CLK_ENABLE();

// GPIO���ã�����ΪPA���
  GPIO_InitStructure.Pin = GPIO_PIN_8;																	// ѡ��PB9���к�������
  GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;											// PB9ͨ���ٶ���Ϊ50MHz
  GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;												// PB9��Ϊ�������
  HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);														// PB9�������
	GPIO_SetBits(GPIOD, GPIO_PIN_8);																			// ����FLASH��Ƭѡ�ź���PB9������Ϊ�ߵ�ƽ��Ҳ���ǲ�ѡ��FLASH

// ����SPI1��SCK, MISO and MOSI 
  GPIO_InitStructure.Pin = GPIO_PIN_13| GPIO_PIN_14| GPIO_PIN_15;				// ѡ��PB13/14/15���к�������
  GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;											// PB13/14/15ͨ���ٶ���Ϊ50MHz
  GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;														// PB13/14/15��Ϊ���������������������ķ�����ȫ���ڲ����ƣ�����Ҫ������
  HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);														// PB13/14/15�������

// SPI����
	SPI2_adau1452.Instance = SPI2;
	SPI2_adau1452.Init.Direction = SPI_DIRECTION_2LINES;									// SPI����Ϊ˫��˫��ȫ˫��
	SPI2_adau1452.Init.Mode = SPI_MODE_MASTER;														// ����SPI����ģʽ:����Ϊ��SPI
	SPI2_adau1452.Init.DataSize = SPI_DATASIZE_8BIT;											// ����SPI�����ݴ�С:SPI���ͽ���8λ֡�ṹ
	SPI2_adau1452.Init.CLKPolarity = SPI_POLARITY_LOW;										// ѡ���˴���ʱ�ӵ���̬:ʱ�ӵ�
	SPI2_adau1452.Init.CLKPhase = SPI_PHASE_1EDGE;												// ���ݲ����ڵ�1��ʱ����
	SPI2_adau1452.Init.NSS = SPI_NSS_SOFT;																// NSS�ź���Ӳ����NSS�ܽţ����������ʹ��SSIλ������
	SPI2_adau1452.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;				// ���岨����Ԥ��Ƶ��ֵΪ4
	SPI2_adau1452.Init.FirstBit = SPI_FIRSTBIT_MSB;												// ָ�����ݴ����MSBλ����LSBλ��ʼ
	SPI2_adau1452.Init.CRCPolynomial = 7;																	// CRCֵ����Ķ���ʽ

	HAL_SPI_Init(&SPI2_adau1452);																					// ����SPI�ṹ���ʼ��������ʼ��SPI�ṹ��
	__HAL_SPI_ENABLE(&SPI2_adau1452);																			// ʹ��SPI2����

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
	while ((SPI3_FLASH.Instance->SR & SPI_FLAG_RXNE) == RESET); 	// �ȴ�������һ���ֽ�
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
// ��ʼ��GPIO�ܽź�SPI�Ĳ������ã�����SPI��GPIO�ĳ�ʼ���ṹ��
	GPIO_InitTypeDef GPIO_InitStructure;

	__HAL_AFIO_REMAP_SWJ_NOJTAG();
	__HAL_RCC_SPI3_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
//	__HAL_RCC_AFIO_CLK_ENABLE();

// GPIO���ã�����ΪPB���
  GPIO_InitStructure.Pin = GPIO_PIN_7;																	// ѡ��PD5���к�������
  GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;											// PD7ͨ���ٶ���Ϊ50MHz
  GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;												// PD7��Ϊ�������
  HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);														// PD7�������
  GPIO_SetBits(GPIOD, GPIO_PIN_7);																			// ����FLASH��Ƭѡ�ź���PD7������Ϊ�ߵ�ƽ��Ҳ���ǲ�ѡ��FLASH

// ����SPI3��SCK, MISO and MOSI 
  GPIO_InitStructure.Pin = GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5;				// ѡ��PB3/4/5���к�������
  GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;											// PB3/4/5ͨ���ٶ���Ϊ50MHz
  GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;														// PB3/4/5��Ϊ���������������������ķ�����ȫ���ڲ����ƣ�����Ҫ������
  HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);														// PB3/4/5�������

/*
// ����SPI1��SCK, MISO and MOSI 
  GPIO_InitStructure.Pin = GPIO_PIN_4;				// ѡ��PB3/4/5���к�������
  GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;											// PB3/4/5ͨ���ٶ���Ϊ50MHz
  GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;														// PB3/4/5��Ϊ���������������������ķ�����ȫ���ڲ����ƣ�����Ҫ������
  HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);														// PB3/4/5�������

// ����SPI1��SCK, MISO and MOSI 
  GPIO_InitStructure.Pin = GPIO_PIN_5;				// ѡ��PB3/4/5���к�������
  GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;											// PB3/4/5ͨ���ٶ���Ϊ50MHz
  GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;														// PB3/4/5��Ϊ���������������������ķ�����ȫ���ڲ����ƣ�����Ҫ������
  HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);														// PB3/4/5�������
*/

// SPI����
	SPI3_FLASH.Instance = SPI3;
	SPI3_FLASH.Init.Direction = SPI_DIRECTION_2LINES;									// SPI����Ϊ˫��˫��ȫ˫��
	SPI3_FLASH.Init.Mode = SPI_MODE_MASTER;														// ����SPI����ģʽ:����Ϊ��SPI
	SPI3_FLASH.Init.DataSize = SPI_DATASIZE_8BIT;											// ����SPI�����ݴ�С:SPI���ͽ���8λ֡�ṹ
	SPI3_FLASH.Init.CLKPolarity = SPI_POLARITY_LOW;										// ѡ���˴���ʱ�ӵ���̬:ʱ�ӵ�
	SPI3_FLASH.Init.CLKPhase = SPI_PHASE_1EDGE;												// ���ݲ����ڵ�1��ʱ����
	SPI3_FLASH.Init.NSS = SPI_NSS_SOFT;																// NSS�ź���Ӳ����NSS�ܽţ����������ʹ��SSIλ������
	SPI3_FLASH.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;				// ���岨����Ԥ��Ƶ��ֵΪ4
	SPI3_FLASH.Init.FirstBit = SPI_FIRSTBIT_MSB;												// ָ�����ݴ����MSBλ����LSBλ��ʼ
	SPI3_FLASH.Init.CRCPolynomial = 7;																	// CRCֵ����Ķ���ʽ

	HAL_SPI_Init(&SPI3_FLASH);																					// ����SPI�ṹ���ʼ��������ʼ��SPI�ṹ��
	__HAL_SPI_ENABLE(&SPI3_FLASH);																			// ʹ��SPI2����
	
	spi3_flash_device();
//	spi3_flash_read((uint8_t *)&ddr, 0, 16);
}


// Connected to Data Flash
void WIZ_CS(uint8_t val)
{
	if (val == LOW) 
	{
		GPIO_ResetBits(GPIOA, WIZ_SCS); 	// GPIOA��WIZ_SCS��GPIO_PIN_4��������������
	}
	else if (val == HIGH)
	{
		GPIO_SetBits(GPIOA, WIZ_SCS); 		// GPIOA��WIZ_SCS��GPIO_PIN_4��������1����
	}
}


uint8_t SPI1_W5500_SendByte(uint8_t byte)
{
	while ((SPI1_W5500.Instance->SR & SPI_FLAG_TXE) == RESET);
	SPI1_W5500.Instance->DR = byte;
//	SPI_I2S_SendData(SPI2, byte);									   									// ����һ���ֽ�

	while ((SPI1_W5500.Instance->SR & SPI_FLAG_RXNE) == RESET); 	// �ȴ�������һ���ֽ�

//	return SPI_ReceiveData(SPI2);								   								// �����յ������� 
	return (SPI1_W5500.Instance->DR);								   								// �����յ������� 
}



