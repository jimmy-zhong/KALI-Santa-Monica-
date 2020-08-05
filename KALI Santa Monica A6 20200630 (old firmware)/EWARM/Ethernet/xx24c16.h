#ifndef __24c16_H
#define __24c16_H

#include "stm32f107xc.h"
//#include "types.h"

#define I2C_SCK	        GPIO_PIN_6  //out
#define I2C_SDA	        GPIO_PIN_7


#define GPIO_SetBits(GPIO_X, PIN_NO)						HAL_GPIO_WritePin(GPIO_X, PIN_NO, GPIO_PIN_SET)
#define GPIO_ResetBits(GPIO_X, PIN_NO)					HAL_GPIO_WritePin(GPIO_X, PIN_NO, GPIO_PIN_RESET)


void at24c16_init(void);
void at24c16_write(uint16_t addr, unsigned char val);
unsigned char at24c16_read(uint16_t addr);

void eep_block_write(uint16_t eepAddr, uint8_t *dat, uint16_t index, uint16_t len);

void erase_eeprom(uint16_t startAddr, uint16_t len);

#endif /* __MAIN_H */




