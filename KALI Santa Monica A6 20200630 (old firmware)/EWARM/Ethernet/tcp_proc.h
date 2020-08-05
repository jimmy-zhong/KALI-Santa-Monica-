#ifndef __TCP_PROC_H__
#define __TCP_PROC_H__


#define CMD_SPKCONTROL_GET										0
#define CMD_SPKCONTROL_SET										1
#define CMD_SPKDELAY_GET											2
#define CMD_SPKDELAY_SET											3
#define CMD_SPKDIM_GET												4
#define CMD_SPKDIM_SET												5
#define CMD_SPKEQPARAMS_GET										6
#define CMD_SPKEQPARAMS_SET										7
#define CMD_SPKLEDENABLE_GET									8
#define CMD_SPKLEDENABLE_SET									9
#define CMD_SPKLOCATE													10

#define CMD_SPKMUTE_GET												12
#define CMD_SPKMUTE_SET												13
#define CMD_SPKNAME_GET												14
#define CMD_SPKNAME_SET												15
#define CMD_SPKSTANDBYDLY_GET									16
#define CMD_SPKSTANDBYDLY_SET									17
#define CMD_SPKSTANDBYENABLE_GET							18
#define CMD_SPKSTANDBYENABLE_SET							19
#define CMD_SPKTRIM_INC												20

#define CMD_SPKTRIMLEVEL_GET									22
#define CMD_SPKTRIMLEVEL_SET									23
#define CMD_SPKPRESET_GET											24
#define CMD_SPKPRESET_SET											25
#define CMD_DISCONNECT												26
#define CMD_FIRMWARE_INFO											27

#define CMD_SPKTRIGGER												29

#define CMD_SPKEQENABLE_GET										31
#define CMD_SPKEQENABLE_SET										32
#define CMD_SPKDIPSWENABLE_GET								33
#define CMD_SPKDIPSWENABLE_SET								34

#define CMD_SPKFWSTART												100
#define CMD_FWDATA														101
#define CMD_SPKFWCOMPLETE											102

#define CMD_RES																200

#define CMD_ERROR253													253
#define CMD_ERROR254													254
#define CMD_UNKNOWN														255


void tcp_init(void);
void UDP_LISTEN(void);
void tcp_proc(void);
void sending_local_status(void);
uint8_t check_float_digits(char *pps);


#endif

