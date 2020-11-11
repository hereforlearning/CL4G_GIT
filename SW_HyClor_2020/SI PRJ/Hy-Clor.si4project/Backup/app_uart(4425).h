#ifndef EUSART_H
#define EUSART_H
#define NUM_RX_MAX 80//20

#define  u32 unsigned int 
#define  u8 unsigned char 

extern void EUSART_Init(void);
extern void EUSART_SendData(unsigned char ucLenth);
extern void EUSART_ReceiveData(unsigned char ucLenth);
extern void EUSART_DataHandler(void);
extern void BluetoothTest(void);
extern void IOKeepOnline(void);

extern unsigned char u8RX_BUFF[100];
extern unsigned char POWER_ON ;
extern unsigned char bTX_DONE;
extern unsigned char u8RX_RECIEVE_CNT;
extern unsigned char u8RX_BUFF_CNT;

extern unsigned char bTX_DONE;
extern unsigned char RX_CNT;
extern unsigned char RX_BUFF_CNT;
extern unsigned char BASE_CNT ;
extern unsigned char READY_SEND_DEGUE ;
extern unsigned char IOKeepOnlineCnt;
extern unsigned char IOTQueueKeyCnt;
unsigned char IOTRESTARTFlag;

extern unsigned char Status4G;
extern unsigned char StatusSIM;
extern unsigned char IOTRESTARTCNT;
extern unsigned char RX_BUFF[100];
extern unsigned char POWER_ON;
extern const unsigned char *AT_CMD[];

/*
open tcp udp at same time
udp  listen to device descovring cmd from  phone or sever at esp32 network index 1
TCP listern to control cmd or send heart package to phone or server   at esp32 network index 0
AT+CIPMODE=1
AT+CIPMUX=1
AT+CIPSEND=<network index>,<packge lenth>
AT+CIPSTART="TCP","192.168.0.140",1234  //tcp

*/
//AT+CIPSEND=0,3
//AT+CIPMODE=1
//AT+CWJAP="AIAQUA2.4","AIAQUA24"
/*
AT+CWMODE=1

OK
AT+CWJAP="AIAQUA2.4","AIAQUA24"
WIFI DISCONNECT
WIFI CONNECTED

busy p...
WIFI GOT IP

OK
AT+CIPSERVER=1,1234

ERROR
AT+CIPSTART="TCP","192.168.0.140",1234
CONNECT

OK
CLOSED
AT+CIPSTART="TCP","192.168.0.140",1234

+STA_CONNECTED:"6a:bf:4b:98:98:69"
+DIST_STA_IP:"6a:bf:4b:98:98:69","192.168.0.2"
0,CONNECT
+IPD,0,58:2a25bf2f6ac8+0f693c98+32+3F9501290C8F605E6EE36D3ADA0EBD53
*/
enum E_CONNECT_STATUE
{
   CONECT_NULL=0X00,
   CONECT_WLAN=0X01,
   CONECT_SERVER=0X02,
   CONECT_PHONE=0X04,
   REGESITER_FLAG=0x10,
};

enum  
{
   TCP_TYPE=0X00,
   UDP_TYPE=0X01,
};


#define UART_HEAD "+IPD:"
typedef struct S_TCP
{
	unsigned char HEAD[5];//+IPD,
	unsigned char TCP_ID;
	unsigned char unused;//,
	unsigned char TCP_LEN[2];
	unsigned char unused_1;//:
	unsigned char id[12];
	unsigned char unused_2;//+
	unsigned char crc[8];
	unsigned char unused_3;//+
	unsigned char len[2];
	unsigned char unused_4;//+
	unsigned char crypt_data[32];
}TS_TCP;

typedef struct S_TCP_DATA
{
	unsigned char id[12];
	unsigned char data[100];
}TS_TCP_DATA;

enum{
	empty,
	filled,
};

struct _UartQueue
{
	unsigned char status;
	unsigned char  DATABuff[NUM_RX_MAX];
};

enum EIOTWAITRESPONED
{
	WaitConnectPhoneResult,
};

#define WaitOverTime 3
enum _WAITRESULT
{
	OVER_TIME,
};

#define UART_QUEUE_LENTH 5
extern struct _UartQueue UartQueueBuff[UART_QUEUE_LENTH];
#define pEusartQueue0 (UartQueueBuff)
#define pEusartQueue2 (&UartQueueBuff[2])
#define pEusartQueue1 (&UartQueueBuff[1])
#define pEusartQueue3 (&UartQueueBuff[3])
#define pEusartQueue4 (&UartQueueBuff[4])

//extern unsigned char *EusartQueue[5];
extern struct	_UartQueue * pEusartQueueStorage;
extern struct	_UartQueue * pEusartQueuejxResolve;

unsigned char  IOTUartQueuePush(unsigned char *data);
void system_delay(unsigned int cnt);
void EUSART_WIFI_INIT(void);
void IotWifiSendString(unsigned char *str,unsigned char channel);
void UartSendString(unsigned char *text);
void IOTRESTARTIDLEFUN(void);
unsigned char IOTUartQueueioifempty();

#endif
