#include <stdio.h>
#include <msp430.h>

#include "Serial.h"
#include "mnglpm.h"
#include "spiLcd.h"
#include "lcd_sigments.h"
#include "CP240x_def.h"
#include "modem.h"
#include "board.h"

unsigned char connId=0;
unsigned long waitForFlag=0;
unsigned char MeterStatus=0;
unsigned int  CustomerID=1234;
unsigned long MeterID=12345678;
unsigned int  Sends=0;
unsigned char units=0;
unsigned char TXMode;


/************************* Protocol***********************
Name			  Protocol
Input:			 
Called by:		  main()
Call to:		  ---
Returns:		  ---

Description:	  This builds  the communication protocol frames
				  
					
***********************************************************/

void FrameTX(unsigned char FrameID,unsigned int FrameNum,unsigned char FrameLen)
{
	int i=0;
	unsigned char frame[32];
	unsigned int checksum;
	//****************Header*************************************//
	frame[i++]=FrameID;
	frame[i++]=(unsigned char)(FrameNum>>8);
	frame[i++]=(unsigned char)(FrameNum);
	frame[i++]=(unsigned char)(FrameLen);
	
	//***********************************************************//

	switch (FrameID)
		{
		case ON_TIME:
			frame[i++]=MeterStatus;
			frame[i++]=(unsigned char)(uintToBcd(CustomerID)>>8);
			frame[i++]=(unsigned char)(uintToBcd(CustomerID));
			frame[i++]=(unsigned char)(ulongToBcd(MeterID)>>24);
			frame[i++]=(unsigned char)(ulongToBcd(MeterID)>>16);
			frame[i++]=(unsigned char)(ulongToBcd(MeterID)>>8);
			frame[i++]=(unsigned char)(MeterID);
			frame[i++]=(unsigned char)(adcBuffer.CWcounter>>24);
			frame[i++]=(unsigned char)(adcBuffer.CWcounter>>16);
			frame[i++]=(unsigned char)(adcBuffer.CWcounter>>8);
			frame[i++]=(unsigned char)(adcBuffer.CWcounter);
			frame[i++]=(unsigned char)(adcBuffer.CCWcounter>>24);
			frame[i++]=(unsigned char)(adcBuffer.CCWcounter>>16);
			frame[i++]=(unsigned char)(adcBuffer.CCWcounter>>8);
			frame[i++]=(unsigned char)(adcBuffer.CCWcounter);
			frame[i++]=units;
			frame[i++]=(unsigned char)(adcBuffer.flow>>8);
			frame[i++]=(unsigned char)(adcBuffer.flow);
			frame[i++]=0x0;										// Spare byte
			frame[i++]=0x0;										// Spare byte
			frame[i++]=(unsigned char)(Sends>>8);				//Beacon TX without Ack
			frame[i++]=(unsigned char)(Sends);				//Beacon TX without Ack
			frame[i++]=0x0;										// Spare byte
			frame[i++]=0x0;										// Spare byte
			frame[i++]=0x0;										// Spare byte
			frame[i++]=0x0;										// Spare byte
			checksum=MessageChecksum(frame,29);
			frame[i++]=(unsigned char)(checksum>>8);
			frame[i++]=(unsigned char)(checksum);
			break;

			
		case ABRUPT:
			break;

		case LOGGER:
			break;

		}


		if(TXMode==PROBE_REQUEST)
			{
			
				P1OUT |=BIT7; 						//turn on modem
				__delay_cycles(200*DCO_FRQ/1000);
				PrintString("AT+DGPIO=30,1\rAT+AVENDORIEENC=1,0,1122334455667788\rAT+WVENDORIESET=1,170,16,");
				for(i=0;i<32;i++)
					{
						SerialWrite(frame[i]);
					}
				
			//	PrintString(frame);
				PrintString("\rAT+WVENDORIESCAN=1\r");
				P1OUT&=~BIT7; 
				PrintString("AT+DGPIO=30,0\rAT+PSSTBY=50000,,1,1\r");
			}
		else if(TXMode==UDP_TX_MODE)
			{
				P1OUT |=BIT7; 						//turn on modem
				__delay_cycles(200*DCO_FRQ/1000);
				PrintString("AT+RESTORENWCONN\rAT+NSUDP=8010\r");
   				SerialWrite(0x1B);
				PrintString("U0192.168.1.101:8010:");
				for(i=0;i<32;i++)
					{
						SerialWrite(frame[i]);
					}
			//	PrintString("frame");
				SerialWrite(0x1B);
				SerialWrite('E');
				SerialWrite('\r');
  	
  				PrintString("AT+STORENWCONN\r");
 				P1OUT&=~BIT7; 	
   				PrintString("AT+PSSTBY=50000,,1,1\r");
			}
	
}



unsigned char sendCommand(unsigned char command,unsigned char message)
{
	if (waitForFlag<TIME_OUT_SEC(5))
	{
	
		if (!crCnt)
		{
			return 0;
		}
		if (RxCnt>2)	
		{
			unsigned char temp1;
			if(protocol_state==3&&RxCnt>4)
			{
				protocol_state=3;
			}
			readStringToBuf();
			temp1=Check_Modem_Message(inBuf,message);
			if(temp1==message)
			{
				waitForFlag=0;
				if (SerialRxReady())
					while (RxCnt)
						readStringToBuf(); //empty RxBuf
				return 1;
			}
		}
		else
		{
			while(RxCnt)
				readStringToBuf();
		}
		
	}
	else
	{
//		RF_DISP(ulpMem.ulp0.ulpBit.bit2^0x01);
//		CP240x_RegBlockWrite(ULPMEM00,(unsigned char *)&ulpMem,1);
		
		while (RxCnt)
				readStringToBuf(); //empty RxBuf
		if (TxCnt==0)
		{
			Send_Modem_Message(command);
		}
		waitForFlag=0;

	}

	return 0;

}

void initTcpIp()
{
	int i;
	switch (protocol_state)
	{
	case START:
		RF_DISP(OFF);
		SerialWrite('\r');
		protocol_state=DISCONNECT;
	case DISCONNECT:
		if (sendCommand(protocol_state,OK_MESSAGE))
		{
			protocol_state=ASSOCIATE;
			PrintString("ATE0");
			SerialWrite('\r');
		}
		break;
	case ASSOCIATE:
		if (sendCommand(protocol_state,SSID_MESSAGE))
		{
			protocol_state=OPEN_CLIENT_TCP;
		}
		break;
	case OPEN_CLIENT_TCP:
		if (sendCommand(protocol_state,CONNECT_MESSAGE))
			protocol_state=CONNECTED;
			sendSerial=1;
		break;
	case CONNECTED:
		RF_DISP(ON);
		CP240x_RegBlockWrite(ULPMEM00,(unsigned char *)&ulpMem,1);
		break;
	default:
		protocol_state=START;
		break;
	}
}

/*void initUdp()
{
	int i;
	switch (protocol_state)
	{
	case START:
		RF_DISP(OFF);
		SerialWrite('\r');
		protocol_state=DISCONNECT;
	case DISCONNECT:
		if (sendCommand(protocol_state,OK_MESSAGE))
		{
			protocol_state=ASSOCIATE;
			PrintString("ATE0");
			SerialWrite('\r');
		}
		break;
	case ASSOCIATE:
		if (sendCommand(protocol_state,SSID_MESSAGE))
		{
			protocol_state=OPEN_CLIENT_TCP;
		}
		break;
	case OPEN_CLIENT_TCP:
		if (sendCommand(protocol_state,CONNECT_MESSAGE))
			protocol_state=CONNECTED;
		break;
	case CONNECTED:
		RF_DISP(ON);
		CP240x_RegBlockWrite(ULPMEM00,(unsigned char *)&ulpMem,1);
		break;
	default:
		protocol_state=START;
		break;
	}
}*/

void Association(void)
{
	int i;
		PrintString("\rAT+WD\r");
		protocol_state=ASSOCIATE;
		P1OUT&=~BIT7;
		PrintString("AT+WA=HSPA_ROUTER\r");
		protocol_state=CONNECTED;
		PrintString("AT+STORENWCONN\rAT+PSSTBY=50000,,1,1\r");	//Store Network Configurations
		__delay_cycles(5*DCO_FRQ);
		
//		if (sendCommand(protocol_state,SSID_MESSAGE))
//			{
//				protocol_state=CONNECTED;
//				P1OUT&=~BIT7;
//				PrintString("AT+STORENWCONN\rAT+PSSTBY=50000\r");	//Store Network Configurations
//				__delay_cycles(40*DCO_FRQ/1000);
//			}
//		else
//		protocol_state=START;
}






/**
* @fn void sendData(unsigned char type,unsigned char sn,unsigned int data_len,unsigned long counter)
*
* @brief This function turn on lcd arrow segments acording to quad state and defined resulution "ARROW_RES"
*  
* @param - none
*
* @author Shlomi Dar
*
* @date 14.06.2011
*/


void sendData(unsigned char type,unsigned char sn,unsigned int data_len,unsigned long counter)
{

	unsigned char checksum;
	unsigned char dataBuf[5];
	unsigned char flowBuf[4];
	unsigned char flags;
	unsigned char units;
	unsigned int len=11;

	

	longToHex(adcBuffer.dispCounter,dataBuf);
	IntToHex(adcBuffer.dispFlow,flowBuf);


//	SerialWrite('1');
//	SerialWrite('2');

	//SerialWrite(0x1B);
	//SerialWrite(0x53);

	/*******heder********/
	//connection number
	//SerialWrite(connId);
	//packet ID type
//	SerialWrite(0xCC);
//	SerialWrite(type);
	//data len
	PrintHexDigit((unsigned char)(len>>8)&0x0F);
	PrintHexDigit((unsigned char)(len>>4)&0x0F);
	PrintHexDigit((unsigned char)(len&0x0F));
//	//unit serial number
	PrintHexDigit((unsigned char)(sn>>4)&0x0F);
	PrintHexDigit((unsigned char)(sn&0x0F));

	/********data************/
	
//	longToHex(adcBuffer.dispCounter,dataBuf);
//	IntToHex(adcBuffer.dispFlow,flowBuf);
//	//main counter
//	PrintHexLong(adcBuffer.dispCounter); //5 bytes
//	//Flow counter
//	PrintHexInt(adcBuffer.dispFlow); // 4 bytes
//	//flags
//	flags=0x30+(ulpMem.ulp0.all&0x0F);
//	SerialWrite(flags); // byte
//	//units
//	units=0x30+((ulpMem.ulp6.all>>1)&0x03);
//	SerialWrite(units); // byte
//
//	/*******checksum*********/
//	checksum=MessageChecksum(dataBuf,sizeof(dataBuf));
//	checksum+=MessageChecksum(flowBuf,sizeof(flowBuf));
//	checksum+=MessageChecksum(&flags,sizeof(flags));
//	checksum+=MessageChecksum(&units,sizeof(units));
//	
//	SerialWrite((unsigned char)((checksum>>4)&0x0F)+0x30);
//	SerialWrite((unsigned char)((checksum)&0x0F)+0x30);
//
//
//	//SerialWrite(0x1B);
//	//SerialWrite('E');
	SerialWrite('\r');

	
}


unsigned MessageChecksum(unsigned char *p,unsigned short len)
{
	unsigned short csum=0;
	unsigned char tmp=1;
	
	p+=8;
	while (len)
	{
		csum+=(*p )&0xFF;
		p++;
		len--;
	}
	return csum;
}

unsigned char Send_Modem_Message(unsigned char state)
{
	switch (state)
	{
	case DISCONNECT:
		PrintString("AT+WD\r");
		break;
	case ASSOCIATE:
		#ifdef DATA_COLLECTOR
		PrintString("AT+WA=nisko\r");////PrintString("AT+WA=nisko");//PrintString("AT+WA=HSPA_ROUTER");//PrintString("AT+WA=nisko");
		#else
		PrintString("AT+WA=HSPA_ROUTER\r");
		#endif
		break;
//	case OPEN_CLIENT_TCP:
//		#ifdef ADHOC
//		PrintString("AT+NCTCP=192.168.1.50,8010\r");
//		#else
//		PrintString("AT+NCTCP=192.168.1.100,45520\r");//PrintString("AT+NCTCP=192.168.1.51,8010");//PrintString("AT+NCTCP=77.126.57.197,45520");//PrintString("AT+NCTCP=192.168.1.100,8020");
//		#endif
//		break;
//	case SEND_DATA:
//		PrintString("/x1BS0Hallo world/x1BE\r");
//		break;
//	case DISABLE_DHCP:
//		PrintString("AT+NDHCP=0\r");
//		break;
//	case SET_STATIC_IP:
//		PrintString("AT+NSET=192.168.1.51,255.255.255.0,192.168.1.1\r");
//		break;
//	case SAVE_PROFILE_0:
//		PrintString("AT&W0\r");
//		break;
//	case SET_SCANTIME:		
//		PrintString("AT+WST=6000,6000\r");
//		break;
//	case GET_SCANTIME:       // Probe Responce Scan time
//		PrintString("AT+WST=?\r");
//		break;
//	case ENCRYPTION:				//Set this command for encryption algorithm on GS1011A and GS1011B
//		PrintString("AT+AVENDORIEENC=1,0,1122334455667788\r");
//		break;
//	case ENABLE_IE_TX:		//Set this command for data TX on  GS1011A.   AT+WVENDORIESET=<Enable/Disable> [, <Vendor IE ID>, <length>, <Data>]
//		PrintString("AT+WVENDORIESET=1,170,6,113355112233\r");
//		break;
//	case ENABLE_IE_RX:		//Set this command for data RX on  GS1011B.   
//		PrintString("AT+WVENDORIESET=1,170\r");
//		break;
//	case SEND_PROBE_RESP:		//Set this command for probe responce on  GS1011B.   AT+WVENDORIERSP=<No Probe Response/Send Probe Response>  [, <MAC>, <Rate>, <Vendor IE ID>, Length[, <Data>]]
//		PrintString("AT+WVENDORIERSP=1,00:1d:c9:d0:08:ac,2,170,6,998877665544\r");
//		break;
//	case SEND_PROBE_REQ:		//Set this command for sending Probe Request.   AT+WVENDORIESCAN=<Channel>
//		PrintString("AT+WVENDORIESCAN=1\r");
//		break;
//	case RADIO_ACTIVE:
//		PrintString("AT+WRXACTIVE=1\r"); //Enable/Disable Radio
//		break;
//	case POWER_SAVE:
//		PrintString("AT+WRXPS=1\r");	//Enable/Disable Power save mode
//		break;
//	case POWER_LEVEL:
//		PrintString("AT+WP=2\r");	//Set TX Power Level
//		break;
//	case STATION_MODE:
//		PrintString("AT+WM=0\r");	//Set Station Operating Mode. 	0-Infrastructure, 1-Ad-hoc, 2-Limited AP
//		break;
//	case STORE_NWCON:
//		PrintString("AT+STORENWCONN\r");	//Store Network Configurations
//		break;
//	case RESTORE_NWCON:
//		PrintString("AT+RESTORENWCONN\r");	//Retore Network Configurations
//		break;
//	case BCHECK_FREQ:
//		PrintString("AT+BCHKSTRT=10\r");	//Battery Check Frequency- every 0<n<=100 packets transmitted
//		break;
//	case BWARNING_LEVEL:
//		PrintString("AT+BATTLVLSET=3300,10,3000\r");	//Battery Warning?Standby frequency.  AT+ BATTLVLSET=<Warning Level>,<Warning Freq>,<Standby Level>
//		break;
//	case BCHECK_RESET:
//		PrintString("AT+BCHK=10\r");	//Battery Check Frequency RESET
//		break;
//	case BCHECK_STOP:
//		PrintString("AT+BCHKSTOP\r");	//Battery Check STOP
//		break;
//	case BVAL_GET:
//		PrintString("AT+BATTVALGET\r");	//Get Battery Value
//		break;
//	case DEEPSLEEP:
//		PrintString("AT+PSDPSLEEP\r");	// Enter Deep sleep mode
//		break;
//	case STANDBY:
//		PrintString("AT+PSSTBY=50000,,1,1\r");	//Enter Standby Mode. 	AT+PSSTBY=x[,<DELAY TIME>,<ALARM1 POL>,<ALARM2 POL>]
//		break;
	default:
		break;
		
	}	
}

unsigned char Check_Modem_Message(unsigned char *buff,unsigned char state)
{
	unsigned char i1,i2;
	unsigned char *Ptr1;
	//unsigned char temp;

	i2=ERR_MESSAGE; 

	switch (state)
	{
		case OK_MESSAGE:
			for(i1=0;i1<10;i1++)
			{
				Ptr1=buff+i1;
				if(*Ptr1=='O' && *(Ptr1+1)=='K' )
				{
					i2=OK_MESSAGE;
					break;
				}
			}
			break;
		case AT_MESSAGE:
			for(i1=0;i1<10;i1++)
			{
				Ptr1=buff+i1;
				if(*Ptr1=='A' && *(Ptr1+1)=='T' )
				{
					i2=AT_MESSAGE;
					break;
				}
			}
			break;
		case CONNECT_MESSAGE:
			for(i1=0;i1<10;i1++)
			{
				Ptr1=buff+i1;
				if((*Ptr1=='C' && *(Ptr1+1)=='O'&& *(Ptr1+2)=='N'&& *(Ptr1+3)=='N'&& *(Ptr1+4)=='E'&& *(Ptr1+5)=='C' && *(Ptr1+6)=='T') )
				{
					connId=*(Ptr1+8);
					i2=CONNECT_MESSAGE;
					break;
				}
			}
			break;
		case SSID_MESSAGE:
			for(i1=0;i1<10;i1++)
			{
				Ptr1=buff+i1;
				if((*Ptr1==' ' && *(Ptr1+1)==' '&& *(Ptr1+2)=='I'&& *(Ptr1+3)=='P'&& *(Ptr1+4)==' ' ) )
				{
					for(i1=10;i1<26;i1++)
					{
						Ptr1=buff+i1;
						if((*Ptr1==' ' && *(Ptr1+1)=='S'&& *(Ptr1+2)=='u'&& *(Ptr1+3)=='b'&& *(Ptr1+4)=='N' && *(Ptr1+5)=='e'&& *(Ptr1+6)=='t') )
						{
							i2=SSID_MESSAGE;
							break;
						}
					}
				}
			}
			break;
		case ERR_MESSAGE:
			for(i1=0;i1<10;i1++)
			{
				Ptr1=buff+i1;
				if(*Ptr1=='E'&& *(Ptr1+1)=='R'&& *(Ptr1+2)=='R'&& *(Ptr1+3)=='O'&& *(Ptr1+4)=='R')
				{
					i2=ERR_MESSAGE;
					break;
				}
			}
			break;
		case IP_MESSAGE:
			for(i1=0;i1<5;i1++)
			{
				Ptr1=buff+i1;
				if(*Ptr1=='.')
				{
					for(i1=5;i1<10;i1++)
					{
						Ptr1=buff+i1;
						if(*Ptr1=='.')
						{
							i2=IP_MESSAGE;
						}
					}
				}
			}
		case K_MESSAGE:
			for(i1=0;i1<10;i1++)
			{
				Ptr1=buff+i1;
				if(*Ptr1=='K' && *(Ptr1+1)=='\r')
				{
					i2=OK_MESSAGE;
					break;
				}
			}

			break;
			
		}
return i2;
}

