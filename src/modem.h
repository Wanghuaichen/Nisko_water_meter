
#ifndef _MODEM_H
#define _MODEM_H


#define ON_TIME 		0x01
#define ABRUPT  		0x02
#define LOGGER			0x03
#define CONFIGURATION 	0x10
#define PROBE_REQUEST	0
#define UDP_TX_MODE		1




extern unsigned char connId;
extern unsigned long waitForFlag;
extern unsigned char TXMode;


void initTcpIp();
//void initUdp();
void Association(void);

unsigned char Send_Modem_Message(unsigned char state);
unsigned char Check_Modem_Message(unsigned char *buff,unsigned char state);
unsigned MessageChecksum(unsigned char *p,unsigned short len);
void sendData(unsigned char type,unsigned char sn,unsigned int data_len,unsigned long counter);
void FrameTX(unsigned char FrameID,unsigned int FrameNum,unsigned char FrameLen);



#endif
