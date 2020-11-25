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
#include "crc32.h"
#include "app_uart.h"
#include "app_aes.h"
#include "keypad.h"

/******************************************************************************/
//CL
unsigned char IOT_KEY=0;
extern unsigned char ucCellCurrent;
extern unsigned char ucDisplayWaitTimerCnt;
extern unsigned char  ucCellPolarity;
void CellPolarityControl(void);
void CellCurrentLED(unsigned char ucVal);
void CellOutputCurrentAdjustAPI(unsigned char CellCurrent);
/******************************************************************************/

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
unsigned char u8RX_BUFF[100];
unsigned char POWER_ON = 1;
unsigned char bTX_DONE;
unsigned char u8RX_RECIEVE_CNT;
unsigned char u8RX_BUFF_CNT;
unsigned char BASE_CNT=0;
unsigned char READY_SEND_DEGUE=0;

uint8_t u8TxData[44] = {0x00,0x55};
uint8_t u8TxCnt=0,u8RX_RECIEVE_CNT=0;
uint8_t tx_send;
uint8_t rx_reci;
void TxIntCallback(void)
{
		bTX_DONE=1;
    Uart_ClrStatus(UARTCH1,UartTC);    
}
void RxIntCallback(void)
{
	uint8_t u8RxData;
    u8RxData=Uart_ReceiveData(UARTCH1);
	u8RX_RECIEVE_CNT=0;	
	u8RX_BUFF[u8RX_BUFF_CNT]=u8RxData;
	if(u8RX_BUFF_CNT<(sizeof(u8RX_BUFF)-1))
		u8RX_BUFF_CNT++;
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
    stcBaud.enRunMode = UartMode1;
    stcBaud.u32Baud = 115200;
    u16Scnt = Uart_CalScnt(UARTCH1,&stcBaud);
    Uart_SetBaud(UARTCH1,u16Scnt);
    
    Uart_EnableIrq(UARTCH1,UartRxIrq);
    Uart_ClrStatus(UARTCH1,UartRC);
    Uart_EnableFunc(UARTCH1,UartRx);
	
    #if 0 //UART TEST 
    while(1)
    {
		unsigned char i=0;
		unsigned char *text="sdfaslkfjeroiquerqieruweoip";
		//if(u8RX_RECIEVE_CNT>=1)
        if(1){
           Uart_DisableIrq(UARTCH1,UartRxIrq);
           Uart_EnableIrq(UARTCH1,UartTxIrq);
			u8RX_RECIEVE_CNT = 0;
			for(i=0;i<33;i++)
			{
 			  	
			bTX_DONE=0;
            M0P_UART1->SBUF ='r';// u8RxData;   
		 	 while(!bTX_DONE);
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
	  //	RESIVE_UAER_KEY=IOTQueueKeyPop();
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
//pEusartQueue5};
struct  _UartQueue * pEusartQueueStorage;
struct  _UartQueue * pEusartQueueResolve;
void IOTUartQueueInit()
{
	pEusartQueueStorage=UartQueueBuff;
	pEusartQueueResolve=UartQueueBuff;
}

unsigned char  IOTUartQueuePush(unsigned char *data)
{
	if((pEusartQueueStorage!=pEusartQueueResolve)||((pEusartQueueStorage==pEusartQueueResolve)&&(pEusartQueueStorage->
status==empty)))
	{
		memcpy((pEusartQueueStorage->DATABuff),data,NUM_RX_MAX);
		memset(u8RX_BUFF,0,NUM_RX_MAX);
		pEusartQueueStorage->status=filled;
		if(pEusartQueueStorage==&UartQueueBuff[UART_QUEUE_LENTH-1])
			pEusartQueueStorage=UartQueueBuff;
		else
			pEusartQueueStorage++;
		return 1;
	}
    memset(u8RX_BUFF,0,NUM_RX_MAX);
	return 0;
}

void IOTUartQueuePop(unsigned char *data)
{
	struct	_UartQueue *q=pEusartQueueResolve;
	memcpy(data,&pEusartQueueResolve->DATABuff[0],NUM_RX_MAX);
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
//"+ADA: "g","ok","TG","s","1"\r\n"
struct Aura4GRec_t
{
	unsigned char resever[7];//+ADA: " 
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
struct TCP_RESOLVE_t
{
    unsigned char num;
    unsigned char len;
    unsigned char data[100];
}TCP;
//+IPD,0,58:2a25bf2f6ac8+0f693c98+32+3F9501290C8F605E6EE36D3ADA0EBD53
struct PACKAGE_HEAD_t
{
	unsigned char head[5];//+IPD,
	unsigned char package_type;//0
	unsigned char unused_1;//,
	unsigned char len[2];
	unsigned char unused_2;//:
};

struct TCP_t
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
struct UDP_t
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



struct PACKAGE_t
{
	struct PACKAGE_HEAD_t PackageHead;
	union {
		struct TCP_t RawTCP;
		struct UDP_t RawUDP;
	};
};

struct STRUCT_PACKAGE_t
{
	unsigned char *ppackage;
	unsigned char package_type;
};

struct NETWORK_STATUE_t
{
	unsigned char ConnectStatue;
	unsigned char RemoteDeviceID[12];
	unsigned int ReomtePort;
	unsigned char DeviceID[12];
}NETWORK_STATUE;

unsigned char RESIVE_UAER_KEY;
unsigned char KEY_USART_MAP[]={
	KEY_UP			  ,
	KEY_DOWN			  ,
	KEY_UP_DOWN	  
};


void system_delay(unsigned int u32cnt)
{
	delay1ms(u32cnt);
}

const unsigned char *AT_CMD[] =
#if 1//4G
{
	"AT+NETOPEN\x0d\x0a",
	"at+capnet=6\,1\x0d\x0a",
	"at+ada=\"start\"\x0d\x0a",
	"AT+ADA=\"enable\"\x0d\x0a",
	"AT+ADA=\"l\"\x0d\x0a",
	"AT+ADA=\"l\"\x0d\x0a",
	"AT+ADA=\"s\"\,\"FD\"\,\"i\"\,1\x0d\x0a"
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
	//Uart_DisableIrq(UARTCH1,UartRxIrq);
	Uart_EnableIrq(UARTCH1,UartTxIrq);
    while ((*text > 0x00))
    {
        EUSART_SendData(*text);
        text++;
		i++;
		if(i>100)
			break;
    }
}

void EUSART_SendData(unsigned char data)//unsigned char ucLenth)
{
	bTX_DONE=0;
    M0P_UART1->SBUF = data;
	while (!bTX_DONE);
}

void EUSART_ReceiveData(unsigned char data)//(unsigned char ucLenth)
{

}


void EUSART_WIFI_INIT(void)
{
    unsigned char i = 0;
    unsigned int J = 0;
    unsigned int J1 = 0;
	u8RX_RECIEVE_CNT=0xff;
	Init_APP_Uart();
	IOTUartQueueInit();
    for (i = 0; i < 6; i++)//(sizeof(AT_CMD)/sizeof(unsigned char *))
    {

        UartSendString((unsigned char *)AT_CMD[i]);
		delay1ms(100);
    }
	 UartSendString("AT+ADA=\"s\"\,\"FD\"\,\"i\"\,1\x0d\x0a");
	 IOTRESTARTIDLEFUN();
	 //send power on signal
}

void EUSART_WIFI_INIT_PROCESS(void)
{
    static unsigned char  step = 0;
    static unsigned int  u32cnt = 0;
	u32cnt++;
	unsigned char t[2]={'0',0};
	if((u32cnt%20000)==0){
		if(step<(sizeof(AT_CMD)/sizeof(AT_CMD[0]))){
				
	  
		  //  UartSendString(t);
			  //UartSendString("testyoulittlebaster\n");
		    UartSendString((unsigned char *)AT_CMD[step]);
//			UartSendString("AT+ADA=\"s\"\,\"FD\"\,\"i\"\,1\x0d\x0a");
				step++;
		}
	}
	//send power on signal
}


void convert_num_to_string( u32 data, unsigned char *string, unsigned char length)
{
    unsigned char temp;
    while (length)
    {
        string[length-- -1] = data % 16 + (((data % 16) < 10) ? '0' : ('A' - 10));
        data /= 16;
    }
}

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


unsigned char  bIotWifiWaitCheck;
enum _WAITRESULT IotWifiWaitResponed()
{
	static unsigned char check;
	if(!bIotWifiWaitCheck)
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
void TCPIPDataResolve(struct STRUCT_PACKAGE_t *packge)
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

unsigned char IOT_RESOLVE_WIFI_ONLINE(struct Aura4GRec_t * pparam)
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

void IOT_SEND_SELF_STATUE()
{
	unsigned char str[]="AT+ADA=\"s\"\,\"UU\"\,\"i\"\ ,92000\x0d\x0a";
	unsigned char temp=0;
	UartSendString("AT+ADA=\"s\"\,\"UU\"\,\"i\"\ ,90001\x0d\x0a");
	delay1ms(30);
	UartSendString("AT+ADA=\"s\"\,\"UU\"\,\"i\"\ ,91001\x0d\x0a");
	delay1ms(30);
	//UartSendString("AT+ADA=\"s\"\,\"UU\"\,\"i\"\ ,92100\x0d\x0a");
	temp=sizeof("AT+ADA=\"s\"\,\"UU\"\,\"i\"\ ,920")-2;
	str[temp++]=ucCellCurrent/100+'0';
	str[temp++]=ucCellCurrent%100/10+'0';
	str[temp++]=ucCellCurrent%10+'0';
	UartSendString(str);
	delay1ms(30);
}

unsigned char IOT_RESOLVE_WIFI_DATA(struct Aura4GRec_t * pparam)
{
//"+ADA: "g","ok","TG","s","1"\r\n"
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
				POWER_ON=!POWER_ON;
				LEDALLControl(0);
			}
			
		}
		else if(pparam->RecData[0]=='1')
		{
            if (!POWER_ON){
				UartSendString("AT+ADA=\"s\"\,\"FD\"\,\"i\"\ ,1\x0d\x0a");
				IOT_KEY=0x80|2;
				POWER_ON=!POWER_ON;
				LEDALLControl(1);
        	}
		}
		else if(pparam->RecData[0]=='3')
		{
			IOT_SEND_SELF_STATUE();
			delay1ms(30);
			if (POWER_ON)
				UartSendString("AT+ADA=\"s\"\,\"FD\"\,\"i\"\,1\x0d\x0a");
			else
				UartSendString("AT+ADA=\"s\"\,\"FD\"\,\"i\"\,0\x0d\x0a");
			delay1ms(30);
			IOT_KEY=0x80|4;
		}
		}
	
		if(pparam->RecString[0]=='Z'){
			temp=pparam->RecData[1]-'0';
			temp=temp*10+pparam->RecData[2]-'0';
			temp=temp*10+pparam->RecData[3]-'0';
			ucCellCurrent = temp;
			CellOutputCurrentAdjustAPI(ucCellCurrent);
			ucDisplayWaitTimerCnt = 3;	
			temp=1;
		}
			
		if(pparam->RecString[0]=='A'){
			if(pparam->RecData[2]=='R')
			{
				temp=0;	
//#define _REVERSE                                 0
			}
			else
				temp=1;
			if(ucCellPolarity!=temp){
				ucCellPolarity=temp;
				delay1ms(100);
				CellPolarityControl();
			}
			temp=temp*10+pparam->RecData[2]-'0';
			temp=1;
			ucDisplayWaitTimerCnt = 3;	
		}
	
	
	}
	return temp;
}

unsigned char  IOT_RESOLVE_KEY(unsigned char *key)
{
	if(IOT_KEY){
		*key=IOT_KEY;
		IOT_KEY=0;
		return 1;
	}
	return 0;
}

unsigned char IOT_RESOLVE_DATA_UDP(struct PACKAGE_t * pparam)
{
/***********************************************************************/
#if 0
	//481c55e223:aabbccddeeff:192.168.0.166:1234
	if(pparam->PackageHead.package_type=='1')
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
#endif
/***********************************************************************/
}

unsigned char IOT_RESOLVE_DATA_ONOFF(struct Aura4GRec_t * pparam)
{
#if 0
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
//						  display(0, 0, "	WIll POWER OFF	 ", 20);
//						  display(1, 0, "					 ", 20);
//						  display(2, 0, "					 ", 20);
//						  display(3, 0, "					 ", 20);
				system_delay(10000);
//						  LCDScreenClear();
//						  BacklightControl(0);
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
				s = "					   ";
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
				s = "SSID:ESP32 		 ";
				display(0, 0, s, 20);
				s = "PASSWORD:1234567890 ";
				display(1, 0, s, 20);
				display(2, 0, " 				   ", 20);
				s = "USE PHONE CONTROL	 ";
				display(3, 0, s, 20);
				POWER_ON_OK = 1;
//						menuid=0x0b;
//							newmenuid=1;
//						message=0;
				goto OUTIF;
			}
		}
	}
#endif
}


void IOTMessageQueueHandler()
{
	struct PACKAGE_t *ptcp;
	struct Aura4GRec_t * pAura4GRec;
	unsigned char u8RX_[NUM_RX_MAX];
	memset(u8RX_, 0, sizeof(u8RX_));
	if(!IOTUartQueueioifempty())
	{        
		IOTUartQueuePop(u8RX_);
		DEBUG_SEND_STRING("WEGETDATAWOHO\n");
		DEBUG_SEND_STRING(u8RX_);
	    pAura4GRec=u8RX_;
		if(IOT_RESOLVE_WIFI_ONLINE(pAura4GRec))
			return;	
		if(IOT_RESOLVE_WIFI_DATA(pAura4GRec))	
			return; 
	}
}

unsigned char eIOTEVENT=_IOT_EVENT_NULL;
void IOTEventHandler()
{
	switch(eIOTEVENT){
	case _IOT_EVENT_UPDATE_KEY:
		unsigned char str[]="AT+ADA=\"s\"\,\"UU\"\,\"i\"\ ,92000\x0d\x0a";
		unsigned char temp=0;
		delay1ms(30);
		//UartSendString("AT+ADA=\"s\"\,\"UU\"\,\"i\"\ ,92100\x0d\x0a");
		temp=sizeof("AT+ADA=\"s\"\,\"UU\"\,\"i\"\ ,920")-2;
		str[temp++]=ucCellCurrent/100+'0';
		str[temp++]=ucCellCurrent%100/10+'0';
		str[temp++]=ucCellCurrent%10+'0';
		UartSendString(str);
		break;
	default:
		break;
	}
	eIOTEVENT=_IOT_EVENT_NULL;
}

void IOTHandler(void)
{
	IOT_LOOP:
#define	DisplayACSIIString_5X7(n1,n2,n3)
//	EUSART_WIFI_INIT_PROCESS();
//	IOKeepOnline();
//	IOTRESTARTHANDLEFUN();
//	IOTQueueKeyHandle();
	IOTMessageQueueHandler();
	IOTEventHandler();
	if(!POWER_ON)
	{
		goto IOT_LOOP;
	}
}

void APP_UART_1MS_HANDLE(void) 
{
#if 0//1ms output test
	static unsigned char dir=0;
	if(dir==0)
	{
		Gpio_SetIO(GpioPortD, GpioPin1);
		dir=1;
	}
	else
	{
		dir=0;
		Gpio_ClrIO(GpioPortD, GpioPin1);
	}
#endif
    makesure_uart_not_interrupt_intoif :
	if(u8RX_RECIEVE_CNT<20)
	{	
		u8RX_RECIEVE_CNT++;
	}
	else if (u8RX_RECIEVE_CNT==20){
		IOTUartQueuePush(u8RX_BUFF);
		u8RX_RECIEVE_CNT=0xff;
		u8RX_BUFF_CNT=0;
	}
	if(IOTQueueKeyCnt<=100)
		IOTQueueKeyCnt++;
	IOKeepOnlineCnt++;
}

/******************************************************************************
 ******************************************************************************/


