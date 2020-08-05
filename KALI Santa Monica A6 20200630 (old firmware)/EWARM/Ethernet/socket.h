
#ifndef	__SOCKET_H__
#define	__SOCKET_H__

//#include "stm32f107xc.h"
#include "config.h"
//#include "Types.h"


/*Socket 端口选择，可按自己的习惯定义*/
/*#define SOCK_TCPS             0
#define SOCK_HUMTEM			  0
#define SOCK_PING			  0
#define SOCK_TCPC             1
#define SOCK_UDPS             2
#define SOCK_WEIBO      	  2
#define SOCK_DHCP             3
#define SOCK_HTTPS            4
#define SOCK_DNS              5
#define SOCK_SMTP             6
#define SOCK_NTP              7
*/


#define	MAX_SOCK_NUM		8	/**< Maxmium number of socket  */
typedef uint8_t				SOCKET;

extern uint8_t socket(SOCKET s, uint8_t protocol, uint16_t port, uint8_t flag); // Opens a socket(TCP or UDP or IP_RAW mode)
extern void sk_close(SOCKET s); // Close socket
extern uint8_t connect(SOCKET s, uint8_t *addr, uint16_t port); // Establish TCP connection (Active connection)
extern void disconnect(SOCKET s); // disconnect the connection
extern uint8_t listen(SOCKET s);	// Establish TCP connection (Passive connection)
extern uint16_t send(SOCKET s, const uint8_t *buf, uint16_t len); // Send data (TCP)
extern uint16_t recv(SOCKET s, uint8_t *buf, uint16_t len);	// Receive data (TCP)
extern uint16_t sendto(SOCKET s, const uint8_t *buf, uint16_t len, uint8_t *addr, uint16_t port); // Send data (UDP/IP RAW)
extern uint16_t recvfrom(SOCKET s, uint8_t *buf, uint16_t len, uint8_t *addr, uint16_t *port); // Receive data (UDP/IP RAW)

#ifdef __MACRAW__
void macraw_open(void);
uint16 macraw_send( const uint8_t *buf, uint16_t len ); //Send data (MACRAW)
uint16 macraw_recv( uint8_t *buf, uint16_t len ); //Recv data (MACRAW)
#endif

#endif
/* _SOCKET_H_ */

