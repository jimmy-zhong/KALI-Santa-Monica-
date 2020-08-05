#ifndef __SPI2_H__
#define __SPI2_H__

#include "stm32f1xx_hal.h"

#define IINCHIP_ISR_DISABLE()
#define IINCHIP_ISR_ENABLE()	

#define FLASH_ERASE_CHIP		0x60
#define FLASH_ERASE_CHIP1		0xC7
#define FLASH_ERASE_4K			0x20
#define FLASH_ERASE_32K			0x52
#define FLASH_ERASE_64K			0xD8

#define FLASH_BLOCK_4K			(uint32_t)0x1000
#define FLASH_BLOCK_32K			(uint32_t)FLASH_BLOCK_4K * 8
#define FLASH_BLOCK_64K			(uint32_t)FLASH_BLOCK_4K * 16


#define __HAL_RCC_SPI2_CLK_ENABLE()   do { \
                                        __IO uint32_t tmpreg; \
                                        SET_BIT(RCC->APB1ENR, RCC_APB1ENR_SPI2EN);\
                                        /* Delay after an RCC peripheral clock enabling */\
                                        tmpreg = READ_BIT(RCC->APB1ENR, RCC_APB1ENR_SPI2EN);\
                                        UNUSED(tmpreg); \
                                      } while(0U)

void WIZ_SPI_Init(void);
void ADAU1452_SPI_Init(void);
void DeRegulate_ADAU1452_SPI(void);
void FLASH_SPI_Init(void);
void WIZ_CS(uint8_t val);
uint8_t SPI1_W5500_SendByte(uint8_t byte);
uint8_t spi3_flash_erase(uint8_t block_erase, uint16_t page);
void ClearMemorySpaceFor(uint32_t memsize);
uint8_t spi3_flash_read(uint8_t *read_buf, uint32_t address, uint16_t no);
uint8_t spi3_flash_write(uint8_t *write_buf, uint32_t address, uint16_t no);
void spi3_flash_device(void);
void spi3_flash_status1(void);
void spi3_flash_status2(void);
void spi3_flash_enwrite(void);
void spi3_flash_diswrite(void);


#endif

