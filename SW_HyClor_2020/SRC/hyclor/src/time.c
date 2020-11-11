#include "include.h"
void Relay_set(unsigned out)
{
     setBit(((uint32_t)&M0P_GPIO->PADIR + GpioPortD), GpioPin7, out?FALSE:TRUE);//输出
     setBit(((uint32_t)&M0P_GPIO->PAPU + GpioPortD), GpioPin7, out?FALSE:FALSE);
     setBit(((uint32_t)&M0P_GPIO->PAPD + GpioPortD), GpioPin7, out?FALSE:TRUE);
}
/***********************************************************************/
//define of time
_bitss flag; 
unsigned char ucDay;//星期值变�?
unsigned char ucHour;//小时
unsigned char ucMin;//分钟
unsigned char ucSec;//秒钟
unsigned char uc_TIMERn;
unsigned char TIMER_Groud;
unsigned char TIMER_DAY;
unsigned char CLOCK_DAY;
unsigned char TIMER_DISPLA_N;
unsigned char TIMER_DISPLA_HOUR;
unsigned char TIMER_DISPLA_MIN;
unsigned char CLOCK_MOD_DISPLA;
unsigned char CLOCK_MOD_N;
unsigned int RUN_SEC;
unsigned char EXIT_TIMER_SEC;
unsigned char RUN_MIN;
unsigned int  RUN_HOUR;
TIMER Timer_Buffer[8];
#if USE_NEW_TIME_JUDGE
unsigned char time_index_now=0;
#endif

void time_init(void)
{
	ucDay=1;//初始化星期一
	ucHour=00;//初始化小�?
	ucMin=00;//初始化分�?
	ucSec=00;//初始化秒�?
	Flag_1s=0;//1秒中断标志位
	ucTimer_Flag=0;//定时模式标志�? 0时为CLOCK�?为TIMER
	TIMER_DISPLA_N=0;//TIMER显示星期的数据数组号�?开�?
	TIMER_DISPLA_HOUR=0;//TIMER小时初始化显�?
	TIMER_DISPLA_MIN=0;//TIMER MIN初始化显�?
	CLOCK_MOD_N=2;//时钟模式下，初始化模式为�?
	EXIT_TIMER_SEC=0;
	FLAG_1MIN=0;
	RUN_SEC=0;
}

/***********************************************************************/
// function of time
void MOD_CHOOSE(void)
{	
	switch(CLOCK_MOD_N)
	{
		case 0:	
			CLOCK_MOD_DISPLA=MOD_ON;
			#if USE_FORCE_PLUGE
				if(1)
			#else
                if(IS_220V)//(_pb5==1)//need change
			#endif
			{
				 Relay_ON;	//打开继电�?
				LED_ON;//打开LED 	
			}
			else
			{

			   	Relay_OFF;	//关闭继电�?
				LED_OFF;//关闭LED
				
			}
			break;
		case 1:
				CLOCK_MOD_DISPLA=MOD_AUTO;
				#if USE_FORCE_PLUGE
					if(1)
				#else
                    if(IS_220V)//(_pb5==1)//need change
				#endif
				{
					Updata_Output_All();
				}
				else
				{
					Relay_OFF;	//关闭继电�?
					LED_OFF;//关闭LED 		
				}
			
			break;
		case 2:	
			CLOCK_MOD_DISPLA=MOD_OFF;
			Relay_OFF;	//关闭继电�?
			LED_OFF;//关闭LED
			break;
		case 3:
			CLOCK_MOD_DISPLA=MOD_AUTO;
			#if USE_FORCE_PLUGE
			if(1)
			#else
	        if(IS_220V)//(_pb5==1)//need change
			#endif
			{
				Updata_Output_All();
			}
				else
			{
				Relay_OFF;	//关闭继电�?
				LED_OFF;//关闭LED 		
			}
			
			break;	
		default:
			break;	
	}
/*	}*/
}

void RUN_CLOCK_EEPROM(void)//保存设备运行时间
{
	if(FLAG_5MIN==1)
	{	
		FLAG_5MIN=0;
		RUN_MIN+=5;
		if(RUN_MIN==60)
		{
			RUN_MIN=0;
			RUN_HOUR++;
	   		SAVE_RUN_TIME();//save run time every hour
		}
		Eeprom_Write_Byte(RUN_MIN_ADDRESS,RUN_MIN);	//5min保存一次分钟�?
	}
}

void time_process(void) 
{
    if(++ucSec>59)
    {
    	ucSec=0;
    	if (++ucMin>59)                                               //60min
    	{
    		ucMin=0;
    		if (++ucHour>23)	                                       //24h
    		{
    			ucHour=0;
    			if (++ucDay>7)                                         //
    			{
    				ucDay=1;
    			}
    		}
    	}
    }
	Flag_1s=~Flag_1s;
	if(++RUN_SEC>299)//300s保存
	{
		FLAG_5MIN=1;	
		RUN_SEC=0;	
	}
	if(FLAG_1MIN==0)//按下TIMER后FLAG_1MIN=0;
	{
		if(++EXIT_TIMER_SEC>39)//40S后，标志位等�?，此时，回到CLOCK显示界面
		{
			FLAG_1MIN=1;
		}
	}
}

unsigned char judge_timer_on(unsigned char max_group)
{
#if USE_NEW_TIME_JUDGE
	unsigned char temp=0;
	unsigned long  TIMER_ON,TIMER_OFF;
	TIMER *pt=Timer_Buffer;
	unsigned long  group_index=0;
	unsigned long  day_time_now=(unsigned long)ucHour*3600+(unsigned int)ucMin*60+ucSec;
	for(group_index=0;group_index<(max_group);group_index++)
	{
		TIMER_ON=(unsigned long)pt->Hour_ON*3600+(unsigned int)pt->Min_ON*60;
		TIMER_OFF=(unsigned long)pt->Hour_OFF*3600+(unsigned int)pt->Min_OFF*60;
		if((CLOCK_DAY&pt->Day_ON)&&(day_time_now>=TIMER_ON)&&(day_time_now<TIMER_OFF))
		{
			if(!temp&&(TIMER_ON+TIMER_OFF))
				temp=1+group_index;
		}
		pt++;
	}
	return temp;
#endif
}

void Updata_Output_All(void)
{
	if(ucTimer_Flag)
		return;
	time_index_now = judge_timer_on(FLAG_GROUP_2?2:8);
	if(time_index_now)	
	{
		Relay_ON;	//turn on relay
		LED_ON;//turn on led
	}
	else
	{
		
		Relay_OFF;//turn off relay
		LED_OFF;//turn off led 
	}
}



SYSTEM_INFO Sys_Info;
SYSTEM_INFO * Sys_Info_Ptr=&Sys_Info;


/***********************************************************************/
//rtc 部分

void RtcAlarmCb(void)
{
}

void rtc_int(void)
{
	stc_rtc_config_t stcRtcConfig; 
    stc_rtc_irq_cb_t stcIrqCb;
    stc_rtc_time_t  stcTime;
    stc_rtc_alarmset_t stcAlarm;
    stc_rtc_cyc_sel_t   stcCycSel;
    stc_lpm_config_t stcLpmCfg;
    stc_gpio_config_t stcGpioCfg;

	DDL_ZERO_STRUCT(stcRtcConfig);
    DDL_ZERO_STRUCT(stcIrqCb);
    DDL_ZERO_STRUCT(stcAlarm);
    DDL_ZERO_STRUCT(stcTime);
    DDL_ZERO_STRUCT(stcCycSel);
    DDL_ZERO_STRUCT(stcLpmCfg);
    DDL_ZERO_STRUCT(stcGpioCfg);
    
    Sysctrl_SetPeripheralGate(SysctrlPeripheralRtc,TRUE);//RTC模块时钟打开

    
#if USE_XTL
	stcRtcConfig.enClkSel = RtcClk32768;//RtcClkHxt1024;//RtcClk32;//RtcClk32768//
#else
	stcRtcConfig.enClkSel = RtcClk32;//RtcClkHxt1024;//RtcClk32;//RtcClk32768//
#endif /* NEVER */
    stcRtcConfig.enAmpmSel = Rtc24h;//Rtc12h;//
   
    stcCycSel.enCyc_sel = RtcPrads;
    stcCycSel.enPrds_sel = Rtc_1S;

	stcRtcConfig.pstcCycSel = &stcCycSel;
	
	#if 1
    Rtc_DisableFunc(RtcCount);
    stcAlarm.u8Minute = 0x59;
    stcAlarm.u8Hour = 0x10;
    stcAlarm.u8Week = 0x02;
    Rtc_DisableFunc(RtcAlarmEn);
    Rtc_EnAlarmIrq(Rtc_AlarmInt_Enable);
    Rtc_SetAlarmTime(&stcAlarm);
    Rtc_EnableFunc(RtcAlarmEn);
	#endif
    stcTime.u8Year = 18;
    stcTime.u8Month = 0x04;
    stcTime.u8Day = 0x16;
    stcTime.u8Hour = 20;
    stcTime.u8Minute = 0;
    stcTime.u8Second = 0;
    stcTime.u8DayOfWeek = Rtc_CalWeek(&stcTime.u8Day);
    stcRtcConfig.pstcTimeDate = &stcTime;
    
    stcIrqCb.pfnAlarmIrqCb = RtcAlarmCb;
    stcIrqCb.pfnTimerIrqCb = RtcCycCb;
    stcRtcConfig.pstcIrqCb = &stcIrqCb;
    stcRtcConfig.bTouchNvic = TRUE;
    
    Rtc_DisableFunc(RtcCount);
    Rtc_Init(&stcRtcConfig); 
    
    Rtc_EnableFunc(RtcCount);

}

/***********************************************************************/

