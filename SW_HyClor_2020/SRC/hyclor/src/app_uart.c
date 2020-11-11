/******************************************************************************
* Copyright (C) 2017, Huada Semiconductor Co.,Ltd All rights reserved.
*
* This software is owned and published by:
* Huada Semiconductor Co.,Ltd ("HDSC").
*
* BY DOWNLOADING, INSTALLING OR USING THIS SOFTWARE, YOU AGREE TO BE BOUND
* BY ALL THE TERMS AND CONDITIONS OF THIS AGREEMENT.
*
* This software contains source code for use with HDSC
* components. This software is licensed by HDSC to be adapted only
* for use in systems utilizing HDSC components. HDSC shall not be
* responsible for misuse or illegal use of this software for devices not
* supported herein. HDSC is providing this software "AS IS" and will
* not be responsible for issues arising from incorrect user implementation
* of the software.
*
* Disclaimer:
* HDSC MAKES NO WARRANTY, EXPRESS OR IMPLIED, ARISING BY LAW OR OTHERWISE,
* REGARDING THE SOFTWARE (INCLUDING ANY ACOOMPANYING WRITTEN MATERIALS),
* ITS PERFORMANCE OR SUITABILITY FOR YOUR INTENDED USE, INCLUDING,
* WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, THE IMPLIED
* WARRANTY OF FITNESS FOR A PARTICULAR PURPOSE OR USE, AND THE IMPLIED
* WARRANTY OF NONINFRINGEMENT.
* HDSC SHALL HAVE NO LIABILITY (WHETHER IN CONTRACT, WARRANTY, TORT,
* NEGLIGENCE OR OTHERWISE) FOR ANY DAMAGES WHATSOEVER (INCLUDING, WITHOUT
* LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION,
* LOSS OF BUSINESS INFORMATION, OR OTHER PECUNIARY LOSS) ARISING FROM USE OR
* INABILITY TO USE THE SOFTWARE, INCLUDING, WITHOUT LIMITATION, ANY DIRECT,
* INDIRECT, INCIDENTAL, SPECIAL OR CONSEQUENTIAL DAMAGES OR LOSS OF DATA,
* SAVINGS OR PROFITS,
* EVEN IF Disclaimer HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
* YOU ASSUME ALL RESPONSIBILITIES FOR SELECTION OF THE SOFTWARE TO ACHIEVE YOUR
* INTENDED RESULTS, AND FOR THE INSTALLATION OF, USE OF, AND RESULTS OBTAINED
* FROM, THE SOFTWARE.
*
* This software may be replicated in part or whole for the licensed use,
* with the restriction that this Disclaimer and Copyright notice must be
* included with each copy of this software, whether used in part or whole,
* at all times.
*/
/******************************************************************************/
/** \file main.c
 **
 ** A detailed description is available at
 ** @link Sample Group Some description @endlink
 **
 **   - 2017-05-17  1.0  cj First version for Device Driver Library of Module.
 **
 ******************************************************************************/

/******************************************************************************
 * Include files
 ******************************************************************************/
#include "ddl.h"
#include "uart.h"
#include "gpio.h"
#include "sysctrl.h"

/******************************************************************************
 * Local pre-processor symbols/macros ('#define')                            
 ******************************************************************************/

/******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/******************************************************************************
 * Local type definitions ('typedef')                                         
 ******************************************************************************/

/******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/

/******************************************************************************
 * Local variable definitions ('static')                                      *
 ******************************************************************************/

/******************************************************************************
 * Local pre-processor symbols/macros ('#define')                             
 ******************************************************************************/
#define     T1_PORT                 (3)
#define     T1_PIN                  (3)
/*****************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/
/**
 ******************************************************************************
 ** \brief  Main function of project
 **
 ** \return uint32_t return value, if needed
 **
 ** This sample
 **
 ******************************************************************************/
uint8_t u8TxData[44] = {0x00,0x55};
uint8_t u8RxData;
uint8_t u8TxCnt=0,u8RxCnt=0;
uint8_t tx_send;
uint8_t rx_reci;
void TxIntCallback(void)
{
    u8TxCnt++;
   // if(u8TxCnt<=1)
	//    M0P_UART1->SBUF = u8TxData[1];
    tx_send=1;
    Uart_ClrStatus(UARTCH1,UartTC);    
}
void RxIntCallback(void)
{
    u8RxData=Uart_ReceiveData(UARTCH1);
	if(u8RxData==NULL)
		return;
    //u8RxCnt++;
	if(u8RxCnt<44){
		u8RxCnt++;
		u8TxData[u8RxCnt]=u8RxData; }	
	else
		u8RxCnt=0;
Uart_ClrStatus(UARTCH1,UartRC);
}
void ErrIntCallback(void)
{
  
}
void PErrIntCallBack(void)
{
}
void CtsIntCallBack(void)
{
}
void Uart_PortInit(void)
{
    stc_gpio_config_t stcGpioCfg;
    DDL_ZERO_STRUCT(stcGpioCfg);
    stcGpioCfg.enDir = GpioDirOut;
    Gpio_Init(GpioPortA,GpioPin2,&stcGpioCfg);
    Gpio_SetAfMode(GpioPortA,GpioPin2,GpioAf1);//TX
    stcGpioCfg.enDir = GpioDirIn;
    Gpio_Init(GpioPortA,GpioPin3,&stcGpioCfg);
    Gpio_SetAfMode(GpioPortA,GpioPin3,GpioAf1);//RX
}

void Init_APP_Uart(void)
{  
	  unsigned int u16Scnt=0;
    stc_uart_config_t  stcConfig;
    stc_uart_irq_cb_t stcUartIrqCb;
    stc_uart_multimode_t stcMulti;
    stc_uart_baud_t stcBaud;
    en_uart_mmdorck_t enTb8;
    DDL_ZERO_STRUCT(stcConfig);
    DDL_ZERO_STRUCT(stcUartIrqCb);
    DDL_ZERO_STRUCT(stcMulti);
    DDL_ZERO_STRUCT(stcBaud);
    
    Sysctrl_SetPeripheralGate(SysctrlPeripheralGpio,TRUE);
    Sysctrl_SetPeripheralGate(SysctrlPeripheralDma,TRUE);
    Sysctrl_SetPeripheralGate(SysctrlPeripheralUart1,TRUE);
    Uart_PortInit();
    
    stcUartIrqCb.pfnRxIrqCb   = RxIntCallback;
    stcUartIrqCb.pfnTxIrqCb   = TxIntCallback;
    stcUartIrqCb.pfnRxFEIrqCb = ErrIntCallback;
    stcUartIrqCb.pfnPEIrqCb   = PErrIntCallBack;
    stcUartIrqCb.pfnCtsIrqCb  = CtsIntCallBack;
    stcConfig.pstcIrqCb = &stcUartIrqCb;
    stcConfig.bTouchNvic = TRUE;
  

    stcConfig.enRunMode = UartMode1;//模式3
    stcConfig.enStopBit = Uart1bit;  

    stcMulti.enMulti_mode = UartNormal;//正常工作模式
    enTb8 = UartEven;//偶校验
    Uart_SetMMDOrCk(UARTCH1,enTb8);
    stcConfig.pstcMultiMode = &stcMulti;
    
    Uart_Init(UARTCH1, &stcConfig);
    
    Uart_SetClkDiv(UARTCH1,Uart8Or16Div);
    stcBaud.u32Pclk = Sysctrl_GetPClkFreq();
    stcBaud.enRunMode = UartMode3;
    stcBaud.u32Baud = 9600;
    u16Scnt = Uart_CalScnt(UARTCH1,&stcBaud);
    Uart_SetBaud(UARTCH1,u16Scnt);
    
    Uart_EnableIrq(UARTCH1,UartRxIrq);
    Uart_ClrStatus(UARTCH1,UartRC);
    Uart_EnableFunc(UARTCH1,UartRx);
	
    #if 0
    while(1)
    {
		unsigned char i=0;
		unsigned char *text="sdfaslkfjeroiquerqieruweoip";
		//if(u8RxCnt>=1)
        if(1){
           Uart_DisableIrq(UARTCH1,UartRxIrq);
           Uart_EnableIrq(UARTCH1,UartTxIrq);
					 u8RxCnt = 0;
					for(i=0;i<33;i++)
					{
 			  	
						tx_send=0;
            M0P_UART1->SBUF ='r';// u8RxData;   
					  while(!tx_send);
					}
				}
				
				#if 0
        else if(u8TxCnt>1)
        {
            u8TxCnt = 0;
            Uart_ClrStatus(UARTCH1,UartTC);   
            Uart_DisableIrq(UARTCH1,UartTxIrq);
            Uart_EnableIrq(UARTCH1,UartRxIrq);
        }
				#endif
		 //  Uart_DisableIrq(UARTCH1,UartTxIrq);
		//    Uart_EnableIrq(UARTCH1,UartRxIrq);
				delay1ms(1000);
    }
	#endif
}

void UART_PROCESS(void)
{


}

/***********************************************************************/
#define QueueLen 3
unsigned char IOTQueueKeyCnt=0xff;
unsigned char IOTQueueKey[QueueLen];
unsigned char IOTQueueKeyLen=0;
unsigned char  IOTQueueKeyIsEmppt()
{
	return IOTQueueKeyLen==0;
}

unsigned char  IOTQueueKeyPop()
{
	unsigned char  temp;
	if(IOTQueueKeyLen>0)
		IOTQueueKeyLen--;
	temp=IOTQueueKey[IOTQueueKeyLen];
	if(IOTQueueKeyIsEmppt()==1)
		IOTQueueKeyCnt=0xff;
	return temp;
	
}

void IOTQueueKeyPush(unsigned char key){
unsigned char temp;
	if(IOTQueueKeyLen>=QueueLen)
		IOTQueueKeyLen=QueueLen-1;
	for(temp=IOTQueueKeyLen;temp>0;temp--)
	{
		IOTQueueKey[temp]=IOTQueueKey[temp-1];
	}
	IOTQueueKey[0]=key;
	if(IOTQueueKeyLen<QueueLen-1)
		IOTQueueKeyLen++;
	if(IOTQueueKeyCnt==0xff)
		IOTQueueKeyCnt=0;
}

void IOTQueueKeyHandle()
{
	if(IOTQueueKeyCnt>=15)
	{
		if(IOTQueueKeyIsEmppt()){
			IOTQueueKeyCnt=0xff;
			return;
		}
		else
			IOTQueueKeyCnt=0;
		RESIVE_UAER_KEY=IOTQueueKeyPop();
	}
}
/***********************************************************************/
unsigned char Status4G=0;
unsigned char StatusSIM=0xff;
unsigned char IOKeepOnlineCnt=0;
unsigned char IOTRESTARTCNT=0xff;
unsigned char IOTRESTARTSTEP=0;
unsigned char IOTRESTARTFlag=0;
void IOTRESTARTHANDLEFUN()
{
	#define IOTRESTARTIDLEENTERTIME 30
	#define IOTRESTARTIDLEENDTIME 40
	if(IOTRESTARTFlag==1)
	{
		IOTRESTARTFlag=0;
		if(IOTRESTARTCNT!=0xff){
            if(IOTRESTARTCNT<=0xff)
				IOTRESTARTCNT++;
			if(IOTRESTARTCNT<7){
				system_delay(5000);
				UartSendString(AT_CMD[IOTRESTARTSTEP]);
				system_delay(5000);
				IOTRESTARTSTEP++;
				if(IOTRESTARTSTEP>=6){
					IOTRESTARTSTEP=0;
					IOTRESTARTCNT=IOTRESTARTIDLEENTERTIME;
				}
			}
			if(IOTRESTARTCNT>=IOTRESTARTIDLEENDTIME){
//				display(0, 0, "iot edle end here wawo", 20);
//				system_delay(5000);
				IOTRESTARTCNT=0;}

		}
	}
}

void IOTRESTARTSTARTFUN(void)
{
	if(IOTRESTARTCNT==0xff){
		IOTRESTARTSTEP=0;
		IOTRESTARTCNT=0;
		IOTRESTARTFlag=0;
	}
}

void IOTRESTARTIDLEFUN(void)
{
	if(IOTRESTARTCNT==0xff){
		IOTRESTARTSTEP=0;
		IOTRESTARTCNT=0;
		IOTRESTARTFlag=0;
	}
}


void IOTRESTARTSTOPFUN()
{
	IOTRESTARTSTEP=0;
	IOTRESTARTCNT=0xff;
	IOTRESTARTFlag=0;
}

void IOKeepOnline(void)
{
//	if(IOTQueueKeyIsEmppt){
	if(IOTUartQueueioifempty){
		if(IOKeepOnlineCnt>=2)
		{
			IOKeepOnlineCnt=0;
			if(POWER_ON)
			{
				UartSendString("AT+ADA=\"s\"\,\"FD\"\,\"i\"\,1\x0d\x0a");
			}
			else
			{
				UartSendString("AT+ADA=\"s\"\,\"FD\"\,\"i\"\,0\x0d\x0a");
			}
			system_delay(1000);
		    UartSendString("AT+ADA=\"l\"\x0d\x0a");
		}
	}
}
/***********************************************************************/
struct _UartQueue UartQueueBuff[UART_QUEUE_LENTH];
//struct _UartQueue *EusartQueue[5]={pEusartQueue0,pEusartQueue1,pEusartQueue2,pEusartQueue3,pEusartQueue4,
pEusartQueue5};
struct  _UartQueue * pEusartQueueStorage;
struct  _UartQueue * pEusartQueueResolve;
void IOTUartQueueInit()
{
	pEusartQueueStorage=UartQueueBuff;
	pEusartQueueResolve=UartQueueBuff;
}

unsigned char  IOTUartQueuePush(unsigned char *data)
{
	if((pEusartQueueStorage!=pEusartQueueStorage)||((pEusartQueueStorage==pEusartQueueStorage)&&(pEusartQueueStorage->
status==empty)))
	{
		memcpy((pEusartQueueStorage->DATABuff),data,NUM_RX_MAX);
		memset(RX_BUFF,0,NUM_RX_MAX);
		pEusartQueueStorage->status=filled;
		if(pEusartQueueStorage==&UartQueueBuff[4])
			pEusartQueueStorage=UartQueueBuff;
		else
			pEusartQueueStorage++;
		return 1;
	}
    memset(RX_BUFF,0,NUM_RX_MAX);
	return 0;
}

void IOTUartQueuePop(unsigned char *data)
{
	struct	_UartQueue *q=pEusartQueueResolve;
	memcpy(data,&pEusartQueueResolve->DATABuff[2],NUM_RX_MAX-2);
	pEusartQueueResolve->status=empty;
	if(pEusartQueueResolve==&UartQueueBuff[4])
		pEusartQueueResolve=UartQueueBuff;
	else
		pEusartQueueResolve++;
	
}

unsigned char IOTUartQueueioifempty()
{
//	struct  _UartQueue * next;
//	if(pEusartQueuejxResolve!=UartQueueBuff[4])
//		pEusartQueuejxResolve=UartQueueBuff;
//	else
//		pEusartQueuejxResolve=UartQueueBuff++;
	if(pEusartQueueResolve->status==empty)
	{
		return 1;
	}
	return 0;
}

/***********************************************************************/
//aura 4g borad 
//+ADA: "g","ok","FD","i",22
struct Aura4GRec
{
	unsigned char resever[5];//+ADA:"
	unsigned char SendOrGet;
	unsigned char resever1[8];//","ok","
	unsigned char RecString[2];//
	unsigned char resever2[3];//","
	unsigned char RecType;//
	unsigned char resever3[2];//",
	unsigned char RecData[5];
};
/***********************************************************************/

unsigned char  IotwifiWaitStatus;
struct ST_TCP
{
    unsigned char num;
    unsigned char len;
    unsigned char data[100];
}TCP;
//+IPD,0,58:2a25bf2f6ac8+0f693c98+32+3F9501290C8F605E6EE36D3ADA0EBD53
struct ST_PACKAGE_HEAD
{
	unsigned char head[5];//+IPD,
	unsigned char package_type;//0
	unsigned char unused_1;//,
	unsigned char len[2];
	unsigned char unused_2;//:
};

struct _ST_TCP
{
	unsigned char ControlDeviceID[12];
	unsigned char unused_3;//+
	unsigned char crc[8];
	unsigned char unused_4;//+
	unsigned char len[2];
	unsigned char unused_5;//+
	unsigned char EncrytData[40];//+
};

//481c55e223:aabbccddeeff:192.168.0.166:1234
struct _ST_UDP
{
//	unsigned char eUDPSignalType;
	union
	{
		struct{
			unsigned char Destaion_ID[10];
			unsigned char reserve1;
			unsigned char SourceId[10];
			unsigned char reserve2;
			unsigned char SourceIp[13];//15
			unsigned char reserce3;
			unsigned char SourcePort[4];
		}FindConnected;
		struct{
			unsigned char rs_2;
		}StartNewConnect;
	};
};



struct ST_PACKAGE
{
	struct ST_PACKAGE_HEAD PackageHead;
	union {
		struct _ST_TCP RawTCP;
		struct _ST_UDP RawUDP;
	};
};

struct ST_STRUCT_PACKAGE
{
	unsigned char *ppackage;
	unsigned char package_type;
};

struct ST_NETWORK_STATUE
{
	unsigned char ConnectStatue;
	unsigned char RemoteDeviceID[12];
	unsigned int ReomtePort;
	unsigned char DeviceID[12];
}NETWORK_STATUE;

unsigned char RESIVE_UAER_KEY;
unsigned char KEY_USART_MAP[]={
pmkey,
cbkey,//oxi
wmkey ,
leftkey ,
upkey,
okkey ,
downkey ,
rightkey ,
ctkey ,
bwkey ,
stkey,
0xff,
LRKey,	//Wingo added for left+right key
};


unsigned char WifiReciveString;
unsigned char RX_BUFF[100];
unsigned char RX_[NUM_RX_MAX];
unsigned char ReciveString = 0;
unsigned char POWER_ON = 1;
unsigned char TX_DONE;
unsigned char RX_DONE;
unsigned char RX_DATA;
unsigned char RX_CNT;
unsigned char RX_BUFF_CNT;
unsigned char BASE_CNT=0;
unsigned char READY_SEND_DEGUE=0;

void system_delay(unsigned int cnt)
{
    unsigned char i = 0;
    unsigned int j = 0;
    for (i = 0; i < 100; i++)
    {
        j = cnt;
        while (j--);
    }
}

#if(UART_CONNECT_SELECT != NONE)
const unsigned char *AT_CMD[] =
#if 1//4G
{
	"AT+NETOPEN\x0d\x0a",
	"at+capnet=6\,1\x0d\x0a",
	"at+ada=\"start\"\x0d\x0a",
	"AT+ADA=\"enable\"\x0d\x0a",
	"AT+ADA=\"l\"\x0d\x0a",
	"AT+ADA=\"l\"\x0d\x0a",
};
#else //ESP32
{
    "AT+RST\x0d\x0a",
	"AT+CWMODE=1\x0d\x0a",
//    "AT+CWMODE=3\x0d\x0a",
//    "AT+CWSAP=\"ESP32\"\,\"1234567890\"\,5\,3\x0d\x0a",
//    "AT+CIPAP=\"192.168.0.1\"\,\"192.168.0.1\"\,\"255.255.255.0\"\x0d\x0a",
	"AT+CWJAP=\"AIAQUA2\.4\"\,\"AIAQUA24\"\x0d\x0a",
    "AT+CIPMUX=1\x0d\x0a",
//    "AT+CIPSERVER=1\,1234\x0d\x0a",
	"AT+CIPSTART=1\,\"UDP\"\,\"192.168.0.255\"\,13333\,12345\,2\x0d\x0a",
	"AT+CIFSR\x0d\x0a",
};
#endif


void UartSendString(unsigned char *text)
{
    unsigned char i = 0, j;
    unsigned char ucFontBuf[16];
    while ((*text > 0x00))
    {
        EUSART_SendData(*text);
        text++;
    }
}

extern unsigned char TX_DONE;

void EUSART_SendData(unsigned char data)//unsigned char ucLenth)
{
}

void EUSART_ReceiveData(unsigned char data)//(unsigned char ucLenth)
{
}

void EUSART_WIFI_INIT(void)
{
    char i = 0;
    unsigned int J = 0;
    unsigned int J1 = 0;
    signed	char wait_ok = 0;
    unsigned char delay = 0;
    unsigned char *p = NULL;
    //goto LL;
	RX_CNT=0xff;
	IOTUartQueueInit();
    for (i = 0; i < 6; i++)//(sizeof(AT_CMD)/sizeof(unsigned char *))
    {
        ReciveString = 0;
        UartSendString((unsigned char *)AT_CMD[i]);
        wait_ok = 0;
        delay = 0;
        for (J1 = 0; J1 < 2000; J1++)
            for (J = 0; J < 2000; J++);
    }
	 UartSendString("AT+ADA=\"s\"\,\"FD\"\,\"i\"\,1\x0d\x0a");
	 IOTRESTARTIDLEFUN();
	 //send power on signal
}

static u32 CRC32[256];
//初始化表,在单片机等RAM较小的系�?可以在源码中把tab构造为const数组
static void init_table() {
   for (int i = 0; i < 256; i++) {
      u32 crc = i;
      for (int j = 0; j < 8; j++) {
         if (crc & 1) {
            crc = (crc >> 1) ^ 0xEDB88320;
         }
         else {
            crc = crc >> 1;
         }
      }
      CRC32[i] = crc;
   }
}

//crc32实现函数
u32 crc32(const u8* buf, int len) {
   static unsigned char init = 0;
   u32 ret = 0xFFFFFFFF;
   if (!init) {
      init_table();
      init = 1;
   }
   for (int i = 0; i < len; i++) {
      ret = CRC32[((ret & 0xFF) ^ buf[i])] ^ (ret >> 8);
   }
   ret = ~ret;
   return ret;
}

void convert_num_to_string(unsigned u32 data, unsigned char *string, unsigned char length)
{
    unsigned char temp;
    while (length)
    {
        string[length-- -1] = data % 16 + (((data % 16) < 10) ? '0' : ('A' - 10));
        data /= 16;
    }
}

/* run this program using the console pauser or add your own getch, system("pause") or input loop */
 void datahex(char* string,unsigned char slength,unsigned char *data) {
	memset(data,0,16);
   // if(string == NULL) 
    //   return NULL;
    //size_t slength = strlen(string);
    //if((slength % 2) = 0) // must be even
       //return NULL;
    unsigned char dlength = slength / 2;
    //uint8_t* data = malloc(dlength);
    //memset(data, 0, dlength);
    unsigned char index = 0;
    while (index < slength) {
        char c = string[index];
        int value = 0;
        if(c >= '0' && c <= '9')
          value = (c - '0');
        else if (c >= 'A' && c <= 'F') 
          value = (10 + (c - 'A'));
        else if (c >= 'a' && c <= 'f')
          value = (10 + (c - 'a'));
        //else {
          //free(data);
          //return NULL;
       // }
		//printf("\ndatais%x %u ",value,index/2);
        data[(index/2)] += value << (((index + 1) % 2) * 4);
	    //printf(" %x %x ",data[(index/2)] ,hex_data[index/2]);
        index++;
    }
    //return data;
}

char data[]="32C6B92A062C8342D4DD7921F0616497";//test weather aes work or not 
unsigned char hex_data[16];
unsigned char key[16]={1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6};

void IotWifiSendString(unsigned char *str,unsigned char channel)
{
	#define CHA_N 11
	unsigned char headstr[20]={"AT+CIPSEND=0,3\x0d\x0a\x00"};
	unsigned char len=0;
	while((str[len]!=0)&&len<50)len++;
    headstr[CHA_N]=channel%10+'0';
	if(len>9){
		headstr[13]=len/10+'0';
		headstr[14]=len%10+'0';
		memcpy(&headstr[15],"\x0d\x0a\x00",3);
	}
	else
		headstr[13]=len%10+'0';
	UartSendString(headstr);
	system_delay(600);
//	memcpy(headstr,"LLL",3);
//	headstr[2]=len+'0';
//	headstr[3]=0;
//	UartSendString(headstr);
	UartSendString(str);
}

void aes_decrypt(unsigned char * decrypt_data){
#if 0
	struct AES_ctx ctx;
	datahex(data,32,hex_data);
	AES_init_ctx(&ctx, key);
	AES_ECB_decrypt(&ctx, hex_data);
	hex_data[19]="";
	display(0,0,hex_data,20);	
#endif

#if 1
	struct AES_ctx ctx;
	unsigned char decrypt_buff[32];
	datahex(decrypt_data,32,decrypt_buff);
	AES_init_ctx(&ctx, key);
	AES_ECB_decrypt(&ctx, decrypt_buff);
//	display(0, 0,decrypt_buff, 20);
	memcpy(decrypt_data,decrypt_buff,16);
#endif
}
void Process_aes_test(void){
	struct AES_ctx ctx;
	datahex("32C6B92A062C8342D4DD7921F0616497",32,hex_data);
	AES_init_ctx(&ctx, key);
	display(0,0,hex_data,20);
	AES_ECB_decrypt(&ctx, hex_data);
	hex_data[19]="";
	display(1,0,hex_data,20);
	datahex("32C6B92A062C8342D4DD7921F0616497",32,hex_data);
	//AES_init_ctx(&ctx, key);
	display(2,0,hex_data,20);
	
	datahex("32C6B92A062C8342D4DD7921F0616497",32,hex_data);
	display(3,0,hex_data,20);
	AES_ECB_decrypt(&ctx, hex_data);
	hex_data[19]="";
//	display(3,0,hex_data,20);
	while(1);
}

void Process_crc32_test()
{
	unsigned char DispStr[20];
	unsigned char test_crc_data[]={0,1,2,3,4,"",6,7,8,9,0};
	static unsigned char i=0;
	unsigned u32 crc_result;
	{
		i=1;
		crc_result=crc32(test_crc_data,5);
		convert_num_to_string(crc_result,DispStr,8);
		display(0,0,DispStr,20);	
		NumberInStr("you can see",15,4,2,DispStr);
		display(2,0,"you look here",20);	
		display(1,0,DispStr,20);	
		while(1);
	}
}

unsigned char  IotWifiWaitCheck;
enum _WAITRESULT IotWifiWaitResponed()
{
	static unsigned char check;
	if(!IotWifiWaitCheck)
		return;
	check++;
	if(check>WaitOverTime)
		return OVER_TIME;
	switch (IotwifiWaitStatus)
	{
		case WaitConnectPhoneResult:
			
			break;
		default :
			break;	
	}

}

void TCPIPRawPackageResolve(unsigned char * ipdata)
{
	//data format and filter uncorrect data
}

void TCPIPDataResolve(struct ST_STRUCT_PACKAGE *packge)
{
	switch(packge->package_type)
	{
		case TCP_TYPE:
			switch(NETWORK_STATUE.ConnectStatue)
			{
				case CONECT_NULL:
					break;
			};
			
		case UDP_TYPE:
			switch(NETWORK_STATUE.ConnectStatue)
			{
				case REGESITER_FLAG|CONECT_NULL:
					
				break;
			}
			break;
		default:
			break;
	}
}



unsigned char IOT_SESOVL_WIFI_ONLINE(struct Aura4GRec * pparam)
{
	unsigned char temp=0;
	if(strncmp(&pparam[15],"+ADA: \"l\"\,\"up\"",sizeof("+ADA: \"l\"\,\"up\"")-2)==0){
		Status4G=1;
		IOTRESTARTSTOPFUN();
		temp=1;
	}
	else if((strncmp(&pparam[15],"+ADA: \"l\"\,\"down\"",sizeof("+ADA: \"l\"\,\"down\"")-1))==0){
		Status4G=0;
		IOTRESTARTIDLEFUN();
		temp=1;
	}
	return temp;
	
}

unsigned char IOT_KEY=0;
unsigned char IOT_SESOVL_WIFI_DATA(struct Aura4GRec * pparam)
{
	unsigned char temp=0;
	if(pparam->SendOrGet=='g'){
	if(pparam->RecString[0]=='T')
	{
		temp = ((pparam->RecData[1]-'0')*10+pparam->RecData[2]-'0' );
		if(temp<sizeof(KEY_USART_MAP))
			RESIVE_UAER_KEY=KEY_USART_MAP[temp];
			UartSendString("AT+ADA=\"s\"\,\"TG\"\,\"s\"\,\"99\"\x0d\x0a");
			IOT_KEY=RESIVE_UAER_KEY;
	}
	if(pparam->RecString[0]=='F'){
		if(pparam->RecData[0]=='0')
		{
			if (POWER_ON){
				UartSendString("AT+ADA=\"s\"\,\"FD\"\,\"i\"\,0\x0d\x0a");
				IOT_KEY=0x80|1;
			}
			
		}
		else if(pparam->RecData[0]=='1')
		{
            if (!POWER_ON){
				UartSendString("AT+ADA=\"s\"\,\"FD\"\,\"i\"\ ,1\x0d\x0a");
				IOT_KEY=0x80|2;
        	}
		}
		else if(pparam->RecData[0]=='3')
		{
				system_delay(500);
				if (POWER_ON)
					UartSendString("AT+ADA=\"s\"\,\"FD\"\,\"i\"\,1\x0d\x0a");
				else
					UartSendString("AT+ADA=\"s\"\,\"FD\"\,\"i\"\,0\x0d\x0a");
				system_delay(500);
				IOT_KEY=0x80|4;
		}
	}
}
	return temp;
}



void EUSART_DataHandler(void)
{
#define	DisplayACSIIString_5X7(n1,n2,n3)
    char i = 0;
    signed  char wait_ok = 0;
    signed  char temp = 0;
    unsigned char delay = 0;
    unsigned char *p = NULL;
    const char *s;
    static unsigned char POWER_ON_OK = 0;
    static unsigned char POWER_OFF_OK = 0;
    unsigned char *p = NULL;
	static unsigned long System_20_Tick_Cnt_remember=0;
	struct ST_PACKAGE *ptcp;
	struct Aura4GRec * pAura4GRec;
	unsigned char str[50];
	IOKeepOnline();
	IOTRESTARTHANDLEFUN();
//	IOTQueueKeyHandle();
POWER_ON_GO:
    {
#if 1
		if(!IOTUartQueueioifempty())
        {        
			IOTUartQueuePop(RX_);
			ptcp=RX_;
/***********************************************************************/
            pAura4GRec=RX_;
			if(IOT_SESOVL_WIFI_ONLINE(pAura4GRec))
				goto CLEARN;	
			if(IOT_SESOVL_WIFI_DATA(pAura4GRec))	
				goto CLEARN;	

/***********************************************************************/
			//481c55e223:aabbccddeeff:192.168.0.166:1234
			if(ptcp->PackageHead.package_type=='1')
			{
				//connected to phone
				display(0,0,"connnecting phone",20);
				memcpy(str,"AT+CIPSTART=0\,\"TCP\"\,\"192.168.0.145\"\,1234\x0d\x0a",sizeof("AT+CIPSTART=1\,\"TCP\"\,\"192.168.0.
145\"\,1234\x0d\x0a"));
				memcpy(&str[sizeof("AT+CIPSTART=0\,\"TCP\"\,\"")-1],ptcp->RawUDP.FindConnected.SourceIp,sizeof("192.168.0.145")-1);
				memcpy(&str[sizeof("AT+CIPSTART=0\,\"TCP\"\,\"192.168.0.145\"\,")-1],ptcp->RawUDP.FindConnected.SourcePort,4);
				str[50]=0;
				UartSendString(str);//AT+CIPSTART=1,"TCP","192.168.0.145",1234
				system_delay(5000);
				return;
			}
/***********************************************************************/
			aes_decrypt(ptcp->RawTCP.EncrytData);
            {
                p = &(ptcp->PackageHead);
                wait_ok = 0;
                wait_ok = !(unsigned char)strncmp("+IPD", RX_, 3);
                if (!wait_ok)
                    goto CLEARN;
                p += 5;
                TCP.num = *p - '0';
                p += 2;
                TCP.len = 0;
				i=0;
                while ((*p != ':')&&(i<15))
                {
                    TCP.len = TCP.len * 10 + *p - '0';
                    p++;
                }
				if(i>15)
					goto CLEARN;
                p++;
				memcpy((unsigned char *)TCP.data,ptcp->RawTCP.EncrytData,20);
	            if (POWER_ON)
                {
                    DisplayACSIIString_5X7(4, 0, "RECIVE DATA:");
                    DisplayACSIIString_5X7(5, 1, "					");
                    ptcp->RawTCP.EncrytData[15] = 0;
                    DisplayACSIIString_5X7(5, 0, ptcp->RawTCP.EncrytData);
                    if (!(unsigned char)strncmp("OFF", ptcp->RawTCP.EncrytData, 2))
                    {
						gotoPOWER_OFF:
						 menuid=0x0b;
//						newmenuid=1;
						POWER_ON = 0;
                        DisplayACSIIString_5X7(7, 0,"  Will POWER OFF  ");
                        display(0, 0, "   WIll POWER OFF   ", 20);
                        display(1, 0, "                    ", 20);
                        display(2, 0, "                    ", 20);
                        display(3, 0, "                    ", 20);
                        system_delay(10000);
                        LCDScreenClear();
                        BacklightControl(0);
                        POWER_OFF_OK = 1;
						goto OUTIF;
                    }
                    else if (ptcp->RawTCP.EncrytData[0] == 'W')
                    {
                        temp = ((ptcp->RawTCP.EncrytData[1] - '0') * 10 + ptcp->RawTCP.EncrytData[2] - '0');

                        if (ptcp->RawTCP.EncrytData[3] == '0')
                            temp = 50;
                        else if (temp != 0)
                            temp = temp / 2;

                        if (ptcp->RawTCP.EncrytData[2] == 0)
                            temp = ptcp->RawTCP.EncrytData[1] - '0';
                        BacklightControl(temp);
                        s = "                      ";
                    }
                    else if (ptcp->RawTCP.EncrytData[0] == 'k')
                    {
						temp = (ptcp->RawTCP.EncrytData[1] - '0');
						RESIVE_UAER_KEY=KEY_USART_MAP[temp];
						ptcp->RawTCP.EncrytData[2]='2';
						ptcp->RawTCP.EncrytData[3]=0;
						IotWifiSendString(&ptcp->RawTCP.EncrytData[0],TCP.num );
                    }
                }
                else if (!POWER_ON)
                {
                    if (!(unsigned char)strncmp("ON", ptcp->RawTCP.EncrytData, 2))
                    {
						gotoPOWER_ON:
						POWER_ON = 1;
                        lcd_init();
                        BacklightControl(BKL_DUTY_PERIOD);
                        s = "SSID:ESP32          ";
                        display(0, 0, s, 20);
                        s = "PASSWORD:1234567890 ";
                        display(1, 0, s, 20);
                        display(2, 0, "                    ", 20);
                        s = "USE PHONE CONTROL   ";
                        display(3, 0, s, 20);
                        POWER_ON_OK = 1;
				        menuid=0x0b;
       					newmenuid=1;
						message=0;
						goto OUTIF;
                    }
                }
            }
CLEARN:		OUTIF:
            memset(RX_, 0, sizeof(RX_));
            WifiReciveString = 0;
			IOKeepOnlineCnt=0;
#endif
			System_20_Tick_Cnt=System_20_Tick_Cnt_remember;
		}
    }
#endif

    return;
	
}

void pic18_int(void)/ 
{
	unsigned char temp[100];
	unsigned char i=0;
	{
		TX_DONE=1;
	}
		static unsigned char cnt =0 ; 
		static unsigned char stop[1] ;
        static unsigned char start;
		unsigned char i=0;
        i=RCREG1;
		RX_CNT=0;	
        RX_BUFF[RX_BUFF_CNT]=i;
        if(RX_BUFF_CNT<(sizeof(RX_BUFF)-1))
			RX_BUFF_CNT++;
	}
    makesure_uart_not_interrupt_intoif :
	if(INTCONbits.TMR0IF)
	{
		INTCONbits.TMR0IF = 0;
		if(RX_CNT<20)
		{	RX_CNT++;
//			goto makesure_uart_not_interrupt;
		}
		else if (RX_CNT==20){
			IOTUartQueuePush(RX_BUFF);
			RX_CNT=0xff;
			RX_BUFF_CNT=0;
		}
		if(IOTQueueKeyCnt<=100)
			IOTQueueKeyCnt++;
		if(systick==100)
		{
			IOKeepOnlineCnt++;
				IOTRESTARTFlag=1;
		}
}



/******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/


