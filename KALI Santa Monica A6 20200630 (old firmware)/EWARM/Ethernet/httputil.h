#ifndef	__HTTPUTIL_H__
#define	__HTTPUTIL_H__

#include <stdio.h>
#include <string.h>
#include "w5500.h"
//#include "W5500_conf.h"
//#include "utility.h"
#include "ult.h"
//#include "bsp_spi_flash.h"
//#include "bsp_usart1.h"
#include "http_server.h"
#include "socket.h"


void proc_http(SOCKET s, unsigned char * buf);
void do_https(void);
void cgi_ipconfig(st_http_request *http_request);
//void trimp(uint8* src, uint8* dst, uint16 len);
uint16_t make_msg_response(uint8_t *buf, int8_t *msg);

void make_cgi_response(uint16_t a, int8_t *b,int8_t *c);
void make_pwd_response(int8_t isRight, uint16_t delay, int8_t *cgi_response_content, int8_t isTimeout);
#endif


