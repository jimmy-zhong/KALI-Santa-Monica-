#ifndef __SIGMASTUDIOFW_H__
#define __SIGMASTUDIOFW_H__

#include <stdint.h>


typedef uint8_t ADI_REG_TYPE;
//typedef const uint8_t ADI_REG_TYPE;

void SIGMA_WRITE_REGISTER_BLOCK(uint8_t dev_address, uint16_t CMD, uint16_t Num, uint8_t *data);
void SIGMA_WRITE_REGISTER_UINT32(uint8_t dev_address, uint16_t CMD, uint16_t Num, uint8_t *data);
void SIGMA_WRITE_DELAY(uint8_t dev_address, uint16_t Num, uint8_t *data);

#endif
