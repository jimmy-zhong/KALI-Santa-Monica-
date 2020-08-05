#ifndef __ULT_H__
#define __ULT_H__


#include"stdio.h"
//#include "types.h"
#include "stm32f107xc.h"


void Systick_Init (uint8_t SYSCLK);
void Delay_s( uint32_t time_s );
void Delay_us(uint32_t time_us);
void Delay_ms(uint32_t time_ms);


uint16_t ATOI(char* str, uint16_t base); 			/* Convert a string to integer number */
uint32_t ATOI32(char* str, uint16_t base); 			/* Convert a string to integer number */
void itoa(uint16_t n, uint8_t *str, uint8_t len);
int ValidATOI(char *str, int base, int *ret); 		/* Verify character string and Convert it to (hexa-)decimal. */
char C2D(unsigned char c); 					/* Convert a character to HEX */

uint16_t swaps(uint16_t i);
uint32_t swapl(uint32_t l);

void replacetochar(char * str, char oldchar, char newchar);

void mid(int8_t *src, int8_t *s1, int8_t *s2, int8_t *sub);
void inet_addr_(unsigned char* addr,unsigned char *ip);
#endif
