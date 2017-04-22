#ifndef __SERIAL_H
#define __SERIAL_H

#define USE_SERIAL
#define BAUD_RATE 9600
#define BAUD_DIV (DCO_FRQ/BAUD_RATE)
#define SERIAL_CH 0
#define SERIAL_MOD 0
#define COUNTER_MASSAGE_LEN 4


#define	TX_BUF_SIZE_0	32   
#define	RX_BUF_SIZE_0	32   
 
#define ERR_MESSAGE		0
#define OK_MESSAGE		1
#define AT_MESSAGE		2
#define CONNECT_MESSAGE	3

#define SSID_MESSAGE	5
#define IP_MESSAGE		6
#define K_MESSAGE		7



#define START 0
#define DISCONNECT 1
#define ASSOCIATE 2
#define OPEN_CLIENT_TCP 3
#define SEND_DATA 4
#define DISABLE_DHCP 5
#define SET_STATIC_IP 6
#define SAVE_PROFILE_0 7
#define SET_SCANTIME 8
#define GET_SCANTIME 9
#define ENCRYPTION	10
#define ENABLE_IE_TX 11
#define ENABLE_IE_RX 12
#define SEND_PROBE_RESP 13
#define SEND_PROBE_REQ 14
#define RADIO_ACTIVE 15
#define POWER_SAVE 16
#define POWER_LEVEL 17
#define STATION_MODE 18
#define STORE_NWCON 19
#define RESTORE_NWCON 20
#define BCHECK_FREQ 21
#define	BWARNING_LEVEL 22
#define	BCHECK_RESET 23
#define	BCHECK_STOP 24
#define	BVAL_GET 25
#define	DEEPSLEEP 26
#define	STANDBY 27



#define CONNECTED 0x55


#define TIME_OUT_SEC(x) (x) // virtual timer 2




void SerialInit(unsigned short baud_div);

void SerialWrite(unsigned char ch);
unsigned char SerialTxIsr(void);

unsigned char SerialRxIsr(void);
//unsigned short SerialRead(void);

void PrintString(char *s);
unsigned char SerialRxReady(void);



unsigned char readStringToBuf();
void PrintHexDigit(unsigned char digit);
void PrintHexLong(unsigned long val);
void PrintHexInt(unsigned int val);
void longToHex(unsigned int val,unsigned char *data);
void IntToHex(unsigned int val,unsigned char *data);





extern unsigned char TxBuf[TX_BUF_SIZE_0];
extern unsigned short RxBuf[RX_BUF_SIZE_0];
extern unsigned char inBuf[RX_BUF_SIZE_0];

extern unsigned char TxCnt;
extern unsigned char TxInIdx;
extern unsigned char TxOutIdx;
extern unsigned char RxCnt;
extern unsigned char RxInIdx;
extern unsigned char RxOutIdx;
extern unsigned char RxInProgress;
extern unsigned char TxErr;
extern unsigned char crCnt;
extern unsigned char protocol_state;

#endif
